/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：funct_Rtcm2De.cpp
* 内容摘要：RTCM 2.3电文编码
*           1001-1004             GPS观测值
*      		1005-1006             基准站信息
*   		1007-1008             天线信息
*   		1009-1012             GLONASS观测值
*   		4087-4088,4091-4092   BDS观测值
*   		1019-1020,4095        GPS/GLONASS/BDS卫星星历
* 其他说明：
* 版本  号：
* 作    者：sbz
* 完成日期：2017-02-14
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

/* adjust hourly rollover of rtcm 2 time -------------------------------------*/
//根据RTCM2中“修正ZS计数”修正时间
static void AdjHour(T_RTCM *ptRtcm, FP64 dZcnt)
{
	FP64 dTow,dHour,dSec;

	dTow=ptRtcm->tTime.dSow;
	dHour=floor(dTow/3600.0);
	dSec=dTow-dHour*3600.0;

	if (dZcnt<dSec-1800.0)
	{
		dZcnt=dZcnt+3600.0;
	}
	else if (dZcnt>dSec+1800.0)
	{
		dZcnt=dZcnt-3600.0;
	}

	ptRtcm->tTime.dSow=dHour*3600+dZcnt;
}

//得到观测值存储位置索引
static INT32S ObsIndex(T_OBS *tObs, T_WEEKSEC tWSec, INT32U iSat)
{
	T_OBSSAT tData0={{0}};
	INT32U i;

	for (i=0;i<tObs->iN;i++)
	{
		if (tObs->ptData[i].iSat==iSat) //卫星数据是否已存在
		{
			return i;
		}
	}
	if( i>MAXOBS)
	{
		return -1;
	}
	tObs->ptData[i]=tData0; //初始化
	tObs->iN=tObs->iN+1; //观测值个数增加1个
	tObs->ptData[i].tObsTime=tWSec; //观测时间
	tObs->ptData[i].iSat=iSat; //卫星编号

	return i;
}

//解析电文类型1/9：差分GPS伪距改正信息
static INT32S Decode_Type1 (T_RTCM *ptRtcm)
{
	INT32U iFact,iUdre,iPrn;
	INT32S iPrc,iRrc,iIod;
	INT32U iSat,iSys=SYS_GPS;
	INT32U i=48;

	while (i+40<=ptRtcm->iLen*8) 
	{
		iFact=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iUdre=GetBitu(ptRtcm->cBuff,i, 2); i+= 2;
		iPrn =GetBitu(ptRtcm->cBuff,i, 5); i+= 5;
		iPrc =GetBits(ptRtcm->cBuff,i,16); i+=16;
		iRrc =GetBits(ptRtcm->cBuff,i, 8); i+= 8;
		iIod =GetBits(ptRtcm->cBuff,i, 8); i+= 8;

		if (iPrn==0) 
		{
			iPrn=32;
		}

		if (iPrc==0x80000000||iPrc==0xFFFF8000) 
		{
			continue;
		}

		/*获得卫星编号*/
		if(!(iSat=Prn2Sat(iSys,iPrn)))
		{
			continue;
		}
		ptRtcm->tDGnss[iSat-1].tObsTime=ptRtcm->tTime;
		ptRtcm->tDGnss[iSat-1].dPrc=iPrc*(iFact?0.32:0.02);
		ptRtcm->tDGnss[iSat-1].dRrc=iRrc*(iFact?0.032:0.002);
		ptRtcm->tDGnss[iSat-1].iIod=iIod;
		ptRtcm->tDGnss[iSat-1].iUdre=iUdre;
	}
	return 7;
}

