/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：funct_Rtcm3En.cpp
* 内容摘要：RTCM 3.1电文编码
*           1001-1004             GPS观测值
*      		1005-1006             基准站信息
*   		1007-1008             天线信息
*   		1009-1012             GLONASS观测值
*   		4087-4088,4091-4092   BDS观测值
*   		1019-1020,4095        GPS/GLONASS/BDS卫星星历
* 其他说明：
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
*    函数名称:  ROUND_U
*    功能描述:  取整返回无符号型（四舍五入）
*    输入参数:  浮点型数据
*    输出参数:  无
*    返 回 值:  取整后无符号型整数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U ROUND_U(FP64 dData)
{
	return (unsigned int)floor(dData+0.5);
}

/******************************************************************************
*    函数名称:  ROUND_S
*    功能描述:  取整返回有符号型（四舍五入）
*    输入参数:  浮点型数据
*    输出参数:  无
*    返 回 值:  取整后有符号型整数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S ROUND_S(FP64 dData)
{
	return (int)floor(dData+0.5);
}

/******************************************************************************
*    函数名称:  LockTime
*    功能描述:  锁定时间
*    输入参数:  观测时间，上一次失锁时间，失锁标识
*    输出参数:  无
*    返 回 值:  失锁间隔
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S LockTime(T_WEEKSEC tTime,T_WEEKSEC *ptLltime,INT8U cLLI)
{
	FP64 dSec;
	
	if ((!ptLltime)||(cLLI&1))
	{
		*ptLltime=tTime;
	}
	dSec=TimeDiff(tTime,*ptLltime);

	return (int)dSec;
}

/******************************************************************************
*    函数名称:  To_Lock
*    功能描述:  锁定时间标识
*    输入参数:  失锁间隔
*    输出参数:  无
*    返 回 值:  失锁标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Lock(INT32S iLock)
{
	if (iLock<0)
	{
		return 0;
	}
	else if (iLock<24)
	{
		return iLock;
	}
	else if (iLock<72)
	{
		return (iLock+24)/2;
	}
	else if (iLock<168)
	{
		return (iLock+120)/4;
	}
	else if (iLock<360)
	{
		return (iLock+408)/8;
	}
	else if (iLock<744)
	{
		return (iLock+1176)/16;
	}
	else if (iLock<937)
	{
		return (iLock+3096)/32;
	}
	else
	{
		return 127;
	}
}

/******************************************************************************
*    函数名称:  To_Code1_GPS
*    功能描述:  GPS L1编码标识
*    输入参数:  L1频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code1_GPS(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L1C: return 0;
		case CODE_L1P: return 1;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  To_Code2_GPS
*    功能描述:  GPS L2编码标识
*    输入参数:  L2频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code2_GPS(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L2C: return 0;
		case CODE_L2P: return 1;
		case CODE_L2N: return 2;
		case CODE_L2W: return 3;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  To_Code1_GLO
*    功能描述:  GLO L1编码标识
*    输入参数:  L1频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code1_GLO(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L1C: return 0;
		case CODE_L1P: return 1;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  To_Code2_GLO
*    功能描述:  GLO L2编码标识
*    输入参数:  L2频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code2_GLO(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L2C: return 0;
		case CODE_L2P: return 1;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  To_Code1_BDS
*    功能描述:  BDS B1编码标识
*    输入参数:  B1频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code1_BDS(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L2I: return 0;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  To_Code3_BDS
*    功能描述:  BDS B3编码标识
*    输入参数:  B3频率编码
*    输出参数:  无
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U To_Code3_BDS(INT8U cCode)
{
	switch(cCode)
	{
		case CODE_L6I: return 0;
		default:       return 0;
	}
}

/******************************************************************************
*    函数名称:  Encode_Head1001
*    功能描述:  编码1001-1004电文头
*    输入参数:  RTCM结构体变量，电文类型，同步标识，卫星个数
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void Encode_Head1001(T_RTCM *ptRtcm,INT32U iType,INT32U iSync,INT32U iNsat)
{
	INT32U iTow;
	INT32U i=24;

	iTow=ROUND_U(ptRtcm->tTime.dSow/0.001);

	SetBitu(ptRtcm->cBuff,i,   12,iType);          /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid); /*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24,30,iTow);           /*周内秒*/
	SetBitu(ptRtcm->cBuff,i+54, 1,iSync);          /*同步标识*/
	SetBitu(ptRtcm->cBuff,i+55, 5,iNsat);          /*GPS卫星个数*/
	SetBitu(ptRtcm->cBuff,i+60, 1,0);              /*GPS平滑标识*/
	SetBitu(ptRtcm->cBuff,i+61, 3,0);              /*GPS平滑间隔*/

	ptRtcm->iNbit=ptRtcm->iNbit+64;
}

/******************************************************************************
*    函数名称:  GetObsGPS
*    功能描述:  生成RTCM电文中GPS观测值
*    输入参数:  RTCM结构体变量，卫星观测数据，
*    输出参数:  
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void GetObsGPS(T_RTCM *ptRtcm,T_OBSSAT tData,INT32U *piCode1,
					  INT32U *piPr1,INT32S *piCppr1,INT32U *piLock1,
					  INT32U *piAmb,INT32U *piCnr1,INT32U *piCode2,
					  INT32S *piPr21,INT32S *piCppr2,INT32U *piLock2,
					  INT32U *piCnr2)
{
	FP64   dLam1,dLam2;
	INT32S iLt1,iLt2;
	FP64   dCppr,dP=0.0;

	dLam1=dLam_Carr[0];
	dLam2=dLam_Carr[1];
	*piPr1=*piAmb=0;

	/*无效值*/
	if (piCppr1)
	{
		*piCppr1=0xFFF80000;
	}
	if (piPr21)
	{
		*piPr21=0xFFFFE000;
	}
	if (piCppr2)
	{
		*piCppr2=0xFFF80000;
	}

	/*GPS L1 伪距*/
	if (tData.dP[0]!=0.0)
	{
		*piAmb=(int)floor(tData.dP[0]/PRUNIT_GPS);
		*piPr1=ROUND_U((tData.dP[0]-(*piAmb)*PRUNIT_GPS)/0.02);
		dP=(*piPr1)*0.02+(*piAmb)*PRUNIT_GPS;
	}
	/*GPS L1载波-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[0]!=0.0))
	{
		dCppr=fmod(tData.dL[0]-dP/dLam1+1500.0,3000)-1500.0;
		if (piCppr1)
		{
			*piCppr1=ROUND_S(dCppr*dLam1/0.0005);
		}
	}
	/*GPS L2伪距-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dP[1]!=0.0)&&(fabs(tData.dP[1]-dP)<=163.82))
	{
		if (piPr21)
		{
			*piPr21=ROUND_S((tData.dP[1]-dP)/0.02);
		}
	}	
	/*GPS L2载波-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[1]!=0.0))
	{
		dCppr=fmod(tData.dL[1]-dP/dLam2+1500.0,3000)-1500.0;
		if (piCppr2)
		{
			*piCppr2=ROUND_S(dCppr*dLam2/0.0005);
		}
	}

	/*锁定时间*/
	iLt1=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][0],tData.cLLI[0]);
	iLt2=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][1],tData.cLLI[1]);

	if (piCnr1)
	{
		*piCnr1=ROUND_U(tData.dCNR[0]/0.25);
	}
	if (piCnr2)
	{
		*piCnr2=ROUND_U(tData.dCNR[1]/0.25);
	}
	if (piLock1)
	{
		*piLock1=To_Lock(iLt1);
	}
	if (piLock2)
	{
		*piLock2=To_Lock(iLt2);
	}
	if (piCode1)
	{
		*piCode1=To_Code1_GPS(tData.cCode[0]);
	}
	if(piCode2)
	{
		*piCode2=To_Code2_GPS(tData.cCode[1]);
	}
}

