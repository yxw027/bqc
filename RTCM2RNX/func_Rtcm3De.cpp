/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
*
* 文件名称：func_Rtcm3De.cpp
* 内容摘要：RTCM 3.1电文解码
*           1001-1004             GPS观测值
*      		1005-1006             基准站信息
*   		1007-1008             天线信息
*   		1009-1012             GLONASS观测值
*   		4087-4088,4091-4092   BDS观测值
*   		1019-1020,4095        GPS/GLONASS/BDS卫星星历
* 版本  号：
* 作    者：sbz
* 完成日期：2015-11-19
*
* 修改记录1：
*    修改日期：
*    版本  号：
*    修改  人：
*    修改内容：
* 修改记录2：
********************************************************************/

/******************************************************************************
*                          包含的头文件                                       *
******************************************************************************/
#include "def_convrnx.h"
#include "def_DataQC.h"

/* msm signal id table -------------------------------------------------------*/
const CHAR *msm_sig_gps[32]={
    /* GPS: ref [13] table 3.5-87, ref [14][15] table 3.5-91 */
    ""  ,"1C","1P","1W","1Y","1M",""  ,"2C","2P","2W","2Y","2M", /*  1-12 */
    ""  ,""  ,"2S","2L","2X",""  ,""  ,""  ,""  ,"5I","5Q","5X", /* 13-24 */
    ""  ,""  ,""  ,""  ,""  ,"1S","1L","1X"                      /* 25-32 */
};
const CHAR *msm_sig_glo[32]={
    /* GLONASS: ref [13] table 3.5-93, ref [14][15] table 3.5-97 */
    ""  ,"1C","1P",""  ,""  ,""  ,""  ,"2C","2P",""  ,"3I","3Q",
    "3X",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};
const CHAR *msm_sig_bds[32]={
    /* BeiDou: ref [15] table 3.5-106 */
    ""  ,"2I","2Q","2X",""  ,""  ,""  ,"6I","6Q","6X",""  ,""  ,
    ""  ,"7I","7Q","7X",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};

/******************************************************************************
*    函数名称:  AdjWeek
*    功能描述:  调整GPS时间
*    输入参数:  RTCM结构体变量,GPS历元时刻
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void AdjWeek(T_RTCM *rtcm, FP64 tow)
{
    FP64 tow_p;

    /* if no time, get cpu time */
    if (rtcm->tTime.iWeek==0) return;
    tow_p=rtcm->tTime.dSow;
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    rtcm->tTime.dSow=tow;
}

/******************************************************************************
*    函数名称:  AdjDay_glot
*    功能描述:  调整GLONASS时间
*    输入参数:  RTCM结构体变量,GLONASS历元时刻
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void AdjDay_glot(T_RTCM *rtcm, FP64 tod)
{
    T_WEEKSEC time;
    FP64 tow,tod_p;

    if (rtcm->tTime.iWeek==0) return;
    time=TimeAdd(rtcm->tTime,10800.0); /* glonass time */
    tow=time.dSow;
    tod_p=fmod(tow,86400.0); tow-=tod_p;
    if      (tod<tod_p-43200.0) tod+=86400.0;
    else if (tod>tod_p+43200.0) tod-=86400.0;
    time.dSow=tow+tod;
    rtcm->tTime=TimeAdd(time,-10800.0);
}

/******************************************************************************
*    函数名称:  Test_Staid
*    功能描述:  验证电文中测站ID的一致性
*    输入参数:  RTCM结构体变量，测站ID
*    输出参数:  RTCM结构体变量
*    返 回 值:  一致性标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Test_Staid(T_RTCM *ptRtcm,INT32U iStaid)
{
    INT32U iType;
    if (ptRtcm->iStaid==0||ptRtcm->iObsflag==1)
    {
        ptRtcm->iStaid=iStaid;
    }
    else if (ptRtcm->iStaid!=iStaid)
    {
        iType=GetBitu(ptRtcm->cBuff,24,12);
        fprintf(stderr,"RTCM 3 %d staid invalid id=%d %d\n",iType,iStaid,ptRtcm->iStaid);

        ptRtcm->iStaid=0;
        return 0;
    }
    return 1;
}

/******************************************************************************
*    函数名称:  ObsIndex
*    功能描述:  得到观测值存储位置索引
*    输入参数:  卫星观测值指针变量，观测时间，卫星编号
*    输出参数:  卫星观测值指针变量
*    返 回 值:  观测值存储位置
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S ObsIndex(T_OBS *tObs, T_WEEKSEC tWSec, INT32U iSat)
{
    T_OBSSAT tData0={{0}};
    INT32U i;

    for (i=0;i<tObs->iN;i++)
    {
        if (tObs->ptData[i].iSat==iSat) /*卫星数据是否已存在*/
        {
            return i;
        }
    }
    if( i>MAXOBS)
    {
        return -1;
    }
    tObs->ptData[i]=tData0; /*初始化*/
    tObs->iN=tObs->iN+1; /*观测值个数增加1个*/
    tObs->ptData[i].tObsTime=tWSec; /*观测时间*/
    tObs->ptData[i].iSat=iSat; /*卫星编号*/

    return i;
}

/******************************************************************************
*    函数名称:  AdjCp
*    功能描述:  调整载波观测值
*    输入参数:  RTCM结构体变量，卫星编号，观测值频率，载波观测值
*    输出参数:  RTCM结构体变量
*    返 回 值:  载波观测值
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static FP64 AdjCp(T_RTCM *ptRtcm,INT32U iSat,INT32U iFreq,FP64 dCp)
{
    FP64 dCp0=ptRtcm->dCp[iSat-1][iFreq];

    if (dCp<dCp0-750.0)
    {
        dCp=dCp+1500.0;
    }
    else if (dCp>dCp0+750.0)
    {
        dCp=dCp-1500.0;
    }
    ptRtcm->dCp[iSat-1][iFreq]=dCp;

    return dCp;
}

/******************************************************************************
*    函数名称:  LossofLock
*    功能描述:  失锁标识
*    输入参数:  RTCM结构体变量，卫星编号，观测值频率，失锁标识
*    输出参数:  无
*    返 回 值:  失锁标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT8U LossofLock(T_RTCM *ptRtcm,INT32U iSat,INT32U iFreq,INT32U iLock)
{
    INT32U iLLI,iLock0;

    iLock0=ptRtcm->cLock[iSat-1][iFreq];
    if ((!iLock&&!iLock0)||(iLock0>iLock))
    {
        iLLI=1;
    }
    else
    {
        iLLI=0;
    }
    return (unsigned char)iLLI;
}

/******************************************************************************
*    函数名称:  SNR
*    功能描述:  信噪比
*    输入参数:  信噪比
*    输出参数:  无
*    返 回 值:  信噪比
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT8U SNR(FP64 dSnr)
{
    if (dSnr>0&&dSnr<255.5)
    {
        return (unsigned char)(dSnr*4+0.5);
    }
    else
    {
        return (unsigned char)(0.0);
    }
}

/******************************************************************************
*    函数名称:  Decode_Head1001
*    功能描述:  解析1001-1004电文的文件头
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量，同步标识
*    返 回 值:  卫星个数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Head1001(T_RTCM *ptRtcm,INT32U *iSync)
{
    INT32U iType,iStaid,iNsat;
    FP64 dTow;
    INT32U i=24;

    iType=GetBitu(ptRtcm->cBuff,i,12);  /*消息类型*/

    if (ptRtcm->iLen*8>=64)
    {
        iStaid=GetBitu(ptRtcm->cBuff,i+12,12); /*测站ID*/
        dTow  =GetBitu(ptRtcm->cBuff,i+24,30)/1000.0; /*观测值历元时刻 周内秒*/
        *iSync=GetBitu(ptRtcm->cBuff,i+54,1); /*GNSS同步标志*/
        iNsat =GetBitu(ptRtcm->cBuff,i+55,5); /*GPS卫星个数*/
    }
    else
    {
        fprintf(stderr,"RTCM 3 %d length error: len=%d\n",iType,ptRtcm->iLen);
        return -1;
    }

    if (!Test_Staid(ptRtcm,iStaid)) /*验证测站一致性*/
    {
        return -1;
    }
    ptRtcm->tTime.dSow=dTow;

    return iNsat;
}