//解析电文类型3：基准站位置信息
static INT32S Decode_Type3(T_RTCM *ptRtcm)
{
	INT32U i=48;

	if (i+96<=ptRtcm->iLen*8)
	{
		ptRtcm->tSta.dAppPos[0]=GetBits(ptRtcm->cBuff,i,32)*0.01; i+=32;
		ptRtcm->tSta.dAppPos[1]=GetBits(ptRtcm->cBuff,i,32)*0.01; i+=32;
		ptRtcm->tSta.dAppPos[2]=GetBits(ptRtcm->cBuff,i,32)*0.01;
	}
	else 
	{
		return -1;
	}
	return 5;
}

//解析电文类型14：GPS周时间
static INT32S Decode_Type14(T_RTCM *ptRtcm)
{
	FP64 dZcnt;
	INT32U iWeek,iHour,iLeaps;
	INT32U i=48;

	dZcnt =GetBitu(ptRtcm->cBuff,24,13)*0.6;

	if (i+24<=ptRtcm->iLen*8) 
	{
		iWeek =GetBitu(ptRtcm->cBuff,i,10); i+=10;
		iHour =GetBitu(ptRtcm->cBuff,i, 8); i+= 8;
		iLeaps=GetBitu(ptRtcm->cBuff,i, 6);
	}
	else 
	{
		return -1;
	}
	
	ptRtcm->tTime.iWeek=iWeek;
	ptRtcm->tTime.dSow=iHour*3600.0+dZcnt;
	ptRtcm->tNav.iLeaps=iLeaps;

	return 6;
}

//解析电文类型16：GPS专用电文
static INT32S Decode_Type16(T_RTCM *ptRtcm)
{
	INT32U iN=0;
	INT32U i=48;

	while ((i+8<=ptRtcm->iLen*8)&&(iN<90)) 
	{
		ptRtcm->cMsg[iN]=GetBitu(ptRtcm->cBuff,i,8); i+=8;
		iN=iN+1;
	}

	ptRtcm->cMsg[iN]='\0';

	return 9;
}

//解析电文类型17：GPS星历
static INT32S Decode_Type17(T_RTCM *ptRtcm)
{
	return 0;
}

//解析电文类型18：RTK载波相位原始观测值
static INT32S Decode_Type18(T_RTCM *ptRtcm)
{
	T_WEEKSEC tTime;
	FP64 dUsec,dDSec;
	INT32U iFreq,iSync=1,iCode,iSys,iPrn,iLoss;
	INT32S iCp,iIndex;
	INT32U iSat;
	INT32U i=48;


	if (i+24<=ptRtcm->iLen*8) 
	{
		iFreq=GetBitu(ptRtcm->cBuff,i, 2); i+= 2+2;
		dUsec=GetBitu(ptRtcm->cBuff,i,20)*1E-6; i+=20;
	}
	else 
	{
		return -1;
	}

	if (iFreq&0x1) 
	{
		return -1;
	}
	iFreq>>=1;

	while ((i+48<=ptRtcm->iLen*8)&&(ptRtcm->tObs.iN<MAXOBS))
	{
		iSync=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iCode=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iSys =GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iPrn =GetBitu(ptRtcm->cBuff,i, 5); i+= 5+3;
		iLoss=GetBitu(ptRtcm->cBuff,i, 5); i+= 5;
		iCp  =GetBits(ptRtcm->cBuff,i,32); i+=32;

		if (iPrn==0)
		{
			iPrn=32;
		}

		if (!(iSat=Prn2Sat(iSys?SYS_GLO:SYS_GPS,iPrn)))
		{
			continue;
		}

		//修正观测时间
		tTime=TimeAdd(ptRtcm->tTime,dUsec);
		if (iSys) //GLONASS时间转换成GPS时间 
		{
			tTime=TimeAdd(ptRtcm->tTime,LEAPS); 
		}

		/*判断是否是同一时刻的观测数据*/
		dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,tTime);

		if ((ptRtcm->iObsflag==1)||(fabs(dDSec)>1E-9)) 
		{
			ptRtcm->tObs.iN=0;
			ptRtcm->iObsflag=0;
		}

		if ((iIndex=ObsIndex(&ptRtcm->tObs,tTime,iSat))>=0) 
		{
			ptRtcm->tObs.ptData[iIndex].dL[iFreq]=-iCp/256.0;
			ptRtcm->tObs.ptData[iIndex].cLLI[iFreq]=(ptRtcm->cLoss[iSat-1][iFreq]!=iLoss);
			ptRtcm->tObs.ptData[iIndex].cCode[iFreq]=
				!iFreq?(iCode?CODE_L1P:CODE_L1C):(iCode?CODE_L2P:CODE_L2C);
			ptRtcm->cLoss[iSat-1][iFreq]=iLoss;
		}
	}
	ptRtcm->iObsflag=!iSync;

	return ptRtcm->iObsflag;
}