/******************************************************************************
*    函数名称:  Encode_Type1001
*    功能描述:  编码1001类型：RTK中GPS L1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1001(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U   iPrn,iCode1,iPr1,iLock1,iAmb;
	INT32S   iCppr1;
	INT32U   iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U   i,j;

	/*统计观测值中GPS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GPS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}


	/*编码电文头*/
	Encode_Head1001(ptRtcm,1001,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码各颗卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];

		/*判断是否是GPS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GPS)
		{
			continue;
		}
		/*生成RTCM电文中GPS观测值*/
		GetObsGPS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,
			      NULL,NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		i=i+58;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1002
*    功能描述:  编码1002类型：RTK中GPS扩展L1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1002(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1;
	INT32S iCppr1;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GPS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GPS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1001(ptRtcm,1002,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码GPS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];

		/*判断是否是GPS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GPS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsGPS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPr1);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58, 8,iAmb);
		SetBitu(ptRtcm->cBuff,i+66, 8,iCnr1);
		i=i+74;
	}

	ptRtcm->iNbit=i;
	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1003
*    功能描述:  编码1003类型：RTK中GPS L1和L2观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1003(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iCode2,iLock2,iAmb;
	INT32S iCppr1,iPr21,iCppr2;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GPS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GPS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1001(ptRtcm,1003,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码GPS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断是否是GPS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GPS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsGPS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,NULL,
			      &iCode2,&iPr21,&iCppr2,&iLock2,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58, 2,iCode2);
		SetBits(ptRtcm->cBuff,i+60,14,iPr21);
		SetBits(ptRtcm->cBuff,i+74,20,iCppr2);
		SetBitu(ptRtcm->cBuff,i+94, 7,iLock2);
		i=i+101;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1004
*    功能描述:  编码1004类型：RTK中GPS扩展的L1和L2观测值
*    输入参数:  RTCM结构变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1004(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1,iCode2,iLock2,iCnr2;
	INT32S iCppr1,iPr21,iCppr2;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GPS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GPS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1001(ptRtcm,1004,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码GPS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断是否是GPS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GPS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsGPS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      &iCode2,&iPr21,&iCppr2,&iLock2,&iCnr2);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,     6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,   1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  24,iPr1);
		SetBits(ptRtcm->cBuff,i+31, 20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51,  7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58,  8,iAmb);
		SetBitu(ptRtcm->cBuff,i+66,  8,iCnr1);
		SetBitu(ptRtcm->cBuff,i+74,  2,iCode2);
		SetBits(ptRtcm->cBuff,i+76, 14,iPr21);
		SetBits(ptRtcm->cBuff,i+90, 20,iCppr2);
		SetBitu(ptRtcm->cBuff,i+110, 7,iLock2);
		SetBitu(ptRtcm->cBuff,i+117, 8,iCnr2);
		i=i+125;
	}
	
	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  SetBits_38
*    功能描述:  生成38位有符号的数据
*    输入参数:  缓存消息，存储位起始位置，生成位的数据
*    输出参数:  缓存消息
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void SetBits_38(INT8U *pcBuff,INT32U iPos,FP64 dValue)
{
	INT32S iDataS;
	INT32U iDataU;

	iDataS=(int)floor(dValue/64.0);
	iDataU=(unsigned int)floor(dValue-iDataS*64.0);
	SetBits(pcBuff,iPos,   32,iDataS);
	SetBitu(pcBuff,iPos+32, 6,iDataU);
}

/******************************************************************************
*    函数名称:  Encode_Type1005
*    功能描述:  编码1005类型：参考站天线参考点坐标，没有天线高
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1005(T_RTCM *ptRtcm)
{
	FP64 *dPos;
	INT32U i=24;

	dPos=ptRtcm->tSta.dAppPos;

	SetBitu(ptRtcm->cBuff,i,   12,1005);           /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid); /*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24, 6,0);              /*ITRF参考时间*/
	SetBitu(ptRtcm->cBuff,i+30, 1,1);              /*GPS标识*/
	SetBitu(ptRtcm->cBuff,i+31, 1,1);              /*GLONASS标识*/
	SetBitu(ptRtcm->cBuff,i+32, 1,0);              /*Galileo标识*/
	SetBitu(ptRtcm->cBuff,i+33, 1,0);              /*测站标识*/
	SetBits_38(ptRtcm->cBuff,i+34,dPos[0]/0.0001); /*X坐标*/
	SetBitu(ptRtcm->cBuff,i+72, 1,1);              /*观测数据同步标识*/
	SetBitu(ptRtcm->cBuff,i+73, 1,0);              /*预留*/
	SetBits_38(ptRtcm->cBuff,i+74,dPos[1]/0.0001); /*Y坐标*/
	SetBitu(ptRtcm->cBuff,i+112,2,0);              /*预留*/
	SetBits_38(ptRtcm->cBuff,i+114,dPos[2]/0.0001);/*Z坐标*/

	i=i+152;
	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1006
*    功能描述:  编码1006类型：参考站天线参考点坐标，包括天线高
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1006(T_RTCM *ptRtcm)
{
	FP64 *dPos;
	INT32U iHgt=0;
	INT32U i=24;

	dPos=ptRtcm->tSta.dAppPos;

	if ((ptRtcm->tSta.dAntDel[0]>=0.0)&&(ptRtcm->tSta.dAntDel[0]<=6.5535))
	{
		iHgt=ROUND_U(ptRtcm->tSta.dAntDel[0]/0.0001);
	}
	else
	{
		iHgt=0;
	}

	SetBitu(ptRtcm->cBuff,i,   12,1006);           /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid); /*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24, 6,0);              /*ITRF参考时间*/
	SetBitu(ptRtcm->cBuff,i+30, 1,1);              /*GPS标识*/
	SetBitu(ptRtcm->cBuff,i+31, 1,1);              /*GLONASS标识*/
	SetBitu(ptRtcm->cBuff,i+32, 1,0);              /*Galileo标识*/
	SetBitu(ptRtcm->cBuff,i+33, 1,0);              /*测站标识*/
	SetBits_38(ptRtcm->cBuff,i+34,dPos[0]/0.0001); /*X坐标*/
	SetBitu(ptRtcm->cBuff,i+72, 1,1);              /*观测数据同步标识*/
	SetBitu(ptRtcm->cBuff,i+73, 1,0);              /*预留*/
	SetBits_38(ptRtcm->cBuff,i+74,dPos[1]/0.0001); /*Y坐标*/
	SetBitu(ptRtcm->cBuff,i+112,2,0);              /*预留*/
	SetBits_38(ptRtcm->cBuff,i+114,dPos[2]/0.0001);/*Z坐标*/
	SetBitu(ptRtcm->cBuff,i+152,16,iHgt);          /*天线高*/

	i=i+168;
	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1007
*    功能描述:  编码1007类型：天线类型描述
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1007(T_RTCM *ptRtcm)
{
	INT32U iN;
	INT32U i=24,j;

	iN=strlen((char *)ptRtcm->tSta.cAnt[1]);
	if (iN>31)
	{
		iN=31;/*天线类型描述最大字节数是31*/
	}

	SetBitu(ptRtcm->cBuff,i,   12,1007);          /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid);/*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24, 8,iN);            /*天线类型描述用的字节*/
	i=i+32;
	for (j=0;j<iN;j++)  /*天线类型描述*/
	{
		SetBitu(ptRtcm->cBuff,i,8,ptRtcm->tSta.cAnt[1][j]);
		i=i+8;
	}
	SetBitu(ptRtcm->cBuff,i,8,ptRtcm->tSta.iAntSetup);/*天线建立ID*/
	i=i+8;

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1008
*    功能描述:  编码1008类型：天线类型描述和序列号
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1008(T_RTCM *ptRtcm)
{
	INT32U iN,iM;
	INT32U i=24,j;

	iN=strlen((char *)ptRtcm->tSta.cAnt[1]);
	if (iN>31)
	{
		iN=31;/*天线类型描述最大字节数是31*/
	}
	iM=strlen((char *)ptRtcm->tSta.cAnt[0]);
	if (iM>31)
	{
		iM=31;/*天线序列号最大字节数是31*/
	}

	SetBitu(ptRtcm->cBuff,i,   12,1008);          /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid);/*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24, 8,iN);            /*天线类型描述用的字节*/
	i=i+32;
	for (j=0;j<iN;j++)  /*天线类型描述*/
	{
		SetBitu(ptRtcm->cBuff,i,8,ptRtcm->tSta.cAnt[1][j]);
		i=i+8;
	}
	SetBitu(ptRtcm->cBuff,i,  8,ptRtcm->tSta.iAntSetup);/*天线建立ID*/
	SetBitu(ptRtcm->cBuff,i+8,8,iM);/*天线序列号用的字节*/
	i=i+16;
	for (j=0;j<iM;j++)  /*天线序列号*/
	{
		SetBitu(ptRtcm->cBuff,i,8,ptRtcm->tSta.cAnt[0][j]);
		i=i+8;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数???  Encode_Head1009
*    功能描述:  编码1009-1012电文的文件头
*    输入参数:  RTCM结构体变量,电文类型，同步标识，卫星个数
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void Encode_Head1009(T_RTCM *ptRtcm,INT32U iType,INT32U iSync,INT32U iNsat)
{
	INT32U iSod;
	T_WEEKSEC tUtc,tGlot;
	INT32U i=24;

	tUtc =TimeAdd(ptRtcm->tTime,-LEAPS);           /*GPS时转换成UTC时*/
	tGlot=TimeAdd(tUtc,10800);                     /*UTC时转换为GLO时*/
	iSod=ROUND_U(fmod(tGlot.dSow,D2S)/0.001);      /*天内秒*/
    
    SetBitu(ptRtcm->cBuff,i,   12,iType);          /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid); /*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24,27,iSod);           /*天内秒*/
	SetBitu(ptRtcm->cBuff,i+51, 1,iSync);          /*同步标识*/
	SetBitu(ptRtcm->cBuff,i+52, 5,iNsat);          /*GLONASS卫星个数*/
	SetBitu(ptRtcm->cBuff,i+57, 1,0);              /*GLONASS平滑标识*/
	SetBitu(ptRtcm->cBuff,i+58, 3,0);              /*GLONASS平滑间隔*/

	ptRtcm->iNbit=i+61;
}

/******************************************************************************
*    函数名称:  GetObsGLO
*    功能描述:  生成RTCM电文中GLO观测值
*    输入参数:  RTCM结构体变量，卫星观测数据，
*    输出参数:  
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void GetObsGLO(T_RTCM *ptRtcm,T_OBSSAT tData,INT32U *piCode1,
					  INT32S iNfreq,INT32U *piPr1,INT32S *piCppr1,
					  INT32U *piLock1,INT32U *piAmb,INT32U *piCnr1,
					  INT32U *piCode2,INT32S *piPr21,INT32S *piCppr2,
					  INT32U *piLock2,INT32U *piCnr2)
{
	FP64   dLam1=0.0,dLam2=0.0;
	INT32S iLt1,iLt2;
	FP64   dCppr,dP=0.0;

    if (iNfreq>=0)
	{
		dLam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(iNfreq-7));
		dLam2=CLIGHT/(FREQ2_GLO+DFRQ2_GLO*(iNfreq-7));
	}
	*piPr1=*piAmb=0;

	/*无效值*/
	if (piCppr1)
	{
		*piCppr1=0xFFF80000;
	}
	if (piPr21)
	{
		*piPr21=0xFFFFE000;
	}
	if (piCppr2)
	{
		*piCppr2=0xFFF80000;
	}

	/*GLO L1 伪距*/
	if (tData.dP[0]!=0.0)
	{
		*piAmb=(int)floor(tData.dP[0]/PRUNIT_GLO);
		*piPr1=ROUND_U((tData.dP[0]-(*piAmb)*PRUNIT_GLO)/0.02);
		dP=(*piPr1)*0.02+(*piAmb)*PRUNIT_GLO;
	}
	/*GLO L1载波-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[0]!=0.0)&&(dLam1>0.0))
	{
		dCppr=fmod(tData.dL[0]-dP/dLam1+1500.0,3000)-1500.0;
		if (piCppr1)
		{
			*piCppr1=ROUND_S(dCppr*dLam1/0.0005);
		}
	}
	/*GLO L2伪距-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dP[1]!=0.0)&&(fabs(tData.dP[1]-dP)<=163.82))
	{
		if (piPr21)
		{
			*piPr21=ROUND_S((tData.dP[1]-dP)/0.02);
		}
	}	
	/*GLO L2载波-L1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[1]!=0.0)&&(dLam2>0.0))
	{
		dCppr=fmod(tData.dL[1]-dP/dLam2+1500.0,3000)-1500.0;
		if (piCppr2)
		{
			*piCppr2=ROUND_S(dCppr*dLam2/0.0005);
		}
	}

	/*锁定时间*/
	iLt1=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][0],tData.cLLI[0]);
	iLt2=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][1],tData.cLLI[1]);

	if (piCnr1)
	{
		*piCnr1=ROUND_U(tData.dCNR[0]/0.25);
	}
	if (piCnr2)
	{
		*piCnr2=ROUND_U(tData.dCNR[1]/0.25);
	}
	if (piLock1)
	{
		*piLock1=To_Lock(iLt1);
	}
	if (piLock2)
	{
		*piLock2=To_Lock(iLt2);
	}
	if (piCode1)
	{
		*piCode1=To_Code1_GLO(tData.cCode[0]);
	}
	if(piCode2)
	{
		*piCode2=To_Code2_GLO(tData.cCode[1]);
	}
}