/******************************************************************************
*    函数名称:  Decode_Type1001
*    功能描述:  解析1001类型：RTK中GPS L1观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1001(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head1001(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1002
*    功能描述:  解析1002类型：RTK中GPS扩展L1观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1002(T_RTCM *ptRtcm)
{
    const INT32U L1codes[]={CODE_L1C,CODE_L1P};

    FP64 dPr1,dCnr1,dDSec,dCp1;
    INT32U iPrn,iCode1,iLock1,iAmb;
    INT32S iCppr1;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys;
    INT32U i=24+64;
    INT32S j;
    if ((iNsat=Decode_Head1001(ptRtcm,&iSync))<0) /*解析电文头*/
    {
        return -1;
    }

    /*逐颗卫星解析电文存储数据*/
    for (j=0;j<iNsat;j++)
    {
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+74<=ptRtcm->iLen*8+24))
        {
            /*解析电文*/
            iPrn  =GetBitu(ptRtcm->cBuff,i,   6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6, 1);
            dPr1  =GetBitu(ptRtcm->cBuff,i+7, 24);
            iCppr1=GetBits(ptRtcm->cBuff,i+31,20);
            iLock1=GetBitu(ptRtcm->cBuff,i+51,7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+58,8);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+66,8);

            i=i+74;

            /*去除SBAS卫星*/
            if (iPrn<40)
            {
                iSys=SYS_GPS;
            }
            else
            {
                continue;
            }

            /*获得卫星编号*/
            if(!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_GPS; /*L1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam_Carr[0]);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam_Carr[0]+dCp1; /*L1载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=L1codes[iCode1];
                        //i /= 4*4;
                        //if(i > 9) i = 9;
                        //else if(i < 1) i = 1;
                        //gnss->snrL1[num] = i;
        }
        else
        {
            return -1;
        }
    }

    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1003
*    功能描述:  解析1003类型：RTK中GPS L1和L2观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1003(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head1001(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1004
*    功能描述:  解析1004类型：RTK中GPS扩展的L1和L2观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1004(T_RTCM *ptRtcm)
{
    const INT32U L1codes[]={CODE_L1C,CODE_L1P};
    const INT32U L2codes[]={CODE_L2C,CODE_L2P,CODE_L2W,CODE_L2W};

    FP64 dPr1,dCnr1,dCnr2;
    FP64 dDSec,dCp1,dCp2;
    INT32U iPrn,iCode1,iLock1,iAmb,iCode2,iLock2;
    INT32S iCppr1,iPr21,iCppr2;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys;
    INT32U i=24+64;
    INT32S j;

    if ((iNsat=Decode_Head1001(ptRtcm,&iSync))<0) /*解析电文头*/
    {
        return -1;
    }

    /*逐颗卫星解析电文存储数据*/
    for (j=0;j<iNsat;j++)
    {
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+125<=ptRtcm->iLen*8+24))
        {
            /*解析电文*/
            iPrn  =GetBitu(ptRtcm->cBuff,i,    6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6,  1);
            dPr1  =GetBitu(ptRtcm->cBuff,i+7,  24);
            iCppr1=GetBits(ptRtcm->cBuff,i+31, 20);
            iLock1=GetBitu(ptRtcm->cBuff,i+51, 7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+58, 8);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+66, 8);
            iCode2=GetBitu(ptRtcm->cBuff,i+74, 2);
            iPr21 =GetBits(ptRtcm->cBuff,i+76, 14);
            iCppr2=GetBits(ptRtcm->cBuff,i+90, 20);
            iLock2=GetBitu(ptRtcm->cBuff,i+110,7);
            dCnr2 =GetBitu(ptRtcm->cBuff,i+117,8);

            i=i+125;

            /*去除SBAS卫星*/
            if (iPrn<40)
            {
                iSys=SYS_GPS;
            }
            else
            {
                continue;
            }

            /*获得卫星编号*/
            if(!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_GPS; /*L1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam_Carr[0]);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam_Carr[0]+dCp1; /*L1载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=L1codes[iCode1];

            if (iPr21!=(int)0xFFFFE000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[1]=dPr1+iPr21*0.02; /*L2伪距*/
            }
            if (iCppr2!=(int)0xFFF80000)
            {
                dCp2=AdjCp(ptRtcm,iSat,1,iCppr2*0.0005/dLam_Carr[1]);
                ptRtcm->tObs.ptData[iIndex].dL[1]=dPr1/dLam_Carr[1]+dCp2; /*L2载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[1] =LossofLock(ptRtcm,iSat,1,iLock2);
            ptRtcm->tObs.ptData[iIndex].dCNR[1] =dCnr2*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[1] =SNR(dCnr2*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[1]=L2codes[iCode2];
        }
        else
        {
            return -1;
        }
    }

    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  GetBits_38
*    功能描述:  得到38位有符号的数据
*    输入参数:  缓存消息，提取位起始位置
*    输出参数:  无
*    返 回 值:  提取得到的有符号数据
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static FP64 GetBits_38(const INT8U *pcBuff,INT32U iPos)
{
    INT32U iBitu;
    FP64 dBits;

    dBits=GetBits(pcBuff,iPos,32)*64.0;
    iBitu=GetBitu(pcBuff,iPos+32,6);

    return (double)(dBits+iBitu);
}

/******************************************************************************
*    函数名称:  Decode_Type1005
*    功能描述:  解析1005类型：参考站天线参考点坐标，没有天线高
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1005(T_RTCM *ptRtcm)
{
    FP64 dAPR[3];
    INT32U iItrf,iStaid;
    INT32U i=24+12,j;

    if (i+140<=ptRtcm->iLen*8+24)
    {
        iStaid =GetBitu(ptRtcm->cBuff,i,   12);
        iItrf  =GetBitu(ptRtcm->cBuff,i+12,6);
        dAPR[0]=GetBits_38(ptRtcm->cBuff,i+22);
        dAPR[1]=GetBits_38(ptRtcm->cBuff,i+62);
        dAPR[2]=GetBits_38(ptRtcm->cBuff,i+102);
    }
    else
    {
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }

    for (j=0;j<3;j++)
    {
        ptRtcm->tSta.dAppPos[j]=dAPR[j]*0.0001; /*测站坐标*/
        ptRtcm->tSta.dAntDel[j]=0.0; /*天线高*/
    }
    ptRtcm->tSta.iItrf      =iItrf;
    ptRtcm->tSta.dAntDel[0] =0.0;

    return 5;
}

/******************************************************************************
*    函数名称:  Decode_Type1006
*    功能描述:  解析1006类型：参考站天线参考点坐标，包括天线高
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1006(T_RTCM *ptRtcm)
{
    FP64 dAPR[3],dAnth;
    INT32U iItrf,iStaid;
    INT32U i=24+12,j;

    if (i+140<=ptRtcm->iLen*8+24)
    {
        iStaid =GetBitu(ptRtcm->cBuff,i,   12);
        iItrf  =GetBitu(ptRtcm->cBuff,i+12,6);
        dAPR[0]=GetBits_38(ptRtcm->cBuff,i+22);
        dAPR[1]=GetBits_38(ptRtcm->cBuff,i+62);
        dAPR[2]=GetBits_38(ptRtcm->cBuff,i+102);
        dAnth  =GetBitu(ptRtcm->cBuff,i+140,16);
    }
    else
    {
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }

    for (j=0;j<3;j++)
    {
        ptRtcm->tSta.dAppPos[j]=dAPR[j]*0.0001; /*测站坐标*/
        ptRtcm->tSta.dAntDel[j]=0.0;
    }
    ptRtcm->tSta.iItrf      =iItrf;
    ptRtcm->tSta.dAntDel[0] =dAnth*0.0001; /*天线高*/

    return 5;
}

/******************************************************************************
*    函数名称:  Decode_Type1007
*    功能描述:  解析1007类型：天线类型描述
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1007(T_RTCM *ptRtcm)
{
    INT8U cDes[32]="";
    INT32U iStaid,iN,iSetup;
    INT32U i=24+12,j;

    /*天线类型描述占用的字节*/
    iN=GetBitu(ptRtcm->cBuff,i+12,8);
    if (iN>31)
    {
        return -1;
    }

    if ((i+28+8*iN)<=(ptRtcm->iLen*8+24))
    {
        iStaid=GetBitu(ptRtcm->cBuff,i,12);
        i=i+20;

        /*天线类型描述*/
        for (j=0;j<iN;j++)
        {
            cDes[j]=(char)GetBitu(ptRtcm->cBuff,i,8);
            i=i+8;
        }
        iSetup=GetBitu(ptRtcm->cBuff,i,8); /*天线建立ID*/
    }
    else
    {
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }

    strncpy((char *)ptRtcm->tSta.cAnt[1],(char *)cDes,iN);
    ptRtcm->tSta.cAnt[1][iN]='\0'; /*天线类型*/
    ptRtcm->tSta.iAntSetup=iSetup;
    ptRtcm->tSta.cAnt[0][0]='\0'; /*天线序列号*/

    return 5;
}

/******************************************************************************
*    函数名称:  Decode_Type1008
*    功能描述:  解析1008类型：天线描述和序列号
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1008(T_RTCM *ptRtcm)
{
    INT8U cDes[32]="",cAntsno[32]="";
    INT32U iStaid,iN,iSetup,iM;
    INT32U i=24+12,j;

    /*天线类型描述和序列号占用的字节*/
    iN=GetBitu(ptRtcm->cBuff,i+12,8);
    iM=GetBitu(ptRtcm->cBuff,i+28+iN*8,8);

    if (iN>31||iM>31)
    {
        return -1;
    }

    if ((i+36+(iN+iM)*8)<=(ptRtcm->iLen*8+24))
    {
        iStaid=GetBitu(ptRtcm->cBuff,i,12);
        i=i+20;

        /*天线类型描述*/
        for (j=0;j<iN;j++)
        {
            cDes[j]=(char)GetBitu(ptRtcm->cBuff,i,8);
            i=i+8;
        }

        iSetup=GetBitu(ptRtcm->cBuff,i,8);;
        i=i+16;

        /*天线序列号*/
        for (j=0;j<iM;j++)
        {
            cAntsno[j]=(char)GetBitu(ptRtcm->cBuff,i,8);
            i=i+8;
        }
    }
    else
    {
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }

    strncpy((char *)ptRtcm->tSta.cAnt[1],(char *)cDes,iN);
    ptRtcm->tSta.cAnt[1][iN]='\0'; /*天线类型*/
    ptRtcm->tSta.iAntSetup=iSetup;
    strncpy((char *)ptRtcm->tSta.cAnt[0],(char *)cAntsno,iM);
    ptRtcm->tSta.cAnt[0][iM]='\0'; /*天线序列号*/

    return 5;
}

