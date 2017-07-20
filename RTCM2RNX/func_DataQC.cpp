/****************************************************************************
* 版权所有（C）2017，广州海格通信集团股份有限公司
*
* 文件名称：func_CycleSlip.cpp
* 内容摘要：周跳探测
* 其他说明：
* 版本  号：
* 作    者：xj
* 完成日期：2017-6
*
* 修改记录1：
*    修改日期：
*    版本  号：
*    修改  人：
*    修改内容：
* 修改记录2：
******************************************************************************/

/******************************************************************************
*                          包含的头文件                                       *
******************************************************************************/
#include "def_DataQC.h"

/******************************************************************************
*                          静态数据变量                                       *
******************************************************************************/
/*GLONASS卫星频率数-----------------------------------------------------------*/
static INT32S iFrqChn[24]=
{
     1,-4, 5, 6, 1,
    -4, 5, 6,-2,-7,
     0,-1,-2,-7, 0,
    -1, 4,-3, 3, 2,
     4,-3, 3, 2
};

/******************************************************************************
*                         全局变量                                       *
******************************************************************************/
T_SatState g_tSatState[MAXSAT];
ObserDataEpoch g_pObsEpoch;
ObserDataEpoch g_slipDectObser;
INT8U g_iCycleFlg[MAXSAT];//该历元周跳标志位         0-无周跳    1-有周跳    -1-数据不全    2-无法探测(单频或Glonass三频或上历元双频本历元单频)
FP64 g_TFRes[MAXSAT][NFREQ+NEXOBS];//三频探测周跳时每个历元EWL、WL、NL残差值
FP64 g_TFIonRes[MAXSAT];//三频探测周跳时电离层残差值
INT32S g_deltaNTF[MAXSAT][NFREQ+NEXOBS];//周跳数值
FP64 g_fMPWin[MAXSAT][NFREQ+NEXOBS][SlidWinNum];//多路径滑动窗口数据