///*RTCM中GLONASS卫星的频率通道号(0-13,-1:error) -------------------------------*/
//static INT32S NFrq_GLO(INT32U iSat,INT32U iPrn,T_RTCM *ptRtcm)
//{
//	if (ptRtcm->tNav.ptGeph[iPrn-1].iSat==iSat)
//	{
//		return ptRtcm->tNav.ptGeph[iPrn-1].iFrq+7;
//	}
//	else
//	{
//		return -1;
//	}
//}

/******************************************************************************
*    函数名称:  Encode_Type1009
*    功能描述:  编码1009类型：RTK中GLONASS L1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1009(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U   iPrn,iCode1,iPr1,iLock1,iAmb;
	INT32S   iNfreq,iCppr1;
	INT32U   iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U   i=24,j;

	/*统计GLO卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GLO)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}
	
	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1009(ptRtcm,1009,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码各颗卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];

		/*判断卫星是否是GLONASS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GLO)
		{
			continue;
		}
        /*卫星频率*/
		iNfreq=iFrqChn[iPrn-1]+7;//NFrq_GLO(tData.iSat,iPrn,ptRtcm);
		/*RTCM电文观测值*/
		GetObsGLO(ptRtcm,tData,&iCode1,iNfreq,&iPr1,&iCppr1,&iLock1,&iAmb,
			      NULL,NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		if (iNfreq<0)
		{
			iNfreq=0;
		}
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  5,iNfreq);
		SetBitu(ptRtcm->cBuff,i+12,25,iPr1);
		SetBits(ptRtcm->cBuff,i+37,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+57, 7,iLock1);
		i=i+64;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1010