/******************************************************************************
*    函数名称:  Decode_Head1009
*    功能描述:  解析1009-1012电文的文件头
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量，同步标识
*    返 回 值:  卫星个数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Head1009(T_RTCM *ptRtcm,INT32U *iSync)
{
    INT32U iType,iStaid,iNsat;
    FP64 dSod;
    T_WEEKSEC tUtc,tGlot;
    T_YMD tYmd;
    INT32U i=24;

    iType=GetBitu(ptRtcm->cBuff,i,12);

    if(i+61<=ptRtcm->iLen*8+24)
    {
        iStaid=GetBitu(ptRtcm->cBuff,i+12,12); /*测站ID*/
        dSod  =GetBitu(ptRtcm->cBuff,i+24,27)/1000.0; /*观测值历元时刻 天内秒 UTC时+3h*/
        *iSync=GetBitu(ptRtcm->cBuff,i+51,1); /*GNSS同步标志*/
        iNsat =GetBitu(ptRtcm->cBuff,i+52,5); /*GLONASS卫星个数*/
    }
    else
    {
        fprintf(stderr,"RTCM 3 %d length error: len=%d\n",iType,ptRtcm->iLen);
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }

    tUtc =TimeAdd(ptRtcm->tTime,-LEAPS); /*GPS时转换成UTC时*/
    tGlot=TimeAdd(tUtc,10800); /*UTC时转换为GLO时*/
    tYmd=WeekSec2YMD(tGlot);
    tYmd.iHor=(int)(dSod/3600.0);
    tYmd.iMin=(int)((dSod-tYmd.iHor*3600.0)/60.0);
    tYmd.dSec=dSod-tYmd.iHor*3600.0-tYmd.iMin*60.0;
    tGlot=YMD2WeekSec(tYmd);
    tUtc=TimeAdd(tGlot,-10800);
    ptRtcm->tTime=TimeAdd(tUtc,LEAPS);
    return iNsat;
}

/******************************************************************************
*    函数名称:  Decode_Type1009
*    功能描述:  解析1009类型：RTK中GLONASS L1观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1009(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head1009(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1010
*    功能描述:  解析1010类型：RTK中GLONASS扩展的L1观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1010(T_RTCM *ptRtcm)
{
    const INT32U L1codes[]={CODE_L1C,CODE_L1P};

    INT32U iPrn,iCode1,iLock1,iAmb;
    FP64 dPr1,dCnr1;
    FP64 dDSec,dCp1,dLam1;
    INT32S iCppr1,iNfreq;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys=SYS_GLO;
    INT32U i=24+61;
    INT32S j;

    if ((iNsat=Decode_Head1009(ptRtcm,&iSync))<0)
    {
        return -1;
    }

    for (j=0;j<iNsat;j++)
    {
        /*解析电文*/
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+79<=ptRtcm->iLen*8+24))
        {
            iPrn  =GetBitu(ptRtcm->cBuff,i,   6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6, 1);
            iNfreq=GetBitu(ptRtcm->cBuff,i+7, 5);
            dPr1  =GetBitu(ptRtcm->cBuff,i+12,25);
            iCppr1=GetBits(ptRtcm->cBuff,i+37,20);
            iLock1=GetBitu(ptRtcm->cBuff,i+57,7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+64,7);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+71,8);
            i=i+79;

            /*卫星编号*/
            if (!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_GLO; /*L1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                dLam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(iNfreq-7));

                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam1);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam1+dCp1; /*L1载波*/
            }

            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=L1codes[iCode1];
        }
        else
        {
            return -1;
        }
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1011
*    功能描述:  解析1011类型：RTK中GLONASS L1和L2观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1011(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head1001(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type1012
*    功能描述:  解析1012类型：RTK中GLONASS扩展的L1和L2观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1012(T_RTCM *ptRtcm)
{
    const INT32U L1codes[]={CODE_L1C,CODE_L1P};
    const INT32U L2codes[]={CODE_L2C,CODE_L2P};

    INT32U iPrn,iCode1,iLock1,iAmb,iCode2,iLock2;
    FP64 dPr1,dCnr1,dCnr2;
    FP64 dDSec,dCp1,dCp2,dLam1,dLam2;
    INT32S iCppr1,iPr21,iCppr2,iNfreq;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys=SYS_GLO;
    INT32U i=24+61;
    INT32S j;

    if ((iNsat=Decode_Head1009(ptRtcm,&iSync))<0)
    {
        return -1;
    }

    /*解析电文*/
    for (j=0;j<iNsat;j++)
    {
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+130<=ptRtcm->iLen*8+24))
        {
            iPrn  =GetBitu(ptRtcm->cBuff,i,    6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6,  1);
            iNfreq=GetBitu(ptRtcm->cBuff,i+7,  5);
            dPr1  =GetBitu(ptRtcm->cBuff,i+12, 25);
            iCppr1=GetBits(ptRtcm->cBuff,i+37, 20);
            iLock1=GetBitu(ptRtcm->cBuff,i+57, 7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+64, 7);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+71, 8);
            iCode2=GetBitu(ptRtcm->cBuff,i+79, 2);
            iPr21 =GetBits(ptRtcm->cBuff,i+81, 14);
            iCppr2=GetBits(ptRtcm->cBuff,i+95, 20);
            iLock2=GetBitu(ptRtcm->cBuff,i+115,7);
            dCnr2 =GetBitu(ptRtcm->cBuff,i+122,8);
            i=i+130;

            /*卫星编号*/
            if (!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_GLO; /*L1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                dLam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(iNfreq-7));

                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam1);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam1+dCp1; /*L1载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=L1codes[iCode1];

            if (iPr21!=(int)0xFFFFE000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[1]=dPr1+iPr21*0.02; /*L2伪距*/
            }
            if (iCppr2!=(int)0xFFF80000)
            {
                dLam2=CLIGHT/(FREQ2_GLO+DFRQ2_GLO*(iNfreq-7));

                dCp2=AdjCp(ptRtcm,iSat,1,iCppr2*0.0005/dLam2);
                ptRtcm->tObs.ptData[iIndex].dL[1]=dPr1/dLam2+dCp2; /*L2载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[1] =LossofLock(ptRtcm,iSat,1,iLock2);
            ptRtcm->tObs.ptData[iIndex].dCNR[1] =dCnr2*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[1] =SNR(dCnr2*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[1]=L2codes[iCode2];
        }
        else
        {
            return -1;
        }
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Head4087
*    功能描述:  解析4087、4088、4091、4092电文的文件头
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量，同步标识
*    返 回 值:  卫星个数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Head4087(T_RTCM *ptRtcm,INT32U *iSync)
{
    INT32U iType,iStaid,iNsat;
    FP64 dTow;
    INT32U i=24;

    iType=GetBitu(ptRtcm->cBuff,i,12);  /*消息类型*/

    if (ptRtcm->iLen*8>=64)
    {
        iStaid=GetBitu(ptRtcm->cBuff,i+12,12); /*测站ID*/
        dTow  =GetBitu(ptRtcm->cBuff,i+24,30)/1000.0; /*观测值历元时刻 周内秒*/
        *iSync=GetBitu(ptRtcm->cBuff,i+54,1); /*GNSS同步标志*/
        iNsat =GetBitu(ptRtcm->cBuff,i+55,5); /*BDS卫星个数*/
    }
    else
    {
        fprintf(stderr,"RTCM 3 %d length error: len=%d\n",iType,ptRtcm->iLen);
        return -1;
    }

    /*验证测站ID*/
    if (!Test_Staid(ptRtcm,iStaid))
    {
        return -1;
    }
    ptRtcm->tTime.dSow=dTow;

    return iNsat;
}

/******************************************************************************
*    函数名称:  Decode_Type4087
*    功能描述:  解析4087类型：RTK中BDS B1观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type4087(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head4087(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type4088
*    功能描述:  解析4088类型：RTK中BDS扩展B1观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type4088(T_RTCM *ptRtcm)
{
    const INT32U B1codes[]={CODE_L2I};

    FP64 dPr1,dCnr1,dDSec,dCp1;
    INT32U iPrn,iCode1,iLock1,iAmb;
    INT32S iCppr1;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys=SYS_BDS;
    INT32U i=24+64;
    INT32S j;

    if ((iNsat=Decode_Head4087(ptRtcm,&iSync))<0) /*解析电文头*/
    {
        return -1;
    }

    /*逐颗卫星解析电文存储数据*/
    for (j=0;j<iNsat;j++)
    {
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+74<=ptRtcm->iLen*8+24))
        {
            /*解析电文*/
            iPrn  =GetBitu(ptRtcm->cBuff,i,   6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6, 1);
            dPr1  =GetBitu(ptRtcm->cBuff,i+7, 24);
            iCppr1=GetBits(ptRtcm->cBuff,i+31,20);
            iLock1=GetBitu(ptRtcm->cBuff,i+51,7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+58,8);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+66,8);

            i=i+74;

            /*获得卫星编号*/
            if(!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_BDS; /*B1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam_Carr[3]);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam_Carr[3]+dCp1; /*B1载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=B1codes[iCode1];
        }
        else
        {
            return -1;
        }
    }

    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type4091
*    功能描述:  解析4091类型：RTK中BDS B1和B3观测值 不支持
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type4091(T_RTCM *ptRtcm)
{
    INT32U iSync;

    if (Decode_Head4087(ptRtcm,&iSync)<0)
    {
        return -1;
    }
    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
}

/******************************************************************************
*    函数名称:  Decode_Type4092
*    功能描述:  解析4092类型：RTK中BDS扩展的B1和B3观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type4092(T_RTCM *ptRtcm)
{
    const INT32U B1codes[]={CODE_L2I};
    const INT32U B3codes[]={CODE_L6I};

    FP64 dPr1,dCnr1,dCnr3;
    FP64 dDSec,dCp1,dCp3;
    INT32U iPrn,iCode1,iLock1,iAmb,iCode3,iLock3;
    INT32S iCppr1,iPr31,iCppr3;
    INT32U iSync;
    INT32S iNsat,iIndex;
    INT32U iSat,iSys=SYS_BDS;
    INT32U i=24+64;
    INT32S j;

    if ((iNsat=Decode_Head4087(ptRtcm,&iSync))<0) /*解析电文头*/
    {
        return -1;
    }

    /*逐颗卫星解析电文存储数据*/
    for (j=0;j<iNsat;j++)
    {
        if ((ptRtcm->tObs.iN<MAXOBS)&&(i+125<=ptRtcm->iLen*8+24))
        {
            /*解析电文*/
            iPrn  =GetBitu(ptRtcm->cBuff,i,    6);
            iCode1=GetBitu(ptRtcm->cBuff,i+6,  1);
            dPr1  =GetBitu(ptRtcm->cBuff,i+7,  24);
            iCppr1=GetBits(ptRtcm->cBuff,i+31, 20);
            iLock1=GetBitu(ptRtcm->cBuff,i+51, 7);
            iAmb  =GetBitu(ptRtcm->cBuff,i+58, 8);
            dCnr1 =GetBitu(ptRtcm->cBuff,i+66, 8);
            iCode3=GetBitu(ptRtcm->cBuff,i+74, 2);
            iPr31 =GetBits(ptRtcm->cBuff,i+76, 14);
            iCppr3=GetBits(ptRtcm->cBuff,i+90, 20);
            iLock3=GetBitu(ptRtcm->cBuff,i+110,7);
            dCnr3 =GetBitu(ptRtcm->cBuff,i+117,8);

            i=i+125;

            /*获得卫星编号*/
            if(!(iSat=Prn2Sat(iSys,iPrn)))
            {
                continue;
            }

            /*判断是否是同一时刻的观测数据*/
            dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag==1||fabs(dDSec)>1E-9)
            {
                ptRtcm->iObsflag=0;
                ptRtcm->tObs.iN=0;
            }

            /*获得观测数据存储的位置*/
            if ((iIndex=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,iSat))<0)
            {
                continue;
            }

            /*存储观测值*/
            dPr1=dPr1*0.02+iAmb*PRUNIT_BDS; /*B1伪距*/
            if (iCppr1!=(int)0xFFF80000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[0]=dPr1;
                dCp1=AdjCp(ptRtcm,iSat,0,iCppr1*0.0005/dLam_Carr[3]);
                ptRtcm->tObs.ptData[iIndex].dL[0]=dPr1/dLam_Carr[3]+dCp1; /*B1载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[0] =LossofLock(ptRtcm,iSat,0,iLock1);
            ptRtcm->tObs.ptData[iIndex].dCNR[0] =dCnr1*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[0] =SNR(dCnr1*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[0]=B1codes[iCode1];

            if (iPr31!=(int)0xFFFFE000)
            {
                ptRtcm->tObs.ptData[iIndex].dP[2]=dPr1+iPr31*0.02; /*B3伪距*/
            }
            if (iCppr3!=(int)0xFFF80000)
            {
                dCp3=AdjCp(ptRtcm,iSat,2,iCppr3*0.0005/dLam_Carr[5]);
                ptRtcm->tObs.ptData[iIndex].dL[2]=dPr1/dLam_Carr[5]+dCp3; /*B3载波*/
            }
            ptRtcm->tObs.ptData[iIndex].cLLI[2] =LossofLock(ptRtcm,iSat,2,iLock3);
            ptRtcm->tObs.ptData[iIndex].dCNR[2] =dCnr3*0.25;
            ptRtcm->tObs.ptData[iIndex].cSNR[2] =SNR(dCnr3*0.25);
            ptRtcm->tObs.ptData[iIndex].cCode[2]=B3codes[iCode3];
        }
        else
        {
            return -1;
        }
    }

    ptRtcm->iObsflag=!iSync;

    return ptRtcm->iObsflag;
    //return 1;
}