/*******************************************************************************
* 函数名称： InitialQC()
* 功能描述： 数据质量探测初始化
* 输入参数：
* 输出参数：
* 返 回 值： 无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void InitialQC(void)
{
    memset(&g_slipDectObser,0,sizeof(g_slipDectObser));
    memset(g_iCycleFlg,0,sizeof(g_iCycleFlg));
    memset(g_TFRes,0,sizeof(g_TFRes));
    memset(g_TFIonRes,0,sizeof(g_TFIonRes));
    //memset(g_tSatState, 0, sizeof(g_tSatState));//初始化卫星状态变量
    memset(g_deltaNTF,0,sizeof(g_deltaNTF));
    memset(g_fMPWin,0,sizeof(g_fMPWin));
    memset(&g_pObsEpoch,0,sizeof(g_pObsEpoch));
}

/*******************************************************************************
* 函数名称： DataQC()
* 功能描述： 数据质量探测主函数
* 输入参数：ptRtcm--RTCM数据
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
T_SatState* DataQC(const T_OBSSAT& obs)
{
    INT8U satId = obs.iSat - 1;//卫星编号从1开始
    g_tSatState[satId].iEpochNum ++;//累积历元数
    g_tSatState[satId].iEpochValid ++;//有效历元数
    for (int l=0;l<NFREQ+NEXOBS;l++)
    {
        g_tSatState[satId].iMPCounter[l] ++;
        if (g_tSatState[satId].iMPCounter[l] > 4294967200)
        {
            g_tSatState[satId].iMPCounter[l] = 601;
        }
    }

    INT8U iFreqnum = 0;
    g_pObsEpoch.validSatflg[satId] = 1;
    g_pObsEpoch.slipflg[satId] = 0;
    for (int k=0;k<NFREQ;k++)
    {
       if ((obs.dL[k] < EPSILON) || (obs.dP[k] < EPSILON))
       {
        continue;
       }
        else
        {
            g_pObsEpoch.phaseValue[k][satId] = obs.dL[k];
            g_pObsEpoch.rangeValue[k][satId] = obs.dP[k];
            g_pObsEpoch.iFreq[satId].dFreqFlg[iFreqnum] = k;
            iFreqnum++;
        }
    }
    g_pObsEpoch.iFreq[satId].iNum = iFreqnum;

    //周跳
    if (g_pObsEpoch.iFreq[satId].iNum == 0)
    {
        g_iCycleFlg[satId] = -1;
    }
    else if (g_pObsEpoch.iFreq[satId].iNum == 1)
    {
        g_iCycleFlg[satId] = 2;
    }
    else if (g_pObsEpoch.iFreq[satId].iNum == 2)
    {
        HMW_DD(&g_pObsEpoch, satId);
        IonoRes_DD(&g_pObsEpoch, satId);
    }
    else if (g_pObsEpoch.iFreq[satId].iNum == 3)
    {
        if ((satId+1) <= (NSATGPS+NSATBDS))
        {
            EWL_DD(&g_pObsEpoch, satId);
            WL_DD(&g_pObsEpoch, satId);
            NL_DD(&g_pObsEpoch, satId);
        }
        else
        {
            g_iCycleFlg[satId] = 2;
        }
    }
    if (g_pObsEpoch.slipflg[satId])
    {
        g_iCycleFlg[satId] = 1;
        for (int l=0;l<NFREQ+NEXOBS;l++)//有周跳则清空多路径指标数据,从当前历元开始累积
        {
            g_tSatState[satId].fMP[l] = 0;
            g_tSatState[satId].iMPCounter[l] = 1;
            g_tSatState[satId].fMPMean[l] = 0;
        }
        memset(g_fMPWin[satId],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));

        if (g_pObsEpoch.iFreq[satId].iNum == 2)
        {
            if (g_pObsEpoch.slipflg[satId] >= 100);
            else
            {
                CycleslipRepair_DD(&g_pObsEpoch,satId, g_deltaNTF[satId]);
                if (abs(g_deltaNTF[satId][g_pObsEpoch.iFreq[satId].dFreqFlg[0]]) > EPSILON)
                {
                    g_tSatState[satId].iCycleslipNum[g_pObsEpoch.iFreq[satId].dFreqFlg[0]]++;
                }
                if (abs(g_deltaNTF[satId][g_pObsEpoch.iFreq[satId].dFreqFlg[1]]) > EPSILON)
                {
                    g_tSatState[satId].iCycleslipNum[g_pObsEpoch.iFreq[satId].dFreqFlg[1]]++;
                }
            }
        }
        else if (g_pObsEpoch.iFreq[satId].iNum == 3)
        {
            if (g_pObsEpoch.slipflg[satId] >= 100);
            else
            {
                CycleslipRepair_TD(&g_pObsEpoch,satId, g_deltaNTF[satId]);

                if (abs(g_deltaNTF[satId][g_pObsEpoch.iFreq[satId].dFreqFlg[0]]) > EPSILON)
                {
                    g_tSatState[satId].iCycleslipNum[g_pObsEpoch.iFreq[satId].dFreqFlg[0]]++;
                }

                if (abs(g_deltaNTF[satId][g_pObsEpoch.iFreq[satId].dFreqFlg[1]]) > EPSILON)
                {
                    g_tSatState[satId].iCycleslipNum[g_pObsEpoch.iFreq[satId].dFreqFlg[1]]++;
                }

                if (abs(g_deltaNTF[satId][g_pObsEpoch.iFreq[satId].dFreqFlg[2]]) > EPSILON)
                {
                    g_tSatState[satId].iCycleslipNum[g_pObsEpoch.iFreq[satId].dFreqFlg[2]]++;
                }
            }
        }
    }

    //多路径分析
    if ((g_pObsEpoch.iFreq[satId].iNum >=2) && (!g_tSatState[satId].iEphFlg))
    {
        MPAnalysOneSat(&g_pObsEpoch,satId);
    }
    else
    {
        for (int l=0;l<NFREQ+NEXOBS;l++)
        {
            g_tSatState[satId].fMP[l] = 0;
            g_tSatState[satId].iMPCounter[l] = 0;
            g_tSatState[satId].fMPMean[l] = 0;
        }
        memset(g_fMPWin[satId],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
    }
    return g_tSatState+satId;
}

/*******************************************************************************
* 函数名称： DataQC()
* 功能描述： 数据质量探测主函数
* 输入参数：ptRtcm--RTCM数据
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void DataQC(T_RTCM *ptRtcm, FILE **pFileCS)
{
    INT8U i,j,l,k,iFreqnum=0;
    INT8U satId,iFlg;
    ObserDataEpoch slipDectObser;

    memset(&slipDectObser,0,sizeof(slipDectObser));
    memset(g_iCycleFlg,0,sizeof(g_iCycleFlg));//每历元更新

    slipDectObser.dEpochTime = ptRtcm->tObs.ptData[0].tObsTime.dSow;
    slipDectObser.validSatNum = ptRtcm->tObs.iN;
    //fprintf(pFileCS[5],"%s%.1f  ","epoch=",slipDectObser.dEpochTime);

    for (j=0;j<MAXSAT;j++)
    {
        iFlg = 0;//标示该历元是否有k号卫星0-否 1-是

        for (i=0;i<slipDectObser.validSatNum;i++)
        {
            if (j == ptRtcm->tObs.ptData[i].iSat - 1)
            {
                iFlg = 1;
                satId = ptRtcm->tObs.ptData[i].iSat - 1;//卫星编号从1开始

                g_tSatState[satId].iEpochNum ++;//累积历元数

                //有效历元数、卫星状态、SPP状态、星历状态
                g_tSatState[satId].iEpochNum ++;//历元数
                if (satId<NSATBDS + NSATGPS)
                {
                    if (!ptRtcm->tNav.ptEph[i].iSvh)
                    {
                        g_tSatState[satId].iEpochValid ++;//有效历元数
                        g_tSatState[satId].iEphFlg = 0;
                        g_tSatState[satId].iSatFlg = 0;
                        g_tSatState[satId].iSPPFlg = 0;
                        for (l=0;l<NFREQ+NEXOBS;l++)
                        {
                            g_tSatState[satId].iMPCounter[l] ++;
                            if (g_tSatState[satId].iMPCounter[l] > 4294967200)
                            {
                                g_tSatState[satId].iMPCounter[l] = 601;
                            }
                        }
                    }
                    else
                    {
                        g_tSatState[satId].iEphFlg = 1;
                        g_tSatState[satId].iSatFlg = 1;
                        g_tSatState[satId].iSPPFlg = 1;
                        for (l=0;l<NFREQ+NEXOBS;l++)
                        {
                            g_tSatState[satId].fMP[l] = 0;
                            g_tSatState[satId].iMPCounter[l] = 0;
                            g_tSatState[satId].fMPMean[l] = 0;
                        }
                        memset(g_fMPWin[satId],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
                    }
                }
                else
                {
                    slipDectObser.iGFrq[satId] = ptRtcm->tObs.ptData[i].iFrq;
                    if (!ptRtcm->tNav.ptGeph[i].iSvh)
                    {
                        g_tSatState[satId].iEpochValid ++;
                        g_tSatState[satId].iEphFlg = 0;
                        g_tSatState[satId].iSatFlg = 0;
                        g_tSatState[satId].iSPPFlg = 0;
                        for (l=0;l<NFREQ+NEXOBS;l++)
                        {
                            g_tSatState[satId].iMPCounter[l] ++;
                            if (g_tSatState[satId].iMPCounter[l] > 4294967200)
                            {
                                g_tSatState[satId].iMPCounter[l] = 601;
                            }
                        }
                    }
                    else
                    {
                        g_tSatState[satId].iEphFlg = 1;
                        g_tSatState[satId].iSatFlg = 1;
                        g_tSatState[satId].iSPPFlg = 1;
                        for (l=0;l<NFREQ+NEXOBS;l++)
                        {
                            g_tSatState[satId].fMP[l] = 0;
                            g_tSatState[satId].iMPCounter[l] = 0;
                            g_tSatState[satId].fMPMean[l] = 0;
                        }
                        memset(g_fMPWin[satId],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
                    }
                }
                iFreqnum = 0;
                slipDectObser.validSatflg[satId] = 1;
                slipDectObser.slipflg[satId] = 0;

                for (k=0;k<NFREQ;k++)
                {
                       if ((ptRtcm->tObs.ptData[i].dL[k] < EPSILON) || (ptRtcm->tObs.ptData[i].dP[k] < EPSILON))
                       {
                        continue;
                       }
                    else
                    {
                        slipDectObser.phaseValue[k][satId] = ptRtcm->tObs.ptData[i].dL[k];
                        slipDectObser.rangeValue[k][satId] = ptRtcm->tObs.ptData[i].dP[k];
                        slipDectObser.iFreq[satId].dFreqFlg[iFreqnum] = k;
                        iFreqnum++;
                    }
                }
                slipDectObser.iFreq[satId].iNum = iFreqnum;
            }
        }

        if (!iFlg)//该历元没有j号星
        {
            for (l=0;l<NFREQ+NEXOBS;l++)
            {
                g_tSatState[j].fMP[l] = 0;
                g_tSatState[j].iMPCounter[l] = 0;
                g_tSatState[j].fMPMean[l] = 0;
            }
            memset(g_fMPWin[j],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
        }
    }

    Cycleslip(&slipDectObser,pFileCS);	//先判周跳，后分析多路径效应

    for (i=0; i<MAXSAT; i++)
    {
        if (slipDectObser.slipflg[i])
        {
            if (slipDectObser.iFreq[i].iNum == 2)
            {
                if (slipDectObser.slipflg[i] >= 100)
                {
                    //fprintf(pFileCS[4],"%s%.1f %s%d 0.5cycle not repair\n","epoch=",slipDectObser.dEpochTime,"satid=",i+1);
                }
                else
                {
                    CycleslipRepair_DD(&slipDectObser, i, g_deltaNTF[i],pFileCS);

                    if (abs(g_deltaNTF[i][slipDectObser.iFreq[i].dFreqFlg[0]]) > EPSILON)
                    {
                        g_tSatState[i].iCycleslipNum[slipDectObser.iFreq[i].dFreqFlg[0]]++;
                    }

                    if (abs(g_deltaNTF[i][slipDectObser.iFreq[i].dFreqFlg[1]]) > EPSILON)
                    {
                        g_tSatState[i].iCycleslipNum[slipDectObser.iFreq[i].dFreqFlg[1]]++;
                    }
                }
            }
            else if (slipDectObser.iFreq[i].iNum == 3)
            {
                if (slipDectObser.slipflg[i] >= 100)
                {
                   // fprintf(pFileCS[4],"%s%.1f %s%d 0.5cycle not repair\n","epoch=",slipDectObser.dEpochTime,"satid=",i+1);
                }
                else
                {
                    CycleslipRepair_TD(&slipDectObser, i, g_deltaNTF[i],pFileCS);

                    if (abs(g_deltaNTF[i][slipDectObser.iFreq[i].dFreqFlg[0]]) > EPSILON)
                    {
                        g_tSatState[i].iCycleslipNum[slipDectObser.iFreq[i].dFreqFlg[0]]++;
                    }

                    if (abs(g_deltaNTF[i][slipDectObser.iFreq[i].dFreqFlg[1]]) > EPSILON)
                    {
                        g_tSatState[i].iCycleslipNum[slipDectObser.iFreq[i].dFreqFlg[1]]++;
                    }

                    if (abs(g_deltaNTF[i][slipDectObser.iFreq[i].dFreqFlg[2]]) > EPSILON)
                    {
                        g_tSatState[i].iCycleslipNum[slipDectObser.iFreq[i].dFreqFlg[2]]++;
                    }
                }
            }
        }
    }

    //多路径分析
    for (i=0;i<ptRtcm->tObs.iN;i++)
    {
        satId = ptRtcm->tObs.ptData[i].iSat - 1;//卫星编号从1开始

        if ((slipDectObser.iFreq[satId].iNum >=2) && (!g_tSatState[satId].iEphFlg))
        {
            MPAnalysOneSat(&slipDectObser,satId,pFileCS);
        }
        else
        {
            for (l=0;l<NFREQ+NEXOBS;l++)
            {
                g_tSatState[satId].fMP[l] = 0;
                g_tSatState[satId].iMPCounter[l] = 0;
                g_tSatState[satId].fMPMean[l] = 0;
            }
            memset(g_fMPWin[j],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
        }
    }
    memcpy(&g_slipDectObser,&slipDectObser,sizeof(slipDectObser));
}

/*******************************************************************************
* 函数名称： Cycleslip()
* 功能描述： 实现探测周跳
* 输入参数： pObsEpoch——经过处理的原始观测值
* 输出参数：
* 返 回 值： slipflg--当前历元观测值的周跳标志位
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void Cycleslip(ObserDataEpoch *pObsEpoch, FILE **pFileCS)
{
    INT8U i,j,l;

    for (i=0; i<MAXSAT; i++)
    {
        if(pObsEpoch->validSatflg[i])//判断本历元是否有i号星
        {
            if (pObsEpoch->iFreq[i].iNum == 0)
            {
                g_iCycleFlg[i] = -1;
                //fprintf(pFileCS[4],"epoch %.1f %s%d data is not complete\n",pObsEpoch->dEpochTime,"satid=",i+1);
            }
            else if (pObsEpoch->iFreq[i].iNum == 1)
            {
                g_iCycleFlg[i] = 2;
                //fprintf(pFileCS[4],"epoch %.1f %s%d data is single frequency\n",pObsEpoch->dEpochTime,"satid=",i+1);
            }
            else if (pObsEpoch->iFreq[i].iNum == 2)
            {
                HMW_DD(pObsEpoch, i, pFileCS);
                IonoRes_DD(pObsEpoch, i, pFileCS);
            }
            else if (pObsEpoch->iFreq[i].iNum == 3)
            {
                if ((i+1) <= (NSATGPS+NSATBDS))
                {
                    EWL_DD(pObsEpoch, i, pFileCS);
                    WL_DD(pObsEpoch, i, pFileCS);
                    NL_DD(pObsEpoch, i, pFileCS);
                }
                else
                {
                    g_iCycleFlg[i] = 2;
                    //fprintf(pFileCS[4],"epoch %.1f %s%d Glonass triple data\n",pObsEpoch->dEpochTime,"satid=",i+1);
                }
            }

            if (pObsEpoch->slipflg[i])
            {
                g_iCycleFlg[i] = 1;

                /*for (l=0;l<pObsEpoch->iFreq[i].iNum;l++)
                {
                    g_tSatState[i].iCycleslipNum[pObsEpoch->iFreq[i].dFreqFlg[l]]++;
                }	*/

                for (l=0;l<NFREQ+NEXOBS;l++)//有周跳则清空多路径指标数据,从当前历元开始累积
                {
                    g_tSatState[i].fMP[l] = 0;
                    g_tSatState[i].iMPCounter[l] = 1;
                    g_tSatState[i].fMPMean[l] = 0;
                }
                memset(g_fMPWin[i],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
            }
        }
        else if(g_slipDectObser.validSatflg[i])//本历元无i号星，上历元有i号星
        {
            g_slipDectObser.detValidflg[i] = 0;
            g_TFIonRes[i] = 0;

            for (j=0;j<3;j++)
            {
                g_TFRes[i][j] = 0;
                g_deltaNTF[i][j] = 0;
            }

            for (l=0;l<NFREQ+NEXOBS;l++)//有周跳则清空多路径指标数据,从当前历元开始累积
            {
                g_tSatState[i].fMP[l] = 0;
                g_tSatState[i].iMPCounter[l] = 0;
                g_tSatState[i].fMPMean[l] = 0;
            }
            memset(g_fMPWin[i],0,(NFREQ+NEXOBS)*SlidWinNum*sizeof(FP64));
        }
    }
}