*    功能描述:  编码1010类型：RTK中GLONASS扩展的L1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1010(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1;
	INT32S iCppr1,iNfreq;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GLONASS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GLO)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1009(ptRtcm,1010,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码各颗卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断是否是GLONASS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GLO)
		{
			continue;
		}

		/*卫星频率*/
		iNfreq=iFrqChn[iPrn-1]+7;//NFrq_GLO(tData.iSat,iPrn,ptRtcm);
		/*RTCM电文观测值*/
		GetObsGLO(ptRtcm,tData,&iCode1,iNfreq,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		if (iNfreq<0)
		{
			iNfreq=0;
		}
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  5,iNfreq);
		SetBitu(ptRtcm->cBuff,i+12,25,iPr1);
		SetBits(ptRtcm->cBuff,i+37,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+57, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+64, 7,iAmb);
		SetBitu(ptRtcm->cBuff,i+71, 8,iCnr1);
		
		i=i+79;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1011
*    功能描述:  编码1011类型：RTK中GLONASS L1和L2观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1011(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iCode2,iLock2,iAmb;
	INT32S iNfreq,iCppr1,iPr21,iCppr2;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GLO卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GLO)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1009(ptRtcm,1011,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码GLO卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断卫星是否是GLO卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GLO)
		{
			continue;
		}

		/*卫星频率*/
		iNfreq=iFrqChn[iPrn-1]+7;//NFrq_GLO(tData.iSat,iPrn,ptRtcm);
		/*RTCM电文观测值*/
		GetObsGLO(ptRtcm,tData,&iCode1,iNfreq,&iPr1,&iCppr1,&iLock1,&iAmb,NULL,
			      &iCode2,&iPr21,&iCppr2,&iLock2,NULL);

		/*编码观测值*/
		if (iNfreq<0)
		{
			iNfreq=0;
		}
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  5,iNfreq);
		SetBitu(ptRtcm->cBuff,i+12,25,iPr1);
		SetBits(ptRtcm->cBuff,i+37,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+57, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+64, 2,iCode2);
		SetBits(ptRtcm->cBuff,i+66,14,iPr21);
		SetBits(ptRtcm->cBuff,i+80,20,iCppr2);
		SetBitu(ptRtcm->cBuff,i+100,7,iLock2);
		i=i+107; 
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1012
*    功能描述:  编码1012类型：RTK中GLONASS扩展的L1和L2观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1012(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1,iCode2,iLock2,iCnr2;
	INT32S iNfreq,iCppr1,iPr21,iCppr2;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计GLO卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_GLO)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head1009(ptRtcm,1012,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码GLO卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断卫星是否是GLO卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_GLO)
		{
			continue;
		}

		/*卫星频率*/
		iNfreq=iFrqChn[iPrn-1]+7;//NFrq_GLO(tData.iSat,iPrn,ptRtcm);
		/*iNfreq=5;*/
		/*RTCM电文观测值*/
		GetObsGLO(ptRtcm,tData,&iCode1,iNfreq,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      &iCode2,&iPr21,&iCppr2,&iLock2,&iCnr2);
				
		/*编码观测值*/
		if (iNfreq<0)
		{
			iNfreq=0;
		}
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  5,iNfreq);
		SetBitu(ptRtcm->cBuff,i+12,25,iPr1);
		SetBits(ptRtcm->cBuff,i+37,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+57, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+64, 7,iAmb);
		SetBitu(ptRtcm->cBuff,i+71, 8,iCnr1);
		SetBitu(ptRtcm->cBuff,i+79, 2,iCode2);
		SetBits(ptRtcm->cBuff,i+81,14,iPr21);
		SetBits(ptRtcm->cBuff,i+95,20,iCppr2);
		SetBitu(ptRtcm->cBuff,i+115,7,iLock2);
		SetBitu(ptRtcm->cBuff,i+122,8,iCnr2);
		i=i+130;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Head4087