//解析电文类型19：RTK伪距原始观测值
static INT32S Decode_Type19(T_RTCM *ptRtcm)
{
	T_WEEKSEC tTime;
	FP64 dUsec,dDSec;
	INT32U iFreq,iSync=1,iCode,iSys,iPrn,iPR;
	INT32U iSat;
	INT32S iIndex;
	INT32U i=48;

	if (i+24<=ptRtcm->iLen*8) 
	{
		iFreq=GetBitu(ptRtcm->cBuff,i, 2);      i+= 2+2;
		dUsec=GetBitu(ptRtcm->cBuff,i,20)*1E-6; i+=20;
	}
	else 
	{
		return -1;
	}

	if (iFreq&0x1)
	{
		return -1;
	}
	iFreq>>=1;

	while ((i+48<=ptRtcm->iLen*8)&&(ptRtcm->tObs.iN<MAXOBS)) 
	{
		iSync=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iCode=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iSys =GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iPrn =GetBitu(ptRtcm->cBuff,i, 5); i+= 5+8;
		iPR  =GetBitu(ptRtcm->cBuff,i,32); i+=32;

		if (iPrn==0) 
		{
			iPrn=32;
		}

		if (!(iSat=Prn2Sat(iSys?SYS_GLO:SYS_GPS,iPrn)))
		{
			continue;
		}

		//修正观测时间
		tTime=TimeAdd(ptRtcm->tTime,dUsec);
		if (iSys) //GLONASS时间转换成GPS时间 
		{
			tTime=TimeAdd(ptRtcm->tTime,LEAPS); 
		}

		/*判断是否是同一时刻的观测数据*/
		dDSec=TimeDiff(ptRtcm->tObs.ptData[0].tObsTime,tTime);

		if ((ptRtcm->iObsflag==1)||(fabs(dDSec)>1E-9)) 
		{
			ptRtcm->tObs.iN=0;
			ptRtcm->iObsflag=0;
		}

		if ((iIndex=ObsIndex(&ptRtcm->tObs,tTime,iSat))>=0) 
		{
			ptRtcm->tObs.ptData[iIndex].dP[iFreq]=iPR*0.02;
			ptRtcm->tObs.ptData[iIndex].cCode[iFreq]=
				!iFreq?(iCode?CODE_L1P:CODE_L1C):(iCode?CODE_L2P:CODE_L2C);
		}
	}
	ptRtcm->iObsflag=!iSync;

	return ptRtcm->iObsflag;
}

//解析电文类型20：RTK载波相位改正信息
static INT32S Decode_Type20(T_RTCM *ptRtcm)
{
	return 0;
}

//解析电文类型21：RTK/高精度伪距改正信息
static INT32S Decode_Type21(T_RTCM *ptRtcm)
{
	return 0;
}