/*******************************************************************************
* 函数名称： satwavelen()
* 功能描述： 计算卫星波长
* 输入参数： sat——卫星号
                    frq——频率
* 输出参数：
* 返 回 值：波长
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
double satwavelen(int sat, int frq, int iGFrq)
{
    const double freq_glo[]={FREQ1_GLO,FREQ2_GLO};
    const double dfrq_glo[]={DFRQ1_GLO,DFRQ2_GLO};
    INT32U sys,iPrn;

    sys=Sat2Sys(sat+1,&iPrn);

    if (sys==SYS_GLO) {
        if (0<=frq&&frq<=1) {
            return CLIGHT/(freq_glo[frq]+dfrq_glo[frq]*iGFrq);
        }
        else if (frq==2) { /* L3 */
            return CLIGHT/FREQ3_GLO;
        }
    }
    else if (sys==SYS_BDS) {
        if      (frq==0) return CLIGHT/FREQ2_BDS; /* B1 */
        else if (frq==1) return CLIGHT/FREQ7_BDS; /* B2 */
        else if (frq==2) return CLIGHT/FREQ6_BDS; /* B3 */
    }
    else {
        if      (frq==0) return CLIGHT/FREQ1_GPS; /* L1 */
        else if (frq==1) return CLIGHT/FREQ2_GPS; /* L2 */
        else if (frq==2) return CLIGHT/FREQ5_GPS; /* L5 */
    }
    return 0.0;
}

/*******************************************************************************
* 函数名称： satwavelen()
* 功能描述： 计算卫星波长
* 输入参数： sat——卫星号
                    frq——频率
* 输出参数：
* 返 回 值：波长
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
double satFreq(int sat, int frq, int iGFrq)
{
    const double freq_glo[]={FREQ1_GLO,FREQ2_GLO};
    const double dfrq_glo[]={DFRQ1_GLO,DFRQ2_GLO};
    INT32U sys,iPrn;

    sys=Sat2Sys(sat+1,&iPrn);

    if (sys==SYS_GLO) {
        if (0<=frq&&frq<=1) {
            return (freq_glo[frq]+dfrq_glo[frq]*iGFrq);
        }
        else if (frq==2) { /* L3 */
            return FREQ3_GLO;
        }
    }
    else if (sys==SYS_BDS) {
        if      (frq==0) return FREQ2_BDS; /* B1 */
        else if (frq==1) return FREQ7_BDS; /* B2 */
        else if (frq==2) return FREQ6_BDS; /* B3 */
    }
    else {
        if      (frq==0) return FREQ1_GPS; /* L1 */
        else if (frq==1) return FREQ2_GPS; /* L2 */
        else if (frq==2) return FREQ5_GPS; /* L5 */
    }
    return 0.0;
}