*    功能描述:  编码4087、4088、4091、4092电文头
*    输入参数:  RTCM结构体变量，电文类型，同步标识，卫星个数
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void Encode_Head4087(T_RTCM *ptRtcm,INT32U iType,INT32U iSync,INT32U iNsat)
{
	INT32U iTow;
	INT32U i=24;

	iTow=ROUND_U(ptRtcm->tTime.dSow/0.001);

	SetBitu(ptRtcm->cBuff,i,   12,iType);          /*电文消息类型*/
	SetBitu(ptRtcm->cBuff,i+12,12,ptRtcm->iStaid); /*测站ID*/
	SetBitu(ptRtcm->cBuff,i+24,30,iTow);           /*周内秒*/
	SetBitu(ptRtcm->cBuff,i+54, 1,iSync);          /*同步标识*/
	SetBitu(ptRtcm->cBuff,i+55, 5,iNsat);          /*BDS卫星个数*/
	SetBitu(ptRtcm->cBuff,i+60, 1,0);              /*BDS平滑标识*/
	SetBitu(ptRtcm->cBuff,i+61, 3,0);              /*BDS平滑间隔*/

	ptRtcm->iNbit=ptRtcm->iNbit+64;
}

/******************************************************************************
*    函数名称:  GetObsBDS
*    功能描述:  生成RTCM电文中BDS观测值
*    输入参数:  RTCM结构体变量，卫星观测数据，
*    输出参数:  
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void GetObsBDS(T_RTCM *ptRtcm,T_OBSSAT tData,INT32U *piCode1,
					  INT32U *piPr1,INT32S *piCppr1,INT32U *piLock1,
					  INT32U *piAmb,INT32U *piCnr1,INT32U *piCode3,
					  INT32S *piPr31,INT32S *piCppr3,INT32U *piLock3,
					  INT32U *piCnr3)
{
	FP64   dLam1,dLam3;
	INT32S iLt1,iLt3;
	FP64   dCppr,dP=0.0;

	dLam1=dLam_Carr[3];
	dLam3=dLam_Carr[5];
	*piPr1=*piAmb=0;

	/*无效值*/
	if (piCppr1)
	{
		*piCppr1=0xFFF80000;
	}
	if (piPr31)
	{
		*piPr31=0xFFFFE000;
	}
	if (piCppr3)
	{
		*piCppr3=0xFFF80000;
	}

	/*BDS B1 伪距*/
	if (tData.dP[0]!=0.0)
	{
		*piAmb=(int)floor(tData.dP[0]/PRUNIT_BDS);
		*piPr1=ROUND_U((tData.dP[0]-(*piAmb)*PRUNIT_BDS)/0.02);
		dP=(*piPr1)*0.02+(*piAmb)*PRUNIT_BDS;
	}
	/*BDS B1载波-B1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[0]!=0.0))
	{
		dCppr=fmod(tData.dL[0]-dP/dLam1+1500.0,3000)-1500.0;
		if (piCppr1)
		{
			*piCppr1=ROUND_S(dCppr*dLam1/0.0005);
		}
	}
	/*BDS B3伪距-B1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dP[2]!=0.0)&&(fabs(tData.dP[2]-dP)<=163.82))
	{
		if (piPr31)
		{
			*piPr31=ROUND_S((tData.dP[2]-dP)/0.02);
		}
	}	
	/*BDS B3载波-B1伪距*/
	if ((tData.dP[0]!=0.0)&&(tData.dL[2]!=0.0))
	{
		dCppr=fmod(tData.dL[2]-dP/dLam3+1500.0,3000)-1500.0;
		if (piCppr3)
		{
			*piCppr3=ROUND_S(dCppr*dLam3/0.0005);
		}
	}

	/*锁定时间*/
	iLt1=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][0],tData.cLLI[0]);
	iLt3=LockTime(tData.tObsTime,&ptRtcm->tLltime[tData.iSat-1][2],tData.cLLI[2]);

	if (piCnr1)
	{
		*piCnr1=ROUND_U(tData.dCNR[0]/0.25);
	}
	if (piCnr3)
	{
		*piCnr3=ROUND_U(tData.dCNR[2]/0.25);
	}
	if (piLock1)
	{
		*piLock1=To_Lock(iLt1);
	}
	if (piLock3)
	{
		*piLock3=To_Lock(iLt3);
	}
	if (piCode1)
	{
		*piCode1=To_Code1_BDS(tData.cCode[0]);
	}
	if(piCode3)
	{
		*piCode3=To_Code3_BDS(tData.cCode[2]);
	}
}

/******************************************************************************
*    函数名称:  Encode_Type4087
*    功能描述:  编码4087类型：RTK中BDS B1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type4087(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U   iPrn,iCode1,iPr1,iLock1,iAmb;
	INT32S   iCppr1;
	INT32U   iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U   i,j;

	/*统计观测值中BDS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_BDS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head4087(ptRtcm,4087,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码各颗卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];

		/*判断是否是BDS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_BDS)
		{
			continue;
		}
		/*生成RTCM电文中BDS观测值*/
		GetObsBDS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,
			      NULL,NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		i=i+58;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type4088