//解析电文类型41/42：差分BDS伪距改正信息
static INT32S Decode_Type41 (T_RTCM *ptRtcm)
{
	INT32U iFact,iUdre,iPrn;
	INT32S iPrc,iRrc,iIod;
	INT32U iSat,iSys=SYS_BDS;
	INT32U i=48;

	while (i+40<=ptRtcm->iLen*8) 
	{
		iFact=GetBitu(ptRtcm->cBuff,i, 1); i+= 1;
		iUdre=GetBitu(ptRtcm->cBuff,i, 2); i+= 2;
		iPrn =GetBitu(ptRtcm->cBuff,i, 5); i+= 5;
		iPrc =GetBits(ptRtcm->cBuff,i,16); i+=16;
		iRrc =GetBits(ptRtcm->cBuff,i, 8); i+= 8;
		iIod =GetBits(ptRtcm->cBuff,i, 8); i+= 8;

		if (iPrn==0) 
		{
			iPrn=32;
		}

		if (iPrc==0x80000000||iPrc==0xFFFF8000) 
		{
			continue;
		}

		/*获得卫星编号*/
		if(!(iSat=Prn2Sat(iSys,iPrn)))
		{
			continue;
		}
		ptRtcm->tDGnss[iSat-1].tObsTime=ptRtcm->tTime;
		ptRtcm->tDGnss[iSat-1].dPrc=iPrc*(iFact?0.32:0.02);
		ptRtcm->tDGnss[iSat-1].dRrc=iRrc*(iFact?0.032:0.002);
		ptRtcm->tDGnss[iSat-1].iIod=iIod;
		ptRtcm->tDGnss[iSat-1].iUdre=iUdre;
	}
	return 7;
}

//解析RTCM2电文
extern INT32S Decode_RTCM2(T_RTCM *ptRtcm)
{
	FP64 dZcnt;
	INT32U iStaID,iSeqno,iStaH,iType;
	INT32S iMark=0;

	iType =GetBitu(ptRtcm->cBuff,8,6);
	iStaID=GetBitu(ptRtcm->cBuff,14,10);
	dZcnt =GetBitu(ptRtcm->cBuff,24,13)*0.6;

	fprintf(stderr,"the rtcm type %d\n",iType);

	if (dZcnt>=3600.0)
	{
		return -1;
	}

    //根据RTCM2中“修正ZS计数”修正时间
	AdjHour(ptRtcm,dZcnt);

	ptRtcm->iSeqno=GetBitu(ptRtcm->cBuff,37, 3);
	ptRtcm->iStah =GetBitu(ptRtcm->cBuff,45, 3);

	//基准站ID一致性判断
	if (iType==3||iType==22||iType==23||iType==24) 
	{
		ptRtcm->iStaid=iStaID;
	}

	if ((ptRtcm->iStaid!=0)&&(iStaID!=ptRtcm->iStaid)) 
	{
		return -1;
	}

	switch (iType) 
	{
		case  1: iMark=Decode_Type1 (ptRtcm); 
			     break;
		case  3: iMark=Decode_Type3 (ptRtcm);
			     break;
		case  9: iMark=Decode_Type1 (ptRtcm); 
			     break;
		case 14: iMark=Decode_Type14(ptRtcm); 
			     break;
		case 16: iMark=Decode_Type16(ptRtcm); 
			     break;
		case 17: iMark=Decode_Type17(ptRtcm); //不支持
			     break;
		case 18: iMark=Decode_Type18(ptRtcm); 
			     break;
		case 19: iMark=Decode_Type19(ptRtcm); 
			     break;
		case 20: iMark=Decode_Type20(ptRtcm); //不支持
			     break;
		case 21: iMark=Decode_Type21(ptRtcm); //不支持
			     break;
		case 41: iMark=Decode_Type41(ptRtcm); 
			     break;
		case 42: iMark=Decode_Type41(ptRtcm); 
			     break;  
	}

	if (iMark>=0) 
	{
		if (1<=iType&&iType<=99) 
		{
			ptRtcm->iNmsg2[iType]=ptRtcm->iNmsg2[iType]+1; 
		}
		else
		{
			ptRtcm->iNmsg2[0]=ptRtcm->iNmsg2[iType]+1;
		}
	}
	return iMark;
}