/******************************************************************************
*    函数名称:  Decode_Type1019
*    功能描述:  解析1019类型：GPS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1019(T_RTCM *ptRtcm)
{
    const FP64 dFit[]={4.0,0.0};

    T_EPH tEph={0};
    INT32U iPrn,iSat,iFit,iSys=SYS_GPS;
    INT32U i=24+12;

    /*解析电文*/
    if (i+476<=ptRtcm->iLen*8+24)
    {
        iPrn          =GetBitu(ptRtcm->cBuff,i,6);               i=i+6;
        tEph.iWeek    =GetBitu(ptRtcm->cBuff,i,10)+1024;         i=i+10;
        tEph.iSva     =GetBitu(ptRtcm->cBuff,i,4);               i=i+4;
        tEph.iCode    =GetBitu(ptRtcm->cBuff,i,2);               i=i+2;
        tEph.dIdot    =GetBits(ptRtcm->cBuff,i,14)*P2_43*SC2RAD; i=i+14;
        tEph.iIode    =GetBitu(ptRtcm->cBuff,i,8);               i=i+8;
        tEph.tToc.dSow=GetBitu(ptRtcm->cBuff,i,16)*16.0;         i=i+16;
        tEph.dF2      =GetBits(ptRtcm->cBuff,i,8)*P2_55;         i=i+8;
        tEph.dF1      =GetBits(ptRtcm->cBuff,i,16)*P2_43;        i=i+16;
        tEph.dF0      =GetBits(ptRtcm->cBuff,i,22)*P2_31;        i=i+22;
        tEph.iIodc    =GetBitu(ptRtcm->cBuff,i,10);              i=i+10;
        tEph.dCrs     =GetBits(ptRtcm->cBuff,i,16)*P2_5;         i=i+16;
        tEph.dDeln    =GetBits(ptRtcm->cBuff,i,16)*P2_43*SC2RAD; i=i+16;
        tEph.dM0      =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCuc     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dE       =GetBitu(ptRtcm->cBuff,i,32)*P2_33;        i=i+32;
        tEph.dCus     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dSqrtA   =GetBitu(ptRtcm->cBuff,i,32)*P2_19;        i=i+32;
        tEph.dToes    =GetBitu(ptRtcm->cBuff,i,16)*16.0;         i=i+16;
        tEph.dCic     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dOMG0    =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCis     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dI0      =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCrc     =GetBits(ptRtcm->cBuff,i,16)*P2_5;         i=i+16;
        tEph.dOmg     =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dOMGd    =GetBits(ptRtcm->cBuff,i,24)*P2_43*SC2RAD; i=i+24;
        tEph.dTgd[0]  =GetBits(ptRtcm->cBuff,i,8) *P2_31;        i=i+8;
        tEph.iSvh     =GetBitu(ptRtcm->cBuff,i,6);               i=i+6;
        tEph.iFlag    =GetBitu(ptRtcm->cBuff,i,1);               i=i+1;
        iFit          =GetBitu(ptRtcm->cBuff,i,1);
    }
    else
    {
        fprintf(stderr,"RTCM 3 1019 length error: len=%d\n",ptRtcm->iLen);
        return -1;
    }

    /*去除SBAS卫星*/
    if (iPrn>=40)
    {
        return -1;
    }

    /*获得卫星编号*/
    if(!(iSat=Prn2Sat(iSys,iPrn)))
    {
        return -1;
    }

    tEph.iSat=iSat;
    tEph.tToc.iWeek=tEph.tToe.iWeek=tEph.iWeek;
    tEph.tToe.dSow=tEph.dToes;
    tEph.tTtr=tEph.tToc;
    ptRtcm->tTime=tEph.tToc; /*GPS导航星历数据时间更新RTCM项目近似时间*/
    tEph.dFit=dFit[iFit];  /*0:4hr,1:>4hr*/

    if (!strstr((char *)ptRtcm->cOpt,"-EPHALL"))
    {
        if (tEph.iIode==ptRtcm->tNav.ptEph[iSat-1].iIode)
        {
            return 0; /*不改变该卫星的广播星历*/
        }
    }

    ptRtcm->tNav.ptEph[iSat-1]=tEph;
    ptRtcm->iEphsat=iSat;
    ptRtcm->tNav.iN=ptRtcm->tNav.iN+1;

    return 2;
}