*    功能描述:  编码4088类型：RTK中BDS扩展B1观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type4088(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1;
	INT32S iCppr1;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计BDS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_BDS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head4087(ptRtcm,4088,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码BDS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];

		/*判断是否是BDS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_BDS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsBDS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      NULL,NULL,NULL,NULL,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPr1);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58, 8,iAmb);
		SetBitu(ptRtcm->cBuff,i+66, 8,iCnr1);
		i=i+74;
	}

	ptRtcm->iNbit=i;
	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type4091
*    功能描述:  编码4091类型：RTK中BDS B1和B3观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type4091(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iCode3,iLock3,iAmb;
	INT32S iCppr1,iPr31,iCppr3;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计BDS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_BDS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head4087(ptRtcm,4091,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码BDS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断是否是BDS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_BDS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsBDS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,NULL,
			      &iCode3,&iPr31,&iCppr3,&iLock3,NULL);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,    6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,  1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7, 24,iPr1);
		SetBits(ptRtcm->cBuff,i+31,20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51, 7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58, 2,iCode3);
		SetBits(ptRtcm->cBuff,i+60,14,iPr31);
		SetBits(ptRtcm->cBuff,i+74,20,iCppr3);
		SetBitu(ptRtcm->cBuff,i+94, 7,iLock3);
		i=i+101;
	}

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type4092
*    功能描述:  编码4092类型：RTK中BDS扩展的B1和B3观测值
*    输入参数:  RTCM结构体变量，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type4092(T_RTCM *ptRtcm,INT32U iSync)
{
	INT32U iPrn,iCode1,iPr1,iLock1,iAmb,iCnr1,iCode3,iLock3,iCnr3;
	INT32S iCppr1,iPr31,iCppr3;
	INT32U iSys,iNsat=0;
	T_OBSSAT tData;
	INT32U i,j;

	/*统计BDS卫星个数*/
	for (j=0;(j<ptRtcm->tObs.iN)&&(iNsat<MAXSAT);j++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[j].iSat,&iPrn);
		if (iSys==SYS_BDS)
		{
			iNsat=iNsat+1;
		}
		else
		{
			continue;
		}
	}

	/*卫星个数是0时，不进行编码*/
	if (iNsat==0)
	{
		return 0;
	}

	/*编码电文头*/
	Encode_Head4087(ptRtcm,4092,iSync,iNsat);
	i=ptRtcm->iNbit;

	/*编码BDS卫星观测值*/
	for (j=0;j<ptRtcm->tObs.iN;j++)
	{
		tData=ptRtcm->tObs.ptData[j];
		/*判断是否是BDS卫星*/
		iSys=Sat2Sys(tData.iSat,&iPrn);
		if (iSys!=SYS_BDS)
		{
			continue;
		}

		/*生成RTCM电文观测值*/
		GetObsBDS(ptRtcm,tData,&iCode1,&iPr1,&iCppr1,&iLock1,&iAmb,&iCnr1,
			      &iCode3,&iPr31,&iCppr3,&iLock3,&iCnr3);

		/*编码观测值*/
		SetBitu(ptRtcm->cBuff,i,     6,iPrn);
		SetBitu(ptRtcm->cBuff,i+6,   1,iCode1);
		SetBitu(ptRtcm->cBuff,i+7,  24,iPr1);
		SetBits(ptRtcm->cBuff,i+31, 20,iCppr1);
		SetBitu(ptRtcm->cBuff,i+51,  7,iLock1);
		SetBitu(ptRtcm->cBuff,i+58,  8,iAmb);
		SetBitu(ptRtcm->cBuff,i+66,  8,iCnr1);
		SetBitu(ptRtcm->cBuff,i+74,  2,iCode3);
		SetBits(ptRtcm->cBuff,i+76, 14,iPr31);
		SetBits(ptRtcm->cBuff,i+90, 20,iCppr3);
		SetBitu(ptRtcm->cBuff,i+110, 7,iLock3);
		SetBitu(ptRtcm->cBuff,i+117, 8,iCnr3);
		i=i+125;
	}
	
	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type1019
*    功能描述:  编码1019类型：GPS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1019(T_RTCM *ptRtcm)
{
	INT32U iPrn,iWeek,iToc,iE,iSqrtA,iToes,iFit;
	INT32S iIdot,iF2,iF1,iF0,iCrs,iDeln,iM0,iCuc,iCus,iCic;
	INT32S iOMG0,iCis,iI0,iCrc,iOmg,iOMGd,iTgd;
	T_EPH  tEph;
	INT32U i=24;
	
	/*判断要输出的是否是GPS卫星*/
	if (Sat2Sys(ptRtcm->iEphsat,&iPrn)!=SYS_GPS)
	{
		return 0;
	}
	tEph=ptRtcm->tNav.ptEph[ptRtcm->iEphsat-1];
	/*卫星星历一致性*/
	if (tEph.iSat!=ptRtcm->iEphsat)
	{
		return 0;
	}

	iWeek =tEph.iWeek%1024;
	iIdot =ROUND_S(tEph.dIdot/P2_43/SC2RAD);
	iToc  =ROUND_U(tEph.tToc.dSow/16.0);
	iF2   =ROUND_S(tEph.dF2/P2_55);
	iF1   =ROUND_S(tEph.dF1/P2_43);
	iF0   =ROUND_S(tEph.dF0/P2_31);
	iCrs  =ROUND_S(tEph.dCrs/P2_5);
	iDeln =ROUND_S(tEph.dDeln/P2_43/SC2RAD);
	iM0   =ROUND_S(tEph.dM0/P2_31/SC2RAD);
	iCuc  =ROUND_S(tEph.dCuc/P2_29);
	iE    =ROUND_U(tEph.dE/P2_33);
	iCus  =ROUND_S(tEph.dCus/P2_29);
	iSqrtA=ROUND_U(tEph.dSqrtA/P2_19);
	iToes =ROUND_U(tEph.dToes/16.0);
	iCic  =ROUND_S(tEph.dCic/P2_29);
	iOMG0 =ROUND_S(tEph.dOMG0/P2_31/SC2RAD);
	iCis  =ROUND_S(tEph.dCis/P2_29);
	iI0   =ROUND_S(tEph.dI0/P2_31/SC2RAD);
	iCrc  =ROUND_S(tEph.dCrc/P2_5);
	iOmg  =ROUND_S(tEph.dOmg/P2_31/SC2RAD);
	iOMGd =ROUND_S(tEph.dOMGd/P2_43/SC2RAD);
	iTgd  =ROUND_S(tEph.dTgd[0]/P2_31);
	if (tEph.dFit==4.0)
	{
		iFit=0;   
	}
	else
	{
		iFit=1;
	}

	SetBitu(ptRtcm->cBuff,i,12,1019);       i=i+12;
	SetBitu(ptRtcm->cBuff,i, 6,iPrn);       i=i+ 6;
	SetBitu(ptRtcm->cBuff,i,10,iWeek);      i=i+10;
	SetBitu(ptRtcm->cBuff,i, 4,tEph.iSva);  i=i+ 4;
	SetBitu(ptRtcm->cBuff,i, 2,tEph.iCode); i=i+ 2;
	SetBits(ptRtcm->cBuff,i,14,iIdot);      i=i+14;
	SetBitu(ptRtcm->cBuff,i, 8,tEph.iIode); i=i+ 8;
	SetBitu(ptRtcm->cBuff,i,16,iToc);       i=i+16;
	SetBits(ptRtcm->cBuff,i, 8,iF2);        i=i+ 8;
	SetBits(ptRtcm->cBuff,i,16,iF1);        i=i+16;
	SetBits(ptRtcm->cBuff,i,22,iF0);        i=i+22;
	SetBitu(ptRtcm->cBuff,i,10,tEph.iIodc); i=i+10;
	SetBits(ptRtcm->cBuff,i,16,iCrs);       i=i+16;
	SetBits(ptRtcm->cBuff,i,16,iDeln);      i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iM0);        i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCuc);       i=i+16;
	SetBitu(ptRtcm->cBuff,i,32,iE);         i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCus);       i=i+16;
	SetBitu(ptRtcm->cBuff,i,32,iSqrtA);     i=i+32;
	SetBitu(ptRtcm->cBuff,i,16,iToes);      i=i+16;
	SetBits(ptRtcm->cBuff,i,16,iCic);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iOMG0);      i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCis);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iI0);        i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCrc);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iOmg);       i=i+32;
	SetBits(ptRtcm->cBuff,i,24,iOMGd);      i=i+24;
	SetBits(ptRtcm->cBuff,i, 8,iTgd);       i=i+ 8;
	SetBitu(ptRtcm->cBuff,i, 6,tEph.iSvh);  i=i+ 6;
	SetBitu(ptRtcm->cBuff,i, 1,tEph.iFlag); i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 1,iFit);       i=i+ 1;

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  SetBitg
*    功能描述:  编码位 有符号的数据
*    输入参数:  缓存消息，存储位起始位置，生成位长度，生成位的数据
*    输出参数:  缓存消息
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void SetBitg(INT8U *pcBuff,INT32U iPos,INT32U iLen,FP64 dValue)
{
	INT32U iSign;

	if (dValue<0)
	{
		iSign=1;
	}
	else
	{
		iSign=0;
	}

	SetBitu(pcBuff,iPos,1,iSign);
	SetBitu(pcBuff,iPos+1,iLen-1,(unsigned int)fabs(dValue));
}