/*******************************************************************************
* 函数名称： IonoRes_DD()
* 功能描述： 实现电离层残差法周跳探测
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值：无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void IonoRes_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 fDeltaIonRes = 0;/* 电离层残差组合周跳检验量 */
    INT8U iFreq1 = 0, iFreq2 = 0, iFlg = 0;
    FP64 fLam1 = 0, fLam2 = 0;
    int i;

    if(g_slipDectObser.validSatflg[satid] == 0)//上个历元没有本卫星
    {
        return;
    }
    else
    {
        if (g_slipDectObser.iFreq[satid].iNum >= 2)
        {
            iFreq1 = pObsEpoch->iFreq[satid].dFreqFlg[0];
            iFreq2 = pObsEpoch->iFreq[satid].dFreqFlg[1];

            for (i=0;i<g_slipDectObser.iFreq[satid].iNum;i++)
            {
                if (iFreq1 == g_slipDectObser.iFreq[satid].dFreqFlg[i])
                {
                    iFlg ++;
                    break;
                }
            }

            for (i=0;i<g_slipDectObser.iFreq[satid].iNum;i++)
            {
                if (iFreq2 == g_slipDectObser.iFreq[satid].dFreqFlg[i])
                {
                    iFlg ++;
                    break;
                }
            }

            if (iFlg == 2)
            {
                fLam1 = satwavelen(satid, iFreq1, pObsEpoch->iGFrq[satid]);
                fLam2 = satwavelen(satid, iFreq2, pObsEpoch->iGFrq[satid]);

                //本历元-上历元
                fDeltaIonRes = (pObsEpoch->phaseValue[iFreq1][satid]*fLam1 - pObsEpoch->phaseValue[iFreq2][satid]*fLam2) -\
                    (g_slipDectObser.phaseValue[iFreq1][satid]*fLam1 - g_slipDectObser.phaseValue[iFreq2][satid]*fLam2);

                pObsEpoch->detValidflg[satid] = 1;

                //fprintf(pFileCS[4],"%s%d %s%.5f\n","satid=",satid+1,"fDeltaIonRes=",fDeltaIonRes);

                if (fabs(fDeltaIonRes)-0.035 > 0) //0.099)/* threshold 0.07(empirical value) */
                {
                    pObsEpoch->slipflg[satid] ++; //有周跳
                    //fprintf(pFileCS[4],"%s%.1f %s%d %s%.5f %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fDeltaIonRes=",fDeltaIonRes,"P1=",pObsEpoch->rangeValue[iFreq1][satid]);
                }
            }
            else
            {
                g_iCycleFlg[satid] = 2;
                //fprintf(pFileCS[4],"epoch %.1f Freq not match, %s%d %s%d %s%d %s%d %s%d\n",pObsEpoch->dEpochTime,"satid=",satid+1,\
                //	    "Frq1",g_slipDectObser.iFreq[satid].dFreqFlg[0],"Frq2",g_slipDectObser.iFreq[satid].dFreqFlg[1],"Frq1",iFreq1,"Frq2",iFreq2);
            }
        }
        else
        {
            g_iCycleFlg[satid] = 2;
            //fprintf(pFileCS[4],"the epoch before is single but this epoch %.1f is dual, %s%d\n",pObsEpoch->dEpochTime,"satid=",satid+1);
        }
    }
}

