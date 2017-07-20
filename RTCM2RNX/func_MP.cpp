/****************************************************************************
* 版权所有（C）2017，广州海格通信集团股份有限公司
* 
* 文件名称：func_MP.cpp
* 内容摘要：多路径分析
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
#include "def_convrnx.h"
#include "def_DataQC.h"

extern T_SatState g_tSatState[MAXSAT];

void MPAnalys(T_RTCM *ptRtcm, FILE **ofp);
void MPAnalysOneSat_OtherName(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **ofp);

extern double satwavelen(int sat, int frq, int iGFrq);
extern double satFreq(int sat, int frq, int iGFrq);


/*******************************************************************************
* 函数名称： MPAnalys()
* 功能描述： 实现多路径分析
* 输入参数： ptRtcm——RTCM数据
* 输出参数：
* 返 回 值：无 
* 其它说明： 
* 修改日期 版本号 修改人 修改内容
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* 2015/08/14 V1.0.0.0 
*******************************************************************************/
void MPAnalys(T_RTCM *ptRtcm, FILE **ofp)
{
	INT8U i,j,k,l,iFreqnum=0;
	INT8U satId,iFlg;
	ObserDataEpoch slipDectObser;	

	memset(&slipDectObser,0,sizeof(slipDectObser));

	slipDectObser.dEpochTime = ptRtcm->tObs.ptData[0].tObsTime.dSow;
	slipDectObser.validSatNum = ptRtcm->tObs.iN;
	fprintf(ofp[5],"%s%.1f  ","epoch=",slipDectObser.dEpochTime);

	for (j=0;j<MAXSAT;j++)
	{
		iFlg = 0;
		for (i=0;i<ptRtcm->tObs.iN;i++)
		{
			if (j == ptRtcm->tObs.ptData[i].iSat - 1)
			{
				iFlg = 1;
				satId = ptRtcm->tObs.ptData[i].iSat - 1;//卫星编号从1开始
				iFreqnum = 0;
				slipDectObser.iGFrq[satId] = ptRtcm->tObs.ptData[i].iFrq;
				
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
				
				if ((iFreqnum >=2) && (!g_tSatState[satId].iEphFlg))
				{					
                        MPAnalysOneSat_OtherName(&slipDectObser,satId,ofp);
				}
				else
				{
					for (l=0;l<NFREQ+NEXOBS;l++)
					{
						g_tSatState[j].fMP[l] = 0;
						g_tSatState[j].iMPCounter[l] = 0;
						g_tSatState[j].fMPMean[l] = 0;
					}
				}
				
				continue;
			}
		}
		
		if (!iFlg)
		{
			for (l=0;l<NFREQ+NEXOBS;l++)
			{
				g_tSatState[j].fMP[l] = 0;
				g_tSatState[j].iMPCounter[l] = 0;
				g_tSatState[j].fMPMean[l] = 0;
			}
		}
	}

	fprintf(ofp[5],"\n");
}


void MPAnalysOneSat_OtherName(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **ofp)
{
    FP64 faphai[2], fLam[NFREQ], fMp[NFREQ];
    INT8U i;
    INT8U iFreq[NFREQ];
    FP64 fFrq, fFrq0;

    for (i=0;i<pObsEpoch->iFreq[satid].iNum;i++)
    {
        iFreq[i] = pObsEpoch->iFreq[satid].dFreqFlg[i];
        fLam[iFreq[i]] = satwavelen(satid, iFreq[i], pObsEpoch->iGFrq[satid]);
    }

    fFrq0 = satFreq(satid, 0, pObsEpoch->iGFrq[satid]);

    for (i=0;i<pObsEpoch->iFreq[satid].iNum-1;i++)
    {
        fFrq = satFreq(satid, iFreq[i+1], pObsEpoch->iGFrq[satid]);
        faphai[i] = (fFrq0*fFrq0) / (fFrq*fFrq);
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
        g_tSatState[satid].fMPMean[iFreq[i]] = g_tSatState[satid].fMPMean[iFreq[i]] + (fMp[iFreq[i]] - g_tSatState[satid].fMPMean[iFreq[i]])/g_tSatState[satid].iMPCounter[iFreq[i]];
        g_tSatState[satid].fMP[iFreq[i]] = fMp[iFreq[i]] - g_tSatState[satid].fMPMean[iFreq[i]];
        if (satid==68)
        fprintf(ofp[5],"%s%d %s%d %s%.3f  ","satid=",satid+1,"freq=",iFreq[i],"fMp=",g_tSatState[satid].fMP[iFreq[i]]);
    }
}







  