/******************************************************************************
*    函数名称:  GetBitg
*    功能描述:  提取位 位数大的有符号数据
*    输入参数:  缓存消息，提取位起始位置，提取位长度
*    输出参数:  无
*    返 回 值:  提取得到的浮点型数据
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static FP64 GetBitg(const INT8U *pcBuff,INT32U iPos,INT32U iLen)
{
    FP64 dValue;
    INT32U iSign;

    dValue=GetBitu(pcBuff,iPos+1,iLen-1);
    iSign =GetBitu(pcBuff,iPos,1);

    if (iSign)
    {
        return -dValue;
    }
    else
    {
        return dValue;
    }
}

/******************************************************************************
*    函数名称:  Decode_Type1020
*    功能描述:  解析1020类型：GLONASS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1020(T_RTCM *ptRtcm)
{
    T_GLOEPH tGeph={0};
    INT32U iTk_h,iTk_m,iTb;
    FP64 dTk_s,dSod;
    INT32U iPrn,iSat,iSys=SYS_GLO;
    INT32U i=24+12;
    T_WEEKSEC tUtc0,tGlot0,tUtc1,tGlot1;
    T_YMD tYmd;

    /*解析电文*/
    if (i+348<=ptRtcm->iLen*8+24)
    {
        iPrn         =GetBitu(ptRtcm->cBuff,i,6);         i=i+6;
        tGeph.iFrq   =GetBitu(ptRtcm->cBuff,i,5)-7;       i=i+5+1+1+2;
        iTk_h        =GetBitu(ptRtcm->cBuff,i,5);         i=i+5;
        iTk_m        =GetBitu(ptRtcm->cBuff,i,6);         i=i+6;
        dTk_s        =GetBitu(ptRtcm->cBuff,i,1)*30.0;    i=i+1;
        tGeph.iSvh   =GetBitu(ptRtcm->cBuff,i,1);         i=i+1+1;
        iTb          =GetBitu(ptRtcm->cBuff,i,7);         i=i+7;
        tGeph.dVel[0]=GetBitg(ptRtcm->cBuff,i,24)*P2_20;  i=i+24;   /*Km*/
        tGeph.dPos[0]=GetBitg(ptRtcm->cBuff,i,27)*P2_11;  i=i+27;
        tGeph.dAcc[0]=GetBitg(ptRtcm->cBuff,i,5) *P2_30;  i=i+5;
        tGeph.dVel[1]=GetBitg(ptRtcm->cBuff,i,24)*P2_20;  i=i+24;
        tGeph.dPos[1]=GetBitg(ptRtcm->cBuff,i,27)*P2_11;  i=i+27;

        tGeph.dAcc[1]=GetBitg(ptRtcm->cBuff,i,5) *P2_30;  i=i+5;
        tGeph.dVel[2]=GetBitg(ptRtcm->cBuff,i,24)*P2_20;  i=i+24;
        tGeph.dPos[2]=GetBitg(ptRtcm->cBuff,i,27)*P2_11;  i=i+27;
        tGeph.dAcc[2]=GetBitg(ptRtcm->cBuff,i,5) *P2_30;  i=i+5+1;
        tGeph.dGamn  =GetBitg(ptRtcm->cBuff,i,11)*P2_40;  i=i+11+2+1;/*卫星相对频率偏差*/
        tGeph.dTaun  =GetBitg(ptRtcm->cBuff,i,22)*P2_30;             /*卫星钟偏差*/
    }
    else
    {
        fprintf(stderr,"RTCM 3 1020 length error: len=%d\n",ptRtcm->iLen);
        return -1;
    }

    /*获得卫星编号*/
    if(!(iSat=Prn2Sat(iSys,iPrn)))
    {
        return -1;
    }

    tGeph.iSat=iSat;
    tGeph.iIode=iTb&0x7F;

    tUtc0 =TimeAdd(ptRtcm->tTime,-LEAPS); /*GPS时转换成UTC时*/
    tGlot0=TimeAdd(tUtc0,10800); /*UTC时转换为GLO时*/
    tYmd=WeekSec2YMD(tGlot0);
    tYmd.iHor=iTk_h;
    tYmd.iMin=iTk_m;
    tYmd.dSec=dTk_s;
    tGlot1=YMD2WeekSec(tYmd);
    tUtc1=TimeAdd(tGlot1,-10800);
    /*电文帧时间 GLO导航电文中是UTC时，程序中是GPST*/
    tGeph.tTof=TimeAdd(tUtc1,LEAPS);

    tYmd=WeekSec2YMD(tGlot0);
    dSod=iTb*900.0;
    tYmd.iHor=(int)(dSod/3600.0);
    tYmd.iMin=(int)((dSod-tYmd.iHor*3600.0)/60.0);
    tYmd.dSec=dSod-tYmd.iHor*3600.0-tYmd.iMin*60.0;
    tGlot1=YMD2WeekSec(tYmd);
    tUtc1=TimeAdd(tGlot1,-10800);
    /*导航电文历元时间 GLO导航电文中是UTC时，程序中是GPST*/
    tGeph.tToc=TimeAdd(tUtc1,LEAPS);

    if (!strstr((char *)ptRtcm->cOpt,"-EPHALL"))
    {
        if (fabs(TimeDiff(tGeph.tToc,ptRtcm->tNav.ptGeph[iPrn-1].tToc))<1.0&&
            (tGeph.iSvh==ptRtcm->tNav.ptGeph[iPrn-1].iSvh))
        {
            return 0; /*不改变该卫星的广播星历*/
        }
    }

    ptRtcm->tNav.ptGeph[iPrn-1]=tGeph;
    ptRtcm->iEphsat=iSat;
    ptRtcm->tNav.iNg=ptRtcm->tNav.iNg+1;

    return 2;
}

/******************************************************************************
*    函数名称:  Decode_Type1046
*    功能描述:  解析1046类型：BDS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_Type1046(T_RTCM *ptRtcm)
{
    T_EPH tEph={0};
    INT32U iPrn,iSat,iSys=SYS_BDS;
    INT32U i=24+12;

    /*解析电文*/
    if (i+472<=ptRtcm->iLen*8+24)
    {
        iPrn          =GetBitu(ptRtcm->cBuff,i,6);               i=i+6;
        tEph.iWeek    =GetBitu(ptRtcm->cBuff,i,10)+1356;         i=i+10;
        tEph.iSva     =GetBitu(ptRtcm->cBuff,i,4);               i=i+4;
        //tEph.iCode    =GetBitu(ptRtcm->cBuff,i,2);               i=i+2;
        tEph.dIdot    =GetBits(ptRtcm->cBuff,i,14)*P2_43*SC2RAD; i=i+14;
        tEph.iIode    =GetBitu(ptRtcm->cBuff,i,8);               i=i+8;
        tEph.tToc.dSow=GetBitu(ptRtcm->cBuff,i,16)*16.0;         i=i+16;
        tEph.dF2      =GetBits(ptRtcm->cBuff,i,8)*P2_55;         i=i+8;
        tEph.dF1      =GetBits(ptRtcm->cBuff,i,16)*P2_43;        i=i+16;
        tEph.dF0      =GetBits(ptRtcm->cBuff,i,22)*P2_31;        i=i+22;
        tEph.iIodc    =GetBitu(ptRtcm->cBuff,i,10);              i=i+10;
        tEph.dCrs     =GetBits(ptRtcm->cBuff,i,16)*P2_5;         i=i+16;
        tEph.dDeln    =GetBits(ptRtcm->cBuff,i,16)*P2_43*SC2RAD; i=i+16;
        tEph.dM0      =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCuc     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dE       =GetBitu(ptRtcm->cBuff,i,32)*P2_33;        i=i+32;
        tEph.dCus     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dSqrtA   =GetBitu(ptRtcm->cBuff,i,32)*P2_19;        i=i+32;
        tEph.dToes    =GetBitu(ptRtcm->cBuff,i,16)*16.0;         i=i+16;
        tEph.dCic     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dOMG0    =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCis     =GetBits(ptRtcm->cBuff,i,16)*P2_29;        i=i+16;
        tEph.dI0      =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dCrc     =GetBits(ptRtcm->cBuff,i,16)*P2_5;         i=i+16;
        tEph.dOmg     =GetBits(ptRtcm->cBuff,i,32)*P2_31*SC2RAD; i=i+32;
        tEph.dOMGd    =GetBits(ptRtcm->cBuff,i,24)*P2_43*SC2RAD; i=i+24;
        tEph.dTgd[0]  =GetBits(ptRtcm->cBuff,i,8) *P2_31;        i=i+8;
        tEph.iSvh     =GetBitu(ptRtcm->cBuff,i,6);               i=i;
        /*tEph.iFlag    =GetBitu(ptRtcm->cBuff,i,1);               i=i+1;
        iFit          =GetBitu(ptRtcm->cBuff,i,1);*/
    }
    else
    {
        fprintf(stderr,"RTCM 3 1019 length error: len=%d\n",ptRtcm->iLen);
        return -1;
    }

    /*获得卫星编号*/
    if(!(iSat=Prn2Sat(iSys,iPrn)))
    {
        return -1;
    }

    tEph.iSat=iSat;
    tEph.tToc.iWeek=tEph.tToe.iWeek=tEph.iWeek;
    tEph.tToe.dSow=tEph.dToes;
    tEph.tTtr=tEph.tToc;
    //ptRtcm->tTime=tEph.tToc; /*BDS导航星历数据时间更新RTCM项目近似时间*/
    //tEph.dFit=dFit[iFit];  /*0:4hr,1:>4hr*/

    if (!strstr((char *)ptRtcm->cOpt,"-EPHALL"))
    {
        if (fabs(TimeDiff(tEph.tToc,ptRtcm->tNav.ptEph[iSat-1].tToc))<1.0) //(tEph.iIode==ptRtcm->tNav.ptEph[iSat-1].iIode)
        {
            return 0; /*不改变该卫星的广播星历*/
        }
    }

    ptRtcm->tNav.ptEph[iSat-1]=tEph;
    ptRtcm->iEphsat=iSat;
    ptRtcm->tNav.iN=ptRtcm->tNav.iN+1;

    return 2;
}