/******************************************************************************
*    函数名称:  Encode_Type1020
*    功能描述:  编码1020类型：GLONASS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type1020(T_RTCM *ptRtcm)
{
	INT32U iPrn,iTk_h,iTk_m,iTk_s,iTb;
	INT32S iPos[3],iVel[3],iAcc[3],iGamn,iTaun;
	T_GLOEPH tGeph;
	T_WEEKSEC tUtcf,tGlof,tUtcc,tGloc;
	T_YMD tYmd;
	INT32U i=24,j;

	/*判断要输出的是否是GLO卫星*/
	if (Sat2Sys(ptRtcm->iEphsat,&iPrn)!=SYS_GLO)
	{
		return 0;
	}
	tGeph=ptRtcm->tNav.ptGeph[iPrn-1];
	/*卫星星历一致性*/
	if (tGeph.iSat!=ptRtcm->iEphsat)
	{
		return 0;
	}

	/*电文帧时间 GLO导航电文中是UTC时，程序中是GPST，RTCM中是GLO时*/
	tUtcf=TimeAdd(tGeph.tTof,-LEAPS); /*GPS时转换成UTC*/
	tGlof=TimeAdd(tUtcf,10800); /*UTC转换成GLO时 UTC+3h*/
	tYmd=WeekSec2YMD(tGlof);
	iTk_h=tYmd.iHor;
	iTk_m=tYmd.iMin;
	iTk_s=ROUND_U(tYmd.dSec/30.0);

	/*导航电文历元时间 GLO导航电文中是UTC时，程序中是GPST，RTCM中是GLO时*/
	tUtcc=TimeAdd(tGeph.tToc,-LEAPS); /*GPS时转换成UTC*/
	tGloc=TimeAdd(tUtcc,10800); /*UTC转换成GLO时 UTC+3h*/
	iTb=ROUND_U(fmod(tGloc.dSow,D2S)/900.0);

	for (j=0;j<3;j++)
	{
		iVel[j]=ROUND_S(tGeph.dVel[j]/P2_20);
		iPos[j]=ROUND_S(tGeph.dPos[j]/P2_11);
		iAcc[j]=ROUND_S(tGeph.dAcc[j]/P2_30);
	}
	iGamn=ROUND_S(tGeph.dGamn/P2_40);
	iTaun=ROUND_S(tGeph.dTaun/P2_30);

	SetBitu(ptRtcm->cBuff,i,12,1020);         i=i+12;
	SetBitu(ptRtcm->cBuff,i, 6,iPrn);         i=i+ 6;
	SetBitu(ptRtcm->cBuff,i, 5,tGeph.iFrq+7); i=i+ 5;
	SetBitu(ptRtcm->cBuff,i, 4,0);            i=i+ 4;
	SetBitu(ptRtcm->cBuff,i, 5,iTk_h);        i=i+ 5;
	SetBitu(ptRtcm->cBuff,i, 6,iTk_m);        i=i+ 6;
	SetBitu(ptRtcm->cBuff,i, 1,iTk_s);        i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 1,tGeph.iSvh);   i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 1,0);            i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 7,iTb);          i=i+ 7;
	SetBitg(ptRtcm->cBuff,i,24,iVel[0]);      i=i+24;
	SetBitg(ptRtcm->cBuff,i,27,iPos[0]);      i=i+27;
	SetBitg(ptRtcm->cBuff,i, 5,iAcc[0]);      i=i+ 5;
	SetBitg(ptRtcm->cBuff,i,24,iVel[1]);      i=i+24;
	SetBitg(ptRtcm->cBuff,i,27,iPos[1]);      i=i+27;
	SetBitg(ptRtcm->cBuff,i, 5,iAcc[1]);      i=i+ 5;
	SetBitg(ptRtcm->cBuff,i,24,iVel[2]);      i=i+24;
	SetBitg(ptRtcm->cBuff,i,27,iPos[2]);      i=i+27;
	SetBitg(ptRtcm->cBuff,i, 5,iAcc[2]);      i=i+ 5;
	SetBitu(ptRtcm->cBuff,i, 1,0);            i=i+ 1;
	SetBitg(ptRtcm->cBuff,i,11,iGamn);        i=i+11;
	SetBitu(ptRtcm->cBuff,i, 3,0);            i=i+ 3;
	SetBitg(ptRtcm->cBuff,i,22,iTaun);        i=i+22;
	SetBitu(ptRtcm->cBuff,i, 5,0);            i=i+ 5;
	SetBitu(ptRtcm->cBuff,i, 5,0);            i=i+ 5;
	SetBitu(ptRtcm->cBuff,i, 1,0);            i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 4,0);            i=i+ 4;
	SetBitu(ptRtcm->cBuff,i,11,0);            i=i+11;
	SetBitu(ptRtcm->cBuff,i, 2,0);            i=i+ 2;
	SetBitu(ptRtcm->cBuff,i, 1,0);            i=i+ 1;
	SetBitu(ptRtcm->cBuff,i,11,0);            i=i+11;
	SetBitu(ptRtcm->cBuff,i,32,0);            i=i+32;
	SetBitu(ptRtcm->cBuff,i, 5,0);            i=i+ 5;
	SetBitu(ptRtcm->cBuff,i,22,0);            i=i+22;
	SetBitu(ptRtcm->cBuff,i, 1,0);            i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 7,0);            i=i+ 7;

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_Type4095
*    功能描述:  编码4095类型：BDS广播星历
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Encode_Type4095(T_RTCM *ptRtcm)
{
	INT32U iPrn,iWeek,iToc,iE,iSqrtA,iToes;
	INT32S iIdot,iF2,iF1,iF0,iCrs,iDeln,iM0,iCuc,iCus,iCic;
	INT32S iOMG0,iCis,iI0,iCrc,iOmg,iOMGd,iTgd;
	T_EPH  tEph;
	INT32U i=24;
	
	/*判断要输出的是否是BDS卫星*/
	if (Sat2Sys(ptRtcm->iEphsat,&iPrn)!=SYS_BDS)
	{
		return 0;
	}
	tEph=ptRtcm->tNav.ptEph[ptRtcm->iEphsat-1];
	/*卫星星历一致性*/
	if (tEph.iSat!=ptRtcm->iEphsat)
	{
		return 0;
	}

	iWeek =tEph.iWeek-1356;
	iIdot =ROUND_S(tEph.dIdot/P2_43/SC2RAD);
	iToc  =ROUND_U(tEph.tToc.dSow/16.0);
	iF2   =ROUND_S(tEph.dF2/P2_55);
	iF1   =ROUND_S(tEph.dF1/P2_43);
	iF0   =ROUND_S(tEph.dF0/P2_31);
	iCrs  =ROUND_S(tEph.dCrs/P2_5);
	iDeln =ROUND_S(tEph.dDeln/P2_43/SC2RAD);
	iM0   =ROUND_S(tEph.dM0/P2_31/SC2RAD);
	iCuc  =ROUND_S(tEph.dCuc/P2_29);
	iE    =ROUND_U(tEph.dE/P2_33);
	iCus  =ROUND_S(tEph.dCus/P2_29);
	iSqrtA=ROUND_U(tEph.dSqrtA/P2_19);
	iToes =ROUND_U(tEph.dToes/16.0);
	iCic  =ROUND_S(tEph.dCic/P2_29);
	iOMG0 =ROUND_S(tEph.dOMG0/P2_31/SC2RAD);
	iCis  =ROUND_S(tEph.dCis/P2_29);
	iI0   =ROUND_S(tEph.dI0/P2_31/SC2RAD);
	iCrc  =ROUND_S(tEph.dCrc/P2_5);
	iOmg  =ROUND_S(tEph.dOmg/P2_31/SC2RAD);
	iOMGd =ROUND_S(tEph.dOMGd/P2_43/SC2RAD);
	iTgd  =ROUND_S(tEph.dTgd[0]/P2_31);
	//if (tEph.dFit=4.0)
	//{
	//	iFit=0;   
	//}
	//else
	//{
	//	iFit=1;
	//}

	SetBitu(ptRtcm->cBuff,i,12,4095);       i=i+12;
	SetBitu(ptRtcm->cBuff,i, 6,iPrn);       i=i+ 6;
	SetBitu(ptRtcm->cBuff,i,10,iWeek);      i=i+10;
	SetBitu(ptRtcm->cBuff,i, 4,tEph.iSva);  i=i+ 4;
	//SetBitu(ptRtcm->cBuff,i, 2,tEph.iCode); i=i+ 2;
	SetBits(ptRtcm->cBuff,i,14,iIdot);      i=i+14;
	SetBitu(ptRtcm->cBuff,i, 8,tEph.iIode); i=i+ 8;
	SetBitu(ptRtcm->cBuff,i,16,iToc);       i=i+16;
	SetBits(ptRtcm->cBuff,i, 8,iF2);        i=i+ 8;
	SetBits(ptRtcm->cBuff,i,16,iF1);        i=i+16;
	SetBits(ptRtcm->cBuff,i,22,iF0);        i=i+22;
	SetBitu(ptRtcm->cBuff,i,10,tEph.iIodc); i=i+10;
	SetBits(ptRtcm->cBuff,i,16,iCrs);       i=i+16;
	SetBits(ptRtcm->cBuff,i,16,iDeln);      i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iM0);        i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCuc);       i=i+16;
	SetBitu(ptRtcm->cBuff,i,32,iE);         i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCus);       i=i+16;
	SetBitu(ptRtcm->cBuff,i,32,iSqrtA);     i=i+32;
	SetBitu(ptRtcm->cBuff,i,16,iToes);      i=i+16;
	SetBits(ptRtcm->cBuff,i,16,iCic);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iOMG0);      i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCis);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iI0);        i=i+32;
	SetBits(ptRtcm->cBuff,i,16,iCrc);       i=i+16;
	SetBits(ptRtcm->cBuff,i,32,iOmg);       i=i+32;
	SetBits(ptRtcm->cBuff,i,24,iOMGd);      i=i+24;
	SetBits(ptRtcm->cBuff,i, 8,iTgd);       i=i+ 8;
	SetBitu(ptRtcm->cBuff,i, 6,tEph.iSvh);  i=i+ 6;
	/*SetBitu(ptRtcm->cBuff,i, 1,tEph.iFlag); i=i+ 1;
	SetBitu(ptRtcm->cBuff,i, 1,iFit);       i=i+ 1;*/

	ptRtcm->iNbit=i;

	return 1;
}