/*******************************************************************************
* 函数名称： HMW_DD()
* 功能描述： 实现HMW（Hatch-Melbourne-Wübbena）组合探测周跳
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值： 无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void HMW_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 fHMWRes = 0;
    FP64 fN_true = 0;
    FP64 fN_before = 0;
    INT8U iFreq1 = 0, iFreq2 = 0, iFlg = 0;
    FP64 fFreq1 = 0, fFreq2 = 0;
    FP64 fLam1 = 0, fLam2 = 0;
    FP64 WAVE_WL;
    int i;

    if(g_slipDectObser.validSatflg[satid] == 0)//该卫星第一次出现
    {
        return;
    }
    else
    {
        if (g_slipDectObser.iFreq[satid].iNum >= 2)
        {
            iFreq1 = pObsEpoch->iFreq[satid].dFreqFlg[0];
            iFreq2 = pObsEpoch->iFreq[satid].dFreqFlg[1];

            for (i=0;i<g_slipDectObser.iFreq[satid].iNum;i++)
            {
                if (iFreq1 == g_slipDectObser.iFreq[satid].dFreqFlg[i])
                {
                    iFlg ++;
                    break;
                }
            }

            for (i=0;i<g_slipDectObser.iFreq[satid].iNum;i++)
            {
                if (iFreq2 == g_slipDectObser.iFreq[satid].dFreqFlg[i])
                {
                    iFlg ++;
                    break;
                }
            }

            if (iFlg == 2)
            {
                fLam1 = satwavelen(satid, iFreq1, pObsEpoch->iGFrq[satid]);
                fLam2 = satwavelen(satid, iFreq2, pObsEpoch->iGFrq[satid]);

                fFreq1 = satFreq(satid, iFreq1, pObsEpoch->iGFrq[satid]);
                fFreq2 = satFreq(satid, iFreq2, pObsEpoch->iGFrq[satid]);

                WAVE_WL = CLIGHT/(fFreq1 - fFreq2);

                //计算当前历元MW组合模糊度,以米为单位
                fN_true = (fFreq1*pObsEpoch->rangeValue[iFreq1][satid] + fFreq2*pObsEpoch->rangeValue[iFreq2][satid])/ \
                    ((fFreq1 + fFreq2)*WAVE_WL) - (pObsEpoch->phaseValue[iFreq1][satid] - pObsEpoch->phaseValue[iFreq2][satid]);

                fN_before = (fFreq1*g_slipDectObser.rangeValue[iFreq1][satid] + fFreq2*g_slipDectObser.rangeValue[iFreq2][satid])/ \
                    ((fFreq1 + fFreq2)*WAVE_WL) - (g_slipDectObser.phaseValue[iFreq1][satid] - g_slipDectObser.phaseValue[iFreq2][satid]);

                fHMWRes = fN_true - fN_before;

                if ((fabs(fHMWRes)-0.45>=0)&&(fabs(fHMWRes)-0.55<=0))
                {
                    pObsEpoch->slipflg[satid] =100; //有周跳
                }
                else if (fabs(fHMWRes)-0.7 > 0)
                {
                    pObsEpoch->slipflg[satid] ++; //有周跳
                }
            }
            else
            {
                g_iCycleFlg[satid] = 2;
            }
        }
        else
        {
            g_iCycleFlg[satid] = 2;
        }
    }
}

/*******************************************************************************
* 函数名称： EWL_DD()
* 功能描述： 利用EWL 组合探测周跳
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值： 无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void EWL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 fEMWRes = 0;
    FP64 fN_true = 0;
    FP64 fN_before = 0;
    FP64 WAVE_EWL_GPS = 0;
    FP64 WAVE_EWL_BDS = 0;

    WAVE_EWL_BDS = CLIGHT/(FREQ6_BDS - FREQ7_BDS);
    WAVE_EWL_GPS = CLIGHT/(FREQ2_GPS - FREQ5_GPS);

    if(g_slipDectObser.validSatflg[satid] == 0)//该卫星第一次出现
    {
        return;
    }
    else
    {
        if (g_slipDectObser.iFreq[satid].iNum == 3)
        {
            //计算当前历元EWL组合模糊度,以周为单位
            if (satid < NSATGPS)
            {
                fN_true = ((FREQ2_GPS*pObsEpoch->rangeValue[Freq2][satid] + FREQ5_GPS*pObsEpoch->rangeValue[Freq3][satid])/(FREQ2_GPS + FREQ5_GPS) - \
                    (FREQ2_GPS*pObsEpoch->phaseValue[Freq2][satid]*dLamGPS[1] - FREQ5_GPS*pObsEpoch->phaseValue[Freq3][satid]*dLamGPS[2])/(FREQ2_GPS - FREQ5_GPS))/WAVE_EWL_GPS;
                //fprintf(pFileCS,"%s%.1f %s%d %s%d %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"type=",type,"satid=",satid,"N-true=",fN_true);

                fN_before =  ((FREQ2_GPS*g_slipDectObser.rangeValue[Freq2][satid] + FREQ5_GPS*g_slipDectObser.rangeValue[Freq3][satid])/(FREQ2_GPS + FREQ5_GPS) - \
                    (FREQ2_GPS*g_slipDectObser.phaseValue[Freq2][satid]*dLamGPS[1] - FREQ5_GPS*g_slipDectObser.phaseValue[Freq3][satid]*dLamGPS[2])/(FREQ2_GPS - FREQ5_GPS))/WAVE_EWL_GPS;
            }
            else if (satid < NSATGPS + NSATBDS)
            {
                fN_true = ((FREQ7_BDS*pObsEpoch->rangeValue[Freq2][satid] + FREQ6_BDS*pObsEpoch->rangeValue[Freq3][satid])/(FREQ7_BDS + FREQ6_BDS) - \
                    (FREQ7_BDS*pObsEpoch->phaseValue[Freq2][satid]*dLamBDS[1] - FREQ6_BDS*pObsEpoch->phaseValue[Freq3][satid]*dLamBDS[2])/(FREQ7_BDS - FREQ6_BDS))/WAVE_EWL_BDS;
                //fprintf(pFileCS,"%s%.1f %s%d %s%d %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"type=",type,"satid=",satid,"N-true=",fN_true);

                fN_before =  ((FREQ7_BDS*g_slipDectObser.rangeValue[Freq2][satid] + FREQ6_BDS*g_slipDectObser.rangeValue[Freq3][satid])/(FREQ7_BDS + FREQ6_BDS) - \
                    (FREQ7_BDS*g_slipDectObser.phaseValue[Freq2][satid]*dLamBDS[1] - FREQ6_BDS*g_slipDectObser.phaseValue[Freq3][satid]*dLamBDS[2])/(FREQ7_BDS - FREQ6_BDS))/WAVE_EWL_BDS;
            }

            fEMWRes = fN_true - fN_before;

            if (fEMWRes > 0)
            {
                g_TFRes[satid][0] = (INT32S)(fEMWRes+0.5);
            }
            else if (fEMWRes < 0)
            {
                g_TFRes[satid][0] = (INT32S)(fEMWRes-0.5);
            }
            else
            {
                g_TFRes[satid][0] = 0;
            }

            //if (satid == 0)
            //fprintf(pFileCS[4],"$%s%.1f %s%d %s%.4f  ","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fEWLRes=",fEMWRes);

            if ((fabs(fEMWRes)-0.45>=0)&&(fabs(fEMWRes)-0.55<=0))
            {
                pObsEpoch->slipflg[satid] =100; //有周跳
                //fprintf(pFileCS[4],"%s%.1f %s%d 0.5cycle %s%.4f %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fEWLRes=",fEMWRes,"P1=",pObsEpoch->rangeValue[Freq1][satid]);
            }
            else if (fabs(fEMWRes)-0.55 > 0)
            {
                pObsEpoch->slipflg[satid] ++; //有周跳
                //fprintf(pFileCS[4],"%s%.1f %s%d %s%.4f %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fEWLRes=",fEMWRes,"P1=",pObsEpoch->rangeValue[Freq1][satid]);
            }
        }
        else
        {
            //g_iCycleFlg[satid] = 2;
            //fprintf(pFileCS[4],"the epoch before is dual but this epoch %.1f is triple, %s%d\n",pObsEpoch->dEpochTime,"satid=",satid+1);
            return;
        }
    }
}

/*******************************************************************************
* 函数名称： WL_DD()
* 功能描述： 利用WL 组合探测周跳
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值： 无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void WL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 fWLRes = 0;
    FP64 fdeltaFi1 = 0;
    FP64 fdeltaFi2 = 0;
    FP64 WAVE_WL_BDS = 0;
    FP64 WAVE_EWL_GPS = 0;
    FP64 WAVE_EWL_BDS = 0;
    FP64 WAVE_WL_GPS = 0;

    WAVE_EWL_BDS = CLIGHT/(FREQ6_BDS - FREQ7_BDS);
    WAVE_EWL_GPS = CLIGHT/(FREQ2_GPS - FREQ5_GPS);

    WAVE_WL_BDS = CLIGHT/(FREQ2_BDS - FREQ7_BDS);
    WAVE_WL_GPS = CLIGHT/(FREQ1_GPS - FREQ5_GPS);

    if(g_slipDectObser.validSatflg[satid] == 0)//该卫星第一次出现
    {
        return;
    }
    else
    {
        if (g_slipDectObser.iFreq[satid].iNum == 3)
        {
            if (satid < NSATGPS)
            {
                fdeltaFi1 = (pObsEpoch->phaseValue[Freq2][satid] - pObsEpoch->phaseValue[Freq3][satid])*WAVE_EWL_GPS/WAVE_WL_GPS - \
                    (pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq3][satid]);

                fdeltaFi2 = (g_slipDectObser.phaseValue[Freq2][satid] - g_slipDectObser.phaseValue[Freq3][satid])*WAVE_EWL_GPS/WAVE_WL_GPS - \
                    (g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq3][satid]);

                fWLRes = fdeltaFi1 - fdeltaFi2 + WAVE_EWL_GPS*g_TFRes[satid][0]/WAVE_WL_GPS;
            }
            else if (satid < NSATGPS + NSATBDS)
            {
                fdeltaFi1 = (pObsEpoch->phaseValue[Freq3][satid] - pObsEpoch->phaseValue[Freq2][satid])*WAVE_EWL_BDS/WAVE_WL_BDS - \
                    (pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq2][satid]);

                fdeltaFi2 = (g_slipDectObser.phaseValue[Freq3][satid] - g_slipDectObser.phaseValue[Freq2][satid])*WAVE_EWL_BDS/WAVE_WL_BDS - \
                    (g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq2][satid]);

                fWLRes = fdeltaFi1 - fdeltaFi2 + WAVE_EWL_BDS*g_TFRes[satid][0]/WAVE_WL_BDS;
            }

            if (fWLRes > 0)
            {
                g_TFRes[satid][1] = (INT32S)(fWLRes+0.5);
            }
            else if (fWLRes < 0)
            {
                g_TFRes[satid][1] = (INT32S)(fWLRes-0.5);
            }
            else
            {
                g_TFRes[satid][1] = 0;
            }

            //if (satid == 0)
            //fprintf(pFileCS[4],"%s%d %s%.4f  ","satid=",satid+1,"fWLRes=",fWLRes);

            if ((fabs(fWLRes)-0.45>=0)&&(fabs(fWLRes)-0.55<=0))
            {
                pObsEpoch->slipflg[satid] =100; //有周跳
                //fprintf(pFileCS[4],"%s%.1f %s%d 0.5cycle %s%.4f %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fWLRes=",fWLRes,"P1=",pObsEpoch->rangeValue[Freq1][satid]);
            }
            else if (fabs(fWLRes)-0.55 > 0)
            {
                pObsEpoch->slipflg[satid] ++; //有周跳
                //fprintf(pFileCS[4],"%s%.1f %s%d %s%.4f %s%.3f\n","epoch=",pObsEpoch->dEpochTime,"satid=",satid+1,"fWLRes=",fWLRes,"P1=",pObsEpoch->rangeValue[Freq1][satid]);
            }
        }
        else
        {
            //g_iCycleFlg[satid] = 2;
            //fprintf(pFileCS[4],"the epoch before is dual but this epoch %.1f is triple, %s%d\n",pObsEpoch->dEpochTime,"satid=",satid+1);
            return;
        }
    }
}

/*******************************************************************************
* 函数名称： NL_DD()
* 功能描述： 利用WL 组合探测周跳
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值： 无
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void NL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 fNLRes = 0;
    FP64 fdeltaIon12 = 0, fdeltaIon1 = 0, fdeltaIon2 = 0, fdeltaIon3 = 0, fdeltaIon4 = 0, fdeltaIon13 = 0;
    FP64 fdeltaFi1 = 0;
    FP64 fdeltaFi2 = 0;
    FP64 WAVE_NL_BDS = 0;
    FP64 WAVE_NL_GPS = 0;
    FP64 WAVE_WL_BDS = 0;
    FP64 WAVE_WL_GPS = 0;
    FP64 isf12_BDS = 0, isf13_BDS = 0;
    FP64 isf12_GPS = 0, isf13_GPS = 0;

    WAVE_WL_BDS = CLIGHT/(FREQ2_BDS - FREQ7_BDS);
    WAVE_WL_GPS = CLIGHT/(FREQ1_GPS - FREQ5_GPS);

    WAVE_NL_BDS = CLIGHT/(2*FREQ2_BDS-FREQ6_BDS) ;
    WAVE_NL_GPS = CLIGHT/(2*FREQ1_GPS-FREQ2_GPS) ;
    isf12_BDS = (FREQ2_BDS*FREQ2_BDS)/(FREQ7_BDS*FREQ7_BDS);
    isf12_GPS = (FREQ1_GPS*FREQ1_GPS)/(FREQ2_GPS*FREQ2_GPS);
    isf13_BDS = (FREQ2_BDS*FREQ2_BDS)/(FREQ6_BDS*FREQ6_BDS);
    isf13_GPS = (FREQ1_GPS*FREQ1_GPS)/(FREQ5_GPS*FREQ5_GPS);


    if(g_slipDectObser.validSatflg[satid] == 0)//该卫星第一次出现
    {
        return;
    }
    else
    {
        if (g_slipDectObser.iFreq[satid].iNum == 3)
        {
            if (pObsEpoch->slipflg[satid])
            {
                if (fabs(g_TFIonRes[satid]) < EPSILON)
                {
                    if ((fabs(g_TFRes[satid][0]) < EPSILON) && (fabs(g_TFRes[satid][1]) > EPSILON))
                    {
                        g_tSatState[satid].iCycleslipNum[2]++;
                    }
                    else if ((fabs(g_TFRes[satid][0]) > EPSILON) && (fabs(g_TFRes[satid][1]) < EPSILON))
                    {
                        g_tSatState[satid].iCycleslipNum[1]++;
                    }
                    else
                    {
                        g_tSatState[satid].iCycleslipNum[0]++;
                        g_tSatState[satid].iCycleslipNum[1]++;
                        g_tSatState[satid].iCycleslipNum[2]++;
                    }
                    pObsEpoch->slipflg[satid] = 0;
                    return;
                }
                else
                {
                    fdeltaIon12 = g_TFIonRes[satid];
                }
            }
            else
            {
                if (satid < NSATGPS)
                {
                    fdeltaIon1 = (dLamGPS[0]*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq2][satid]*dLamGPS[1])/(1 - isf12_GPS);
                    fdeltaIon2 = (dLamGPS[0]*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq2][satid]*dLamGPS[1])/(1 - isf12_GPS);
                    fdeltaIon3 = (dLamGPS[0]*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq3][satid]*dLamGPS[2])/(1 - isf13_GPS);
                    fdeltaIon4 = (dLamGPS[0]*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq3][satid]*dLamGPS[2])/(1 - isf13_GPS);
                }
                else if (satid < NSATGPS + NSATBDS)
                {
                    fdeltaIon1 = (dLamBDS[0]*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq2][satid]*dLamBDS[1])/(1 - isf12_BDS);
                    fdeltaIon2 = (dLamBDS[0]*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq2][satid]*dLamBDS[1])/(1 - isf12_BDS);
                    fdeltaIon3 = (dLamBDS[0]*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq3][satid]*dLamBDS[2])/(1 - isf13_BDS);
                    fdeltaIon4 = (dLamBDS[0]*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq3][satid]*dLamBDS[2])/(1 - isf13_BDS);
                }

                fdeltaIon12 = fdeltaIon1 - fdeltaIon2;
                fdeltaIon13 = fdeltaIon3 - fdeltaIon4;

                fdeltaIon12 = 0.5*(fdeltaIon12 + fdeltaIon13);

                if (fabs(fdeltaIon12 - g_TFIonRes[satid])>0.06)
                {
                    fdeltaIon12 = g_TFIonRes[satid];
                }
                else
                {
                    g_TFIonRes[satid] = fdeltaIon12;
                }
            }

            if (satid < NSATGPS)
            {
                fdeltaFi1 = (pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq3][satid])*WAVE_WL_GPS/WAVE_NL_GPS - \
                    (2*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq2][satid]);

                fdeltaFi2 = (g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq3][satid])*WAVE_WL_GPS/WAVE_NL_GPS - \
                    (2*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq2][satid]);

                fNLRes = fdeltaFi1 - fdeltaFi2 + 1.926*fdeltaIon12/WAVE_NL_GPS + WAVE_WL_GPS*g_TFRes[satid][1]/WAVE_NL_GPS;	//GPS   1.926
            }
            else if (satid < NSATGPS + NSATBDS)
            {
                fdeltaFi1 = (pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq2][satid])*WAVE_WL_BDS/WAVE_NL_BDS - \
                    (2*pObsEpoch->phaseValue[Freq1][satid] - pObsEpoch->phaseValue[Freq3][satid]);

                fdeltaFi2 = (g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq2][satid])*WAVE_WL_BDS/WAVE_NL_BDS - \
                    (2*g_slipDectObser.phaseValue[Freq1][satid] - g_slipDectObser.phaseValue[Freq3][satid]);

                fNLRes = fdeltaFi1 - fdeltaFi2 + 1.941*fdeltaIon12/WAVE_NL_BDS + WAVE_WL_BDS*g_TFRes[satid][1]/WAVE_NL_BDS;	//GPS   1.926
            }

            if (fNLRes > 0)
            {
                g_TFRes[satid][2] = (INT32S)(fNLRes+0.5);
            }
            else if (fNLRes < 0)
            {
                g_TFRes[satid][2] = (INT32S)(fNLRes-0.5);
            }
            else
            {
                g_TFRes[satid][2] = 0;
            }

            if ((fabs(fNLRes)-0.45>=0)&&(fabs(fNLRes)-0.55<=0))
            {
                pObsEpoch->slipflg[satid] =100; //有周跳
            }
            else if (fabs(fNLRes)-0.55 > 0)
            {
                pObsEpoch->slipflg[satid] ++; //有周跳
            }
        }
        else
        {
            if (g_slipDectObser.iFreq[satid].iNum == 2)
            {
                pObsEpoch->iFreq[satid].iNum = 2;

                for (int i=0;i<g_slipDectObser.iFreq[satid].iNum;i++)
                {
                    pObsEpoch->iFreq[satid].dFreqFlg[i] = g_slipDectObser.iFreq[satid].dFreqFlg[i];
                }

                HMW_DD(pObsEpoch, satid);
                IonoRes_DD(pObsEpoch, satid);
            }
            else
            {
                g_iCycleFlg[satid] = 2;
            }
        }
    }
}

/*******************************************************************************
* 函数名称： CycleslipRepair_DD()
* 功能描述： 实现双频周跳修复
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值：
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void CycleslipRepair_DD(ObserDataEpoch *pObsEpoch, INT8U satid, INT32S deltaN[],FILE **pFileCS)
{
    FP64 fHMWRes = 0;  /* HMW 组合周跳检验量 */
    FP64 fDeltaIonRes = 0;/* 电离层残差组合周跳检验量 */
    FP64 fHMWRes1 = 0;
    FP64 fHMWRes2 = 0;
    FP64 slipN[2],fdeltaN[2];
    FP64 A[4],Ainv[4];
    INT8U iFreq1 = 0, iFreq2 = 0;
    FP64 fFreq1 = 0, fFreq2 = 0;
    FP64 fLam1 = 0, fLam2 = 0;
    FP64 WAVE_WL;

    iFreq1 = pObsEpoch->iFreq[satid].dFreqFlg[0];
    iFreq2 = pObsEpoch->iFreq[satid].dFreqFlg[1];

    fLam1 = satwavelen(satid, iFreq1, pObsEpoch->iGFrq[satid]);
    fLam2 = satwavelen(satid, iFreq2, pObsEpoch->iGFrq[satid]);

    fFreq1 = satFreq(satid, iFreq1, pObsEpoch->iGFrq[satid]);
    fFreq2 = satFreq(satid, iFreq2, pObsEpoch->iGFrq[satid]);

    WAVE_WL = CLIGHT/(fFreq1 - fFreq2);

    //计算当前历元HMW组合模糊度
    fHMWRes1 = (pObsEpoch->phaseValue[iFreq1][satid] - pObsEpoch->phaseValue[iFreq2][satid])-(fFreq1*pObsEpoch->rangeValue[iFreq1][satid] + \
                     fFreq2*pObsEpoch->rangeValue[iFreq2][satid])/((fFreq1 + fFreq2)*WAVE_WL);
    //计算上一历元HMW组合模糊度
    fHMWRes2 = (g_slipDectObser.phaseValue[iFreq1][satid] - g_slipDectObser.phaseValue[iFreq2][satid])-(fFreq1*g_slipDectObser.rangeValue[iFreq1][satid] +\
                       fFreq2*g_slipDectObser.rangeValue[iFreq2][satid])/((fFreq1 + fFreq2)*WAVE_WL);
    fHMWRes = fHMWRes1 - fHMWRes2;

    //本历元-上历元电离层残差组合
    fDeltaIonRes = (pObsEpoch->phaseValue[iFreq1][satid]- fFreq1*pObsEpoch->phaseValue[iFreq2][satid] / fFreq2)-\
                       (g_slipDectObser.phaseValue[iFreq1][satid]- fFreq1*g_slipDectObser.phaseValue[iFreq2][satid] / fFreq2);

    A[0] = 1;A[1] = -1;A[2] = 1;A[3] = -fFreq1/fFreq2;

    slipN[0] = fHMWRes;
    slipN[1] = fDeltaIonRes;

    MatInv(A,2,Ainv);
    MatMul(Ainv,slipN,2,2,1,fdeltaN);

    if (fdeltaN[0] > 0)
    {
        deltaN[iFreq1] = (INT32S)(fdeltaN[0]+0.5);
    }
    else if (fdeltaN[0] < 0)
    {
        deltaN[iFreq1] = (INT32S)(fdeltaN[0]-0.5);
    }
    else
    {
        deltaN[iFreq1] = 0;
    }

    if (fdeltaN[1] > 0)
    {
        deltaN[iFreq2] = (INT32S)(fdeltaN[1]+0.5);
    }
    else if (fdeltaN[1] < 0)
    {
        deltaN[iFreq2] = (INT32S)(fdeltaN[1]-0.5);
    }
    else
    {
        deltaN[iFreq2] = 0;
    }
}