/******************************************************************************
*    函数名称:  SigIndex
*    功能描述:  获取信号下标
*    输入参数:  卫星系统，信号编码，信号频率，信号个数，选项，信号下标
*    输出参数:  信号下标
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void SigIndex(INT32U sys, const INT8U *code, const INT32U *freq, INT32U n,
                    const CHAR *opt, INT32U *ind)
{
    INT32U i,nex,pri,pri_h[8]={0},index[8]={0},ex[32]={0};

    /* test code priority */
    for (i=0;i<n;i++) {
        if (!code[i]) continue;

        if (freq[i]>NFREQ) { /* save as extended signal if freq > NFREQ */
            ex[i]=1;
            continue;
        }
        /* code priority */
        pri=GetCodepri(sys,code[i],opt);

        /* select highest priority signal */
        if (pri>pri_h[freq[i]-1]) {
            if (index[freq[i]-1]) ex[index[freq[i]-1]-1]=1;
            pri_h[freq[i]-1]=pri;
            index[freq[i]-1]=i+1;
        }
        else ex[i]=1;
    }
    /* signal index in obs data */
    for (i=nex=0;i<n;i++) {
        if (ex[i]==0) ind[i]=freq[i]-1;
        else if (nex<NEXOBS) ind[i]=NFREQ+nex++;
        else { /* no space in obs data */
            printf("rtcm msm: no space in obs data sys=%d code=%d\n",sys,code[i]);
            ind[i]=-1;
        }
    }
}

/******************************************************************************
*    函数名称:  Save_MSM_Obs
*    功能描述:  保存MSM观测值
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void Save_MSM_Obs(T_RTCM *ptRtcm, INT32S iSys, T_MSM_H *ptMSM_H, const FP64 *r,
                        const FP64 *pr, const FP64 *cp, const FP64 *rr,
                        const FP64 *rrf, const FP64 *cnr, const INT32S *lock,
                        const INT32S *ex, const INT32S *half)
{
    const CHAR *sig[32];
    FP64 tt,wl;
    INT8U code[32];
    CHAR *msm_type="",*q=NULL;
    INT32U i,j,	k,type,prn,sat,index=0,freq[32],ind[32];
    INT32S fn;
    type=GetBitu(ptRtcm->cBuff,24,12);

    switch (iSys) {
    case SYS_GPS: msm_type=q=ptRtcm->cMsmtype[0]; break;
    case SYS_GLO: msm_type=q=ptRtcm->cMsmtype[1]; break;
    case SYS_BDS: msm_type=q=ptRtcm->cMsmtype[2]; break;
    }
    /* id to signal */
    for (i=0;i<ptMSM_H->iNsig;i++) {
        switch (iSys) {
        case SYS_GPS: sig[i]=msm_sig_gps[ptMSM_H->iSigs[i]-1]; break;
        case SYS_GLO: sig[i]=msm_sig_glo[ptMSM_H->iSigs[i]-1]; break;
        case SYS_BDS: sig[i]=msm_sig_bds[ptMSM_H->iSigs[i]-1]; break;
        default: sig[i]=""; break;
        }
        /* signal to rinex obs type */
        code[i]=Obs2Code(sig[i],freq+i);

        /* freqency index for beidou */
        if (iSys==SYS_BDS) {
            if      (freq[i]==5) freq[i]=2; /* B2 */
            else if (freq[i]==4) freq[i]=3; /* B3 */
        }
        if (code[i]!=CODE_NONE) {
            if (q) q+=sprintf(q,"L%s%s",sig[i],i<ptMSM_H->iNsig-1?",":"");
        }
        else {
            if (q) q+=sprintf(q,"(%d)%s",ptMSM_H->iSigs[i],i<ptMSM_H->iNsig-1?",":"");

            printf("rtcm3 %d: unknown signal id=%2d\n",type,ptMSM_H->iSigs[i]);
        }
    }
    //fprintf(stderr,"rtcm3 %d: signals=%s\n",type,msm_type);

    /* get signal index */
    SigIndex(iSys,code,freq,ptMSM_H->iNsig,ptRtcm->cOpt,ind);

    for (i=j=0;i<ptMSM_H->iNsat;i++) {

        prn=ptMSM_H->iSats[i];
        if ((sat=Prn2Sat(iSys,prn))) {
            tt=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,ptRtcm->tTime);
            if (ptRtcm->iObsflag||fabs(tt)>1E-9) {
                ptRtcm->tObs.iN=ptRtcm->iObsflag=0;
            }
            index=ObsIndex(&ptRtcm->tObs,ptRtcm->tTime,sat);
        }
        else {
            printf("rtcm3 %d satellite error: prn=%d\n",type,prn);
        }
        for (k=0;k<ptMSM_H->iNsig;k++) {
            if (!ptMSM_H->iCellmask[k+i*ptMSM_H->iNsig]) continue;

            if (sat&&index>=0&&ind[k]>=0) {

                /* satellite carrier wave length */
                wl=SatWavelen(sat,freq[k]-1,&ptRtcm->tNav);

                /* glonass wave length by extended info */
                if (iSys==SYS_GLO&&ex&&ex[i]<=13) {
                    fn=ex[i]-7;
                    wl=CLIGHT/((freq[k]==2?FREQ2_GLO:FREQ1_GLO)+
                        (freq[k]==2?DFRQ2_GLO:DFRQ1_GLO)*fn);
                    ptRtcm->tObs.ptData[index].iFrq=fn;
                }
                if(wl>0){
                    ptRtcm->tObs.ptData[index].dFrq[ind[k]]=CLIGHT/wl;
                }
                /* pseudorange (m) */
                if (r[i]!=0.0&&pr[j]>-1E12) {
                    ptRtcm->tObs.ptData[index].dP[ind[k]]=r[i]+pr[j];
                }
                /* carrier-phase (cycle) */
                if (r[i]!=0.0&&cp[j]>-1E12&&wl>0.0) {
                    ptRtcm->tObs.ptData[index].dL[ind[k]]=(r[i]+cp[j])/wl;
                }
                /* doppler (hz) */
                if (rr&&rrf&&rrf[j]>-1E12&&wl>0.0) {
                    ptRtcm->tObs.ptData[index].fD[ind[k]]=(float)(-(rr[i]+rrf[j])/wl);
                }
                ptRtcm->tObs.ptData[index].cLLI[ind[k]]=
                    LossofLock(ptRtcm,sat,ind[k],lock[j])+(half[j]?3:0);
                ptRtcm->tObs.ptData[index].dCNR [ind[k]]=(unsigned char)(cnr[j]);  //输出载噪比
                ptRtcm->tObs.ptData[index].cCode[ind[k]]=code[k];
            }
            j++;
        }
    }

}