/******************************************************************************
*    函数名称:  Encode_RTCM3
*    功能描述:  编码RTCM3电文
*    输入参数:  RTCM结构体变量,电文类型，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
INT32U Encode_RTCM3(T_RTCM *ptRtcm,INT32U iType,INT32U iSync)
{
	INT32U iMark=0;

	switch (iType)
	{
		case 1001: iMark=Encode_Type1001(ptRtcm,iSync);
				   break;
		case 1002: iMark=Encode_Type1002(ptRtcm,iSync);
				   break;
		case 1003: iMark=Encode_Type1003(ptRtcm,iSync);
				   break;
		case 1004: iMark=Encode_Type1004(ptRtcm,iSync);
				   break;
		case 1005: iMark=Encode_Type1005(ptRtcm);
				   break;
		case 1006: iMark=Encode_Type1006(ptRtcm);
				   break;
		case 1007: iMark=Encode_Type1007(ptRtcm);
				   break;
		case 1008: iMark=Encode_Type1008(ptRtcm);
				   break;
		case 1009: iMark=Encode_Type1009(ptRtcm,iSync);
				   break;
		case 1010: iMark=Encode_Type1010(ptRtcm,iSync);
				   break;
		case 1011: iMark=Encode_Type1011(ptRtcm,iSync);
				   break;
		case 1012: iMark=Encode_Type1012(ptRtcm,iSync);
				   break;
		case 4087: iMark=Encode_Type4087(ptRtcm,iSync);
				   break;
		case 4088: iMark=Encode_Type4088(ptRtcm,iSync);
				   break;
		case 4091: iMark=Encode_Type4091(ptRtcm,iSync);
				   break;
		case 4092: iMark=Encode_Type4092(ptRtcm,iSync);
			       break;
		case 1019: iMark=Encode_Type1019(ptRtcm);
				   break;
		case 1020: iMark=Encode_Type1020(ptRtcm);
				   break;
		case 4095: iMark=Encode_Type4095(ptRtcm);
				   break;
		default:   break;
	}

	if (iMark>0)
	{
		iType=iType-1000;
		if (iType>1000)  /*RTCM中BDS电文*/
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