/*******************************************************************************
* 函数名称： CycleslipRepair_TD()
* 功能描述： 实现三频周跳修复
* 输入参数： pObsEpoch——经过处理的原始观测值
satid——卫星序号
type——测站标志
deltaN[]——周跳
* 输出参数： 当前历元观测值的周跳标志位
* 返 回 值：
* 其它说明：
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
* 2015/08/14 V1.0.0.0
*******************************************************************************/
void CycleslipRepair_TD(ObserDataEpoch *pObsEpoch, INT8U satid, INT32S deltaN[],FILE **pFileCS)
{
    FP64 slipN[3],fdeltaN[3];
    FP64 A[9];
    INT8U i;

    for (i=0;i<3;i++)
    {
        slipN[i] = g_TFRes[satid][i];
    }

    A[0] = 1;A[1] = -1;A[2] = 1;
    A[3] = 2;A[4] = -2;A[5] = 1;
    A[6] = 1;A[7] = -2;A[8] = 1;

    MatMul(A,slipN,3,3,1,fdeltaN);

    if (satid < NSATGPS)
    {
        deltaN[0] = -fdeltaN[0];
        deltaN[1] = -fdeltaN[1];
        deltaN[2] = -fdeltaN[2];
    }
    else if (satid < NSATGPS + NSATBDS)
    {
        deltaN[0] = -fdeltaN[0];
        deltaN[1] = -fdeltaN[2];
        deltaN[2] = -fdeltaN[1];
    }
}