/******************************************************************************
*    函数名称:  Decode_MSM_HEAD
*    功能描述:  解析MSM_HEAD
*    输入参数:  RTCM结构体变量,卫星系统，MSM多电文标识，IODS，电文头结构体变量，
*               电文头结构体的大小
*    输出参数:  RTCM结构体变量
*    返 回 值:  MSM单元掩码的大小
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM_Head(T_RTCM *ptRtcm, INT32S iSys, INT32S *iSync, INT32S *iIod,
                             T_MSM_H *ptMSM_H, INT32S *iHsize)
{
    T_MSM_H h0={0};
    FP64 tow,tod;
    CHAR *msg;
    INT32U i=24,j,dow,mask,staid,type,ncell=0;

    type=GetBitu(ptRtcm->cBuff,i,12); i+=12;

    *ptMSM_H=h0;
    if (i+157<=(ptRtcm->iLen+3)*8) {
        staid     =GetBitu(ptRtcm->cBuff,i,12);       i+=12;

        if (iSys==SYS_GLO) {
            dow   =GetBitu(ptRtcm->cBuff,i, 3);       i+= 3;
            tod   =GetBitu(ptRtcm->cBuff,i,27)*0.001; i+=27;
            AdjDay_glot(ptRtcm,tod);
            ptRtcm->tTime=TimeAdd(ptRtcm->tTime,LEAPS);  //  加入跳秒

        }
        else if (iSys==SYS_BDS) {
            tow   =GetBitu(ptRtcm->cBuff,i,30)*0.001; i+=30;
            tow+=14.0; /* BDT -> GPST */
            AdjWeek(ptRtcm,tow);
        }
        else {
            tow   =GetBitu(ptRtcm->cBuff,i,30)*0.001; i+=30;
            AdjWeek(ptRtcm,tow);
        }
        *iSync     =GetBitu(ptRtcm->cBuff,i, 1);       i+= 1;
        *iIod      =GetBitu(ptRtcm->cBuff,i, 3);       i+= 3;
        ptMSM_H->iTime_s =GetBitu(ptRtcm->cBuff,i, 7);       i+= 7;
        ptMSM_H->iClk_str=GetBitu(ptRtcm->cBuff,i, 2);       i+= 2;
        ptMSM_H->iClk_ext=GetBitu(ptRtcm->cBuff,i, 2);       i+= 2;
        ptMSM_H->iSmooth =GetBitu(ptRtcm->cBuff,i, 1);       i+= 1;
        ptMSM_H->iTint_s =GetBitu(ptRtcm->cBuff,i, 3);       i+= 3;
        for (j=1;j<=64;j++) {
            mask=GetBitu(ptRtcm->cBuff,i,1); i+=1;
            if (mask) ptMSM_H->iSats[ptMSM_H->iNsat++]=j;
        }
        for (j=1;j<=32;j++) {
            mask=GetBitu(ptRtcm->cBuff,i,1); i+=1;
            if (mask) ptMSM_H->iSigs[ptMSM_H->iNsig++]=j;
        }
    }
    else {
        fprintf(stderr,"rtcm3 %d length error: len=%d\n",type,ptRtcm->iLen+3);
        return -1;
    }
    /* test station id */
    if (!Test_Staid(ptRtcm,staid)) return -1;

    if (ptMSM_H->iNsat*ptMSM_H->iNsig>64) {
        fprintf(stderr,"rtcm3 %d number of sats and sigs error: nsat=%d nsig=%d\n",
            type,ptMSM_H->iNsat,ptMSM_H->iNsig);
        return -1;
    }
    if (i+ptMSM_H->iNsat*ptMSM_H->iNsig>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: len=%d nsat=%d nsig=%d\n",type,
            ptRtcm->iLen+3,ptMSM_H->iNsat,ptMSM_H->iNsig);
        return -1;
    }
    for (j=0;j<ptMSM_H->iNsat*ptMSM_H->iNsig;j++) {
        ptMSM_H->iCellmask[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
        if (ptMSM_H->iCellmask[j]) ncell++;
    }
    *iHsize=i;

    if (ptRtcm->outtype) {
        msg=ptRtcm->cMsgtype+strlen(ptRtcm->cMsgtype);
        sprintf(msg," week=%d sec=%f staid=%3d nsat=%2d nsig=%2d iod=%2d ncell=%2d sync=%d",
            ptRtcm->tTime.iWeek,ptRtcm->tTime.dSow,staid,ptMSM_H->iNsat,ptMSM_H->iNsig,*iIod,ncell,*iSync);
    }
    return ncell;
}

/******************************************************************************
*    函数名称:  Decode_MSM1
*    功能描述:  解析MSM1
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM1(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    INT32S i,sync,iod;
    FP64 r[64],pr[64];
    INT32S j,type,ncell,rng_m,prv;

    type=GetBitu(ptRtcm->cBuff,24,12);

    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*10+ncell*15>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) r[j]=0.0;
    for (j=0;j<ncell;j++) pr[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (prv!=-16384) pr[j]=prv*P2_24*RANGE_MS;
    }

    ptRtcm->iObsflag=!sync;
    return sync?0:1;
}

/******************************************************************************
*    函数名称:  Decode_MSM2
*    功能描述:  解析MSM2
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM2(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    INT32S i,sync,iod;
    FP64 r[64],cp[64];
    INT32S j,type,ncell,rng_m,cpv,lock[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*10+ncell*27>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) r[j]=0.0;
    for (j=0;j<ncell;j++) cp[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,22); i+=22;
        if (cpv!=-2097152) cp[j]=cpv*P2_29*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,4); i+=4;
    }
    for (j=0;j<ncell;j++) { /* half-cycle ambiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }

    ptRtcm->iObsflag=!sync;
    return sync?0:1;
}
/******************************************************************************
*    函数名称:  Decode_MSM3
*    功能描述:  解析MSM3
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM3(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    INT32S i,sync,iod;

    FP64 r[64],pr[64],cp[64];
    INT32S j,type,ncell,rng_m,prv,cpv,lock[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    if (ncell=(Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*10+ncell*42>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) r[j]=0.0;
    for (j=0;j<ncell;j++) pr[j]=cp[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (prv!=-16384) pr[j]=prv*P2_24*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,22); i+=22;
        if (cpv!=-2097152) cp[j]=cpv*P2_29*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,4); i+=4;
    }
    for (j=0;j<ncell;j++) { /* half-cycle ambiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }

    ptRtcm->iObsflag=!sync;
    return sync?0:1;
}

/******************************************************************************
*    函数名称:  Decode_MSM4
*    功能描述:  解析MSM4
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM4(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    FP64 r[64],pr[64],cp[64],cnr[64];
    INT32S i,j,type,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    /* decode msm header */
    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*18+ncell*48>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) { /* range */
        rng  =GetBitu(ptRtcm->cBuff,i, 8); i+= 8;
        if (rng!=255) r[j]=rng*RANGE_MS;
        else r[j]=0.0;
    }
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (prv!=-16384) pr[j]=prv*P2_24*RANGE_MS;
        else pr[j]=-1E16;
    }
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,22); i+=22;
        if (cpv!=-2097152) cp[j]=cpv*P2_29*RANGE_MS;
        else cp[j]=-1E16;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,4); i+=4;
    }
    for (j=0;j<ncell;j++) { /* half-cycle ambiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }
    for (j=0;j<ncell;j++) { /* cnr */
        cnr[j]=GetBitu(ptRtcm->cBuff,i,6)*1.0; i+=6;
    }
    /* save obs data in msm message */
    Save_MSM_Obs(ptRtcm,iSys,&h,r,pr,cp,NULL,NULL,cnr,lock,NULL,half);

    ptRtcm->iObsflag=!sync;
    return sync?0:1;

}

/******************************************************************************
*    函数名称:  Decode_MSM5
*    功能描述:  解析MSM5
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM5(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    FP64 r[64],rr[64],pr[64],cp[64],rrf[64],cnr[64];
    INT32S i,j,type,sync,iod,ncell,rng,rng_m,rate,prv,cpv,rrv,lock[64];
    INT32S ex[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    /* decode msm header */
    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*36+ncell*63>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) {
        r[j]=rr[j]=0.0; ex[j]=15;
    }
    for (j=0;j<ncell;j++) pr[j]=cp[j]=rrf[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) { /* range */
        rng  =GetBitu(ptRtcm->cBuff,i, 8); i+= 8;
        if (rng!=255) r[j]=rng*RANGE_MS;
    }
    for (j=0;j<h.iNsat;j++) { /* extended info */
        ex[j]=GetBitu(ptRtcm->cBuff,i, 4); i+= 4;
    }
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    for (j=0;j<h.iNsat;j++) { /* phaserangerate */
        rate =GetBits(ptRtcm->cBuff,i,14); i+=14;
        if (rate!=-8192) rr[j]=rate*1.0;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (prv!=-16384) pr[j]=prv*P2_24*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,22); i+=22;
        if (cpv!=-2097152) cp[j]=cpv*P2_29*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,4); i+=4;
    }
    for (j=0;j<ncell;j++) { /* half-cycle ambiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }
    for (j=0;j<ncell;j++) { /* cnr */
        cnr[j]=GetBitu(ptRtcm->cBuff,i,6)*1.0; i+=6;
    }
    for (j=0;j<ncell;j++) { /* phaserange rate */
        rrv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (rrv!=-16384) rrf[j]=rrv*0.0001;
    }
    /* save obs data in msm message */
    Save_MSM_Obs(ptRtcm,iSys,&h,r,pr,cp,rr,rrf,cnr,lock,ex,half);

    ptRtcm->iObsflag=!sync;
    return sync?0:1;

}

/******************************************************************************
*    函数名称:  Decode_MSM6
*    功能描述:  解析MSM6
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM6(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    FP64 r[64],pr[64],cp[64],cnr[64];
    INT32S i,j,type,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    /* decode msm header */
    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*18+ncell*65>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) r[j]=0.0;
    for (j=0;j<ncell;j++) pr[j]=cp[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) { /* range */
        rng  =GetBitu(ptRtcm->cBuff,i, 8); i+= 8;
        if (rng!=255) r[j]=rng*RANGE_MS;
    }
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,20); i+=20;
        if (prv!=-524288) pr[j]=prv*P2_29*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,24); i+=24;
        if (cpv!=-8388608) cp[j]=cpv*P2_31*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,10); i+=10;
    }
    for (j=0;j<ncell;j++) { /* half-cycle ambiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }
    for (j=0;j<ncell;j++) { /* cnr */
        cnr[j]=GetBitu(ptRtcm->cBuff,i,10)*0.0625; i+=10;
    }
    /* save obs data in msm message */
    Save_MSM_Obs(ptRtcm,iSys,&h,r,pr,cp,NULL,NULL,cnr,lock,NULL,half);

    ptRtcm->iObsflag=!sync;
    return sync?0:1;
}