void MPAnalysOneSat(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **)
{
    FP64 faphai[2], fLam[NFREQ], fMp[NFREQ];
    INT8U i;
    INT32U j;
    INT8U iFreq[NFREQ];
    FP64 fFrq, fFrq0;
    FP64 fMPtotal = 0;

    for (i=0;i<pObsEpoch->iFreq[satid].iNum;i++)
    {
        iFreq[i] = pObsEpoch->iFreq[satid].dFreqFlg[i];
        fLam[iFreq[i]] = satwavelen(satid, iFreq[i], pObsEpoch->iGFrq[satid]);
    }

    if (iFreq[0])
    {
        //fprintf(ofp[5],"%s%d donot have freq0 ","satid=",satid+1);
    }
    else
    {
        fFrq0 = satFreq(satid, 0, pObsEpoch->iGFrq[satid]);

        for (i=0;i<pObsEpoch->iFreq[satid].iNum-1;i++)
        {
            fFrq = satFreq(satid, iFreq[i+1], pObsEpoch->iGFrq[satid]);
            faphai[i] = (fFrq0*fFrq0) / (fFrq*fFrq);
        }

        if ((g_slipDectObser.iFreq[satid].iNum == 3)&&(pObsEpoch->iFreq[satid].iNum == 2))
        {
            if (pObsEpoch->iFreq[satid].dFreqFlg[1] == 2)
            {
                g_tSatState[satid].iMPCounter[0] = 1;
                g_tSatState[satid].fMPMean[0] = 0;
                g_tSatState[satid].fMP[0] = 0;
                memset(g_fMPWin[satid][0],0,SlidWinNum*sizeof(FP64));
                g_tSatState[satid].iMPCounter[1] = 0;
                g_tSatState[satid].fMPMean[1] = 0;
                g_tSatState[satid].fMP[1] = 0;
                memset(g_fMPWin[satid][1],0,SlidWinNum*sizeof(FP64));
            }
            else if (pObsEpoch->iFreq[satid].dFreqFlg[1] == 1)
            {
                g_tSatState[satid].iMPCounter[2] = 0;
                g_tSatState[satid].fMPMean[2] = 0;
                g_tSatState[satid].fMP[2] = 0;
                memset(g_fMPWin[satid][2],0,SlidWinNum*sizeof(FP64));
            }
        }

        fMp[0] = pObsEpoch->rangeValue[0][satid] + ((1+faphai[0])*pObsEpoch->phaseValue[0][satid]*fLam[0])/(1-faphai[0]) \
                  - (2*pObsEpoch->phaseValue[iFreq[1]][satid]*fLam[iFreq[1]])/(1-faphai[0]);

        for (i=0;i<pObsEpoch->iFreq[satid].iNum-1;i++)
        {
            fMp[iFreq[i+1]] = pObsEpoch->rangeValue[iFreq[i+1]][satid] + (2*faphai[i]*pObsEpoch->phaseValue[0][satid]*fLam[0])/(1-faphai[i]) \
                  - ((1+faphai[i])*pObsEpoch->phaseValue[iFreq[i+1]][satid]*fLam[iFreq[i+1]])/(1-faphai[i]);
        }

        for (i=0;i<pObsEpoch->iFreq[satid].iNum;i++)
        {
            if (g_tSatState[satid].iMPCounter[iFreq[i]] <= SlidWinNum)
            {
                g_fMPWin[satid][iFreq[i]][g_tSatState[satid].iMPCounter[iFreq[i]]-1] = fMp[iFreq[i]];
                fMPtotal = 0;
                for (j=0;j<g_tSatState[satid].iMPCounter[iFreq[i]];j++)
                {
                    fMPtotal += g_fMPWin[satid][iFreq[i]][j];
                }
                g_tSatState[satid].fMPMean[iFreq[i]] = fMPtotal/g_tSatState[satid].iMPCounter[iFreq[i]];
                g_tSatState[satid].fMP[iFreq[i]] = fMp[iFreq[i]] - g_tSatState[satid].fMPMean[iFreq[i]];
            }
            else
            {
                for (j=0;j<SlidWinNum-1;j++)
                {
                    g_fMPWin[satid][iFreq[i]][j] = g_fMPWin[satid][iFreq[i]][j+1];
                }
                g_fMPWin[satid][iFreq[i]][SlidWinNum-1] = fMp[iFreq[i]];
                fMPtotal = 0;
                for (j=0;j<SlidWinNum;j++)
                {
                    fMPtotal += g_fMPWin[satid][iFreq[i]][j];
                }
                g_tSatState[satid].fMPMean[iFreq[i]] = fMPtotal/SlidWinNum;
                g_tSatState[satid].fMP[iFreq[i]] = fMp[iFreq[i]] - g_tSatState[satid].fMPMean[iFreq[i]];
            }
        }
    }
}

/*矩阵相乘 C[m][k]=A[m][n]*B[n][k]*/
void MatMul(FP64 A[],FP64 B[],INT32U iM,INT32U iN,INT32U iK,FP64 C[])
{
    INT32U i,j,t;

    for (i=0;i<iM;i++)
    {
        for (j=0;j<iK;j++)
        {
            C[i*iK+j]=0.0;
            for (t=0;t<iN;t++)
            {
                C[i*iK+j]=C[i*iK+j]+A[i*iN+t]*B[t*iK+j];
            }
        }
    }
}

/*矩阵求逆 A=B(-1)*/
INT32S MatInv(FP64 A[],INT32U m,FP64 B[])
{
    FP64 MatrixInvisDim[MAXDIM],MatrixInvjsDim[MAXDIM],MatrixInvtmpDim[MAXDIM];
    INT32S i, j, k;
    INT32S l, u, v;
    FP64 d, p;

    for (i=0;i<m*m;i++) {
        MatrixInvtmpDim[i] = A[i];
    }

    for (k = 0; k <= m - 1; k++) {
        d = 0.0;
        for (i = k; i <= m - 1; i++) {
            for (j = k; j <= m - 1; j++) {
                l = i * m + j;
                p = fabs(MatrixInvtmpDim[l]);
                if (p > d) {
                    d = p;
                    MatrixInvisDim[k] = i;
                    MatrixInvjsDim[k] = j;
                }
            }
        }
        if (fabs(d) < EPSILON) {
            return -1;
        }
        if (MatrixInvisDim[k] != k) {
            for (j = 0; j <= m - 1; j++) {
                u = k * m + j;
                v = (MatrixInvisDim[k] *m + j);
                p = MatrixInvtmpDim[u];
                MatrixInvtmpDim[u] = MatrixInvtmpDim[v];
                MatrixInvtmpDim[v] = p;
            }
        }
        if (MatrixInvjsDim[k] != k) {
            for (i = 0; i <= m - 1; i++) {
                u = i * m + k;
                v = (i * m + MatrixInvjsDim[k]);
                p = MatrixInvtmpDim[u];
                MatrixInvtmpDim[u] = MatrixInvtmpDim[v];
                MatrixInvtmpDim[v] = p;
            }
        }
        l = k * m + k;
        if ((fabs(MatrixInvtmpDim[l])<EPSILON)) MatrixInvtmpDim[l]=EPSILON;
        MatrixInvtmpDim[l] = 1.0/MatrixInvtmpDim[l];
        for (j = 0; j <= m - 1; j++) {
            if (j != k) {
                u = k * m + j;
                MatrixInvtmpDim[u] = MatrixInvtmpDim[u] *MatrixInvtmpDim[l];
            }
        }
        for (i = 0; i <= m - 1; i++) {
            if (i != k) {
                for (j = 0; j <= m - 1; j++) {
                    if (j != k) {
                        u = i * m + j;
                        MatrixInvtmpDim[u] = MatrixInvtmpDim[u] - MatrixInvtmpDim[i *m + k] *MatrixInvtmpDim[k *m + j];
                    }
                }
            }
        }
        for (i = 0; i <= m - 1; i++) {
            if (i != k) {
                u = i * m + k;
                MatrixInvtmpDim[u] =  - MatrixInvtmpDim[u] *MatrixInvtmpDim[l];
            }
        }
    }
    for (k = m - 1; k >= 0; k--) {
        if (MatrixInvjsDim[k] != k)
            for (j = 0; j <= m - 1; j++) {
                u = k * m + j;
                v = (MatrixInvjsDim[k] *m + j);
                p = MatrixInvtmpDim[u];
                MatrixInvtmpDim[u] = MatrixInvtmpDim[v];
                MatrixInvtmpDim[v] = p;
            }
            if (MatrixInvisDim[k] != k)
                for (i = 0; i <= m - 1; i++) {
                    u = i * m + k;
                    v = (i * m + MatrixInvisDim[k]);
                    p = MatrixInvtmpDim[u];
                    MatrixInvtmpDim[u] = MatrixInvtmpDim[v];
                    MatrixInvtmpDim[v] = p;
                }
    }
    for (i=0;i<m*m;i++) {
        B[i] = MatrixInvtmpDim[i];
    }

    return 0;
}