/******************************************************************************
*    函数名称:  Decode_MSM7
*    功能描述:  解析MSM7
*    输入参数:  RTCM结构体变量,卫星系统类型
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S Decode_MSM7(T_RTCM *ptRtcm, INT32S iSys)
{
    T_MSM_H h={0};
    FP64 r[64],rr[64],pr[64],cp[64],rrf[64],cnr[64];
    INT32S i,j,type,sync,iod,ncell,rng,rng_m,rate,prv,cpv,rrv,lock[64];
    INT32S ex[64],half[64];

    type=GetBitu(ptRtcm->cBuff,24,12);

    /* decode msm header */
    if ((ncell=Decode_MSM_Head(ptRtcm,iSys,&sync,&iod,&h,&i))<0) return -1;

    if (i+h.iNsat*36+ncell*80>(ptRtcm->iLen+3)*8) {
        fprintf(stderr,"rtcm3 %d length error: nsat=%d ncell=%d len=%d\n",type,h.iNsat,
            ncell,ptRtcm->iLen+3);
        return -1;
    }
    for (j=0;j<h.iNsat;j++) {
        r[j]=rr[j]=0.0; ex[j]=15;
    }
    for (j=0;j<ncell;j++) pr[j]=cp[j]=rrf[j]=-1E16;

    /* decode satellite data */
    for (j=0;j<h.iNsat;j++) { /* range */
        rng  =GetBitu(ptRtcm->cBuff,i, 8); i+= 8;
        if (rng!=255) r[j]=rng*RANGE_MS;
    }
    for (j=0;j<h.iNsat;j++) { /* extended info */
        ex[j]=GetBitu(ptRtcm->cBuff,i, 4); i+= 4;
    }
    for (j=0;j<h.iNsat;j++) {
        rng_m=GetBitu(ptRtcm->cBuff,i,10); i+=10;
        if (r[j]!=0.0) r[j]+=rng_m*P2_10*RANGE_MS;
    }
    for (j=0;j<h.iNsat;j++) { /* phaserangerate */
        rate =GetBits(ptRtcm->cBuff,i,14); i+=14;
        if (rate!=-8192) rr[j]=rate*1.0;
    }
    /* decode signal data */
    for (j=0;j<ncell;j++) { /* pseudorange */
        prv=GetBits(ptRtcm->cBuff,i,20); i+=20;
        if (prv!=-524288) pr[j]=prv*P2_29*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* phaserange */
        cpv=GetBits(ptRtcm->cBuff,i,24); i+=24;
        if (cpv!=-8388608) cp[j]=cpv*P2_31*RANGE_MS;
    }
    for (j=0;j<ncell;j++) { /* lock time */
        lock[j]=GetBitu(ptRtcm->cBuff,i,10); i+=10;
    }
    for (j=0;j<ncell;j++) { /* half-cycle amiguity */
        half[j]=GetBitu(ptRtcm->cBuff,i,1); i+=1;
    }
    for (j=0;j<ncell;j++) { /* cnr */
        cnr[j]=GetBitu(ptRtcm->cBuff,i,10)*0.0625; i+=10;
    }
    for (j=0;j<ncell;j++) { /* phaserangerate */
        rrv=GetBits(ptRtcm->cBuff,i,15); i+=15;
        if (rrv!=-16384) rrf[j]=rrv*0.0001;
    }
    /* save obs data in msm message */
    Save_MSM_Obs(ptRtcm,iSys,&h,r,pr,cp,rr,rrf,cnr,lock,ex,half);

    ptRtcm->iObsflag=!sync;
    return sync?0:1;
}

/******************************************************************************
*    函数名称:  Decode_RTCM3
*    功能描述:  解析RTCM3电文
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S Decode_RTCM3(T_RTCM *ptRtcm)
{
    INT32S iMark=0;
    INT32U iType;
    iType=GetBitu(ptRtcm->cBuff,24,12);

    switch (iType)
    {
        case 1001:iMark=Decode_Type1001(ptRtcm);  /*不支持*/
                  break;
        case 1002:iMark=Decode_Type1002(ptRtcm);
                  break;
        case 1003:iMark=Decode_Type1003(ptRtcm);  /*不支持*/
                  break;
        case 1004:iMark=Decode_Type1004(ptRtcm);
                  break;
        case 1005:iMark=Decode_Type1005(ptRtcm);
                  break;
        case 1006:iMark=Decode_Type1006(ptRtcm);
                  break;
        case 1007:iMark=Decode_Type1007(ptRtcm);
                  break;
        case 1008:iMark=Decode_Type1008(ptRtcm);
                  break;
        case 1009:iMark=Decode_Type1009(ptRtcm);  /*不支持*/
                  break;
        case 1010:iMark=Decode_Type1010(ptRtcm);
                  break;
        case 1011:iMark=Decode_Type1011(ptRtcm);  /*不支持*/
                  break;
        case 1012:iMark=Decode_Type1012(ptRtcm);
                  break;
        case 4087:iMark=Decode_Type4087(ptRtcm);  /*不支持*/
                  break;
        case 4088:iMark=Decode_Type4088(ptRtcm);
                  break;
        case 4091:iMark=Decode_Type4091(ptRtcm);  /*不支持*/
                  break;
        case 4092:iMark=Decode_Type4092(ptRtcm);
                  break;
        case 1019:iMark=Decode_Type1019(ptRtcm);
                  break;
        case 1020:iMark=Decode_Type1020(ptRtcm);
                  break;
        case 1046:iMark=Decode_Type1046(ptRtcm);
                  break;
        case 1071:iMark=Decode_MSM1(ptRtcm,SYS_GPS);
            break;
        case 1072:iMark=Decode_MSM2(ptRtcm,SYS_GPS);
            break;
        case 1073:iMark=Decode_MSM3(ptRtcm,SYS_GPS);
            break;
        case 1074:iMark=Decode_MSM4(ptRtcm,SYS_GPS);
            break;
        case 1075:iMark=Decode_MSM5(ptRtcm,SYS_GPS);
            break;
        case 1076:iMark=Decode_MSM6(ptRtcm,SYS_GPS);
            break;
        case 1077:iMark=Decode_MSM7(ptRtcm,SYS_GPS);
            break;
        case 1081:iMark=Decode_MSM1(ptRtcm,SYS_GLO);
            break;
        case 1082:iMark=Decode_MSM2(ptRtcm,SYS_GLO);
            break;
        case 1083:iMark=Decode_MSM3(ptRtcm,SYS_GLO);
            break;
        case 1084:iMark=Decode_MSM4(ptRtcm,SYS_GLO);
            break;
        case 1085:iMark=Decode_MSM5(ptRtcm,SYS_GLO);
            break;
        case 1086:iMark=Decode_MSM6(ptRtcm,SYS_GLO);
            break;
        case 1087:iMark=Decode_MSM7(ptRtcm,SYS_GLO);
            break;
        case 1121:iMark=Decode_MSM1(ptRtcm,SYS_BDS);
            break;
        case 1122:iMark=Decode_MSM2(ptRtcm,SYS_BDS);
            break;
        case 1123:iMark=Decode_MSM3(ptRtcm,SYS_BDS);
            break;
        case 1124:iMark=Decode_MSM4(ptRtcm,SYS_BDS);
            break;
        case 1125:iMark=Decode_MSM5(ptRtcm,SYS_BDS);
            break;
        case 1126:iMark=Decode_MSM6(ptRtcm,SYS_BDS);
            break;
        case 1127:iMark=Decode_MSM7(ptRtcm,SYS_BDS);
            break;
        default:  break;
    }

    ptRtcm->iType = iType;

    /*统计不同类型电文个数*/
    if (iMark>=0)
    {
        iType=iType-1000;
        if (iType>1000) /*RTCM中BDS电文*/
        {
            iType=iType-3000;
        }
        if ((iType>=1)&&(iType<=299))
        {
            ptRtcm->iNmsg3[iType]=ptRtcm->iNmsg3[iType]+1;
        }
        else
        {
            ptRtcm->iNmsg3[0]=ptRtcm->iNmsg3[0]+1;
        }
    }

    return iMark;
}

/******************************************************************************
*    函数名称:  Get_Type_From_Sat_Num
*    功能描述:  get sat type by sat num
*    输入参数:  sat num
*    输出参数:  sat type
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern int Get_Type_From_RTCM_Num(int nSatNum)
{
    INT32S sys=Sat2Sys(nSatNum,NULL);
    int nRet = -1;
    switch(sys)
    {
    case SYS_GPS:
        nRet = 0;
        break;
    case SYS_BDS:
        nRet = 1;
        break;
    case SYS_GLO:
        nRet = 2;
        break;
    default:
        nRet = -1;
        break;
    }
    return nRet;
}

/******************************************************************************
*    函数名称:  Get_Type_From_Sat_Num
*    功能描述:  get sat type by sat num
*    输入参数:  sat num
*    输出参数:  sat type
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern int Get_Num_From_RTCM_Num(int nSatNum)
{
    INT32S sys=Sat2Sys(nSatNum,NULL);
    int nRet = -1;
    switch(sys)
    {
    case SYS_GPS:
        nSatNum = nSatNum+MINPRNGPS-1;
        nRet = nSatNum + 100;
        break;
    case SYS_BDS:
        nSatNum = nSatNum-NSATGPS+MINPRNBDS-1;
        nRet = nSatNum + 200;
        break;
    case SYS_GLO:
        nSatNum = nSatNum-NSATGPS-NSATBDS+MINPRNGLO-1;
        nRet = nSatNum + 300;
        break;
    default:
        nRet = -1;
        break;
    }
    return nRet;
}




