/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：func_Convrnx.cpp
* 内容摘要：观测值类型设置
*           RTCM 3.1电文解码输出到RINEX文件
*           RTCM 3.1电文编码
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
static INT8U cType_str[]="CLDS";  /*观测值类型*/

/******************************************************************************
*    函数名称:  ConvCode322
*    功能描述:  RINEX观测值类型转换 ver.3 -> ver.2
*    输入参数:  RINEX版本号，卫星系统，观测值类型
*    输出参数:  观测值类型
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void ConvCode322(FP64 dVer,INT32U iSys,INT8U *pcObsType)
{
	if ((dVer>=2.12)&&(iSys==SYS_GPS||iSys==SYS_BDS)&&!strcmp((char *)(pcObsType+1),"1C")) /*L1C/A*/ 
	{
		strcpy((char *)(pcObsType+1),"A");
	}
	else if ((dVer>=2.12)&&(iSys==SYS_GPS||iSys==SYS_BDS)&&(!strcmp((char *)(pcObsType+1),"1S")||
		!strcmp((char *)(pcObsType+1),"1L")||!strcmp((char *)(pcObsType+1),"1X"))) /*L1C*/
	{
		strcpy((char *)(pcObsType+1),"B");
	}
	else if ((dVer>=2.12)&&(iSys==SYS_GPS||iSys==SYS_BDS)&&(!strcmp((char *)(pcObsType+1),"2S")||
		!strcmp((char *)(pcObsType+1),"2L")||!strcmp((char *)(pcObsType+1),"2X"))) /*L2C*/
	{
		strcpy((char *)(pcObsType+1),"C");
	}
	else if ((dVer>=2.12)&&iSys==SYS_GLO&&!strcmp((char *)(pcObsType+1),"1C")) /*L1C/A*/
	{
		strcpy((char *)(pcObsType+1),"A");
	}
	else if ((dVer>=2.12)&&iSys==SYS_GLO&&!strcmp((char *)(pcObsType+1),"2C")) /*L2C/A*/
	{ 
		strcpy((char *)(pcObsType+1),"D");
	}
	else if (!strcmp((char *)pcObsType,"C1P")||!strcmp((char *)pcObsType,"C1W")||!strcmp((char *)pcObsType,"C1Y")||
		!strcmp((char *)pcObsType,"C1N")) /*L1P,P(Y)*/
	{ 
		strcpy((char *)pcObsType,"P1");
	}
	else if (!strcmp((char *)pcObsType,"C2P")||!strcmp((char *)pcObsType,"C2W")||!strcmp((char *)pcObsType,"C2Y")||
		!strcmp((char *)pcObsType,"C2N")||!strcmp((char *)pcObsType,"C2D")) /*L2P,P(Y)*/
	{ 
		strcpy((char *)pcObsType,"P2");
	}
	else 
	{
		pcObsType[2]='\0';
	}
}

/******************************************************************************
*    函数名称:  SetOpt_ObsType
*    功能描述:  生成RINEX文件中的观测值类型
*    输入参数:  观测值编码，卫星系统，配置结构体变量
*    输出参数:  配置结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void SetOpt_ObsType(INT32U *piCodes,INT32U iSys, T_CONVOPT *ptOpt)
{
	INT8U cType[4];
	CHAR  *pcID;
	INT32U iFreq;
	INT32U i,j,k;

	ptOpt->iNobs[iSys]=0;
	if (!(iNavSys[iSys]&ptOpt->iNavSys))
	{
		return;
	}

	for (i=0;piCodes[i];i++)
	{
		if (!(pcID=Code2Obs(piCodes[i],&iFreq)))
		{
			continue;
		}

		for (j=0;j<4;j++)
		{
			if (!(ptOpt->iObsType&(1<<j)))
			{
				continue;
			}

			sprintf((char *)cType,"%c%s",cType_str[j],pcID); /*RINEX ver.3 观测值类型*/

			if (ptOpt->dRnxver<=2.99)                 /*ver.2*/
			{
	            /*ver.3->ver.2*/
				ConvCode322(ptOpt->dRnxver,iNavSys[0],cType);
				
				for (k=0;k<ptOpt->iNobs[0];k++)       /*是否是重复的观测值类型*/
				{   
					if (!strcmp((char *)ptOpt->cObsType[0][k],(char *)cType)) 
					{
						break;
					}
				}
				if ((k>=ptOpt->iNobs[0])&&(ptOpt->iNobs[0]<MAXOBSTYPE))
				{
					strcpy((char *)ptOpt->cObsType[0][ptOpt->iNobs[0]],(char *)cType);
					ptOpt->iNobs[0]=ptOpt->iNobs[0]+1;
				}
			}
			else if (ptOpt->iNobs[iSys]<MAXOBSTYPE)   /*ver.3*/
			{
				strcpy((char *)ptOpt->cObsType[iSys][ptOpt->iNobs[iSys]],(char *)cType);
				ptOpt->iNobs[iSys]=ptOpt->iNobs[iSys]+1;
			}
		}
	}
}

/******************************************************************************
*    函数名称:  SetObsType
*    功能描述:  设置观测值类型
*    输入参数:  配置结构体变量
*    输出参数:  配置结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void SetObsType(T_CONVOPT *ptOpt)
{
	INT32U *piCodes;
	INT32U i;

	/*RTCM2中编码*/
	INT32U iCodes_rtcm2[3][8]=
	{
		{CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
		{CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P}
	};
	/*RTCM3中编码*/
	INT32U iCodes_rtcm3[3][8]=
	{
		{CODE_L1C,CODE_L2W,CODE_L2X,CODE_L5X},
		{CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
		//{CODE_L2I,CODE_L6I}
		{CODE_L2I,CODE_L7I,CODE_L6I}
	};
	/*RINEX中编码*/
	INT32U iCodes_rinex[3][32]=
	{
		{CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1Y,CODE_L1M,CODE_L1N,CODE_L1S,CODE_L1L,
		 CODE_L2C,CODE_L2D,CODE_L2S,CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,CODE_L2Y,
		 CODE_L2M,CODE_L2N,CODE_L5I,CODE_L5Q,CODE_L5X},
		{CODE_L2I,CODE_L7I,CODE_L6I}
	};
	/*其他类型编码*/
	INT32U iCodes_other[3][8]=
	{
		{CODE_L1C},{CODE_L1C},{CODE_L2I}
	};

	for (i=0;i<3;i++)
	{
		switch (ptOpt->iFormat) 
		{
			case STRFMT_RTCM2: piCodes=iCodes_rtcm2[i];
				               break;
			case STRFMT_RTCM3: piCodes=iCodes_rtcm3[i]; 
				               break;
			case STRFMT_RINEX: piCodes=iCodes_rinex[i]; 
				               break;
			default:           piCodes=iCodes_other[i];
				               break;
		}

		/*生成RINEX文件中的观测值类型*/
		SetOpt_ObsType(piCodes,i,ptOpt);
	}
}

/******************************************************************************
*    函数名称:  InputFile
*    功能描述:  根据电文格式选择电文解析函数
*    输入参数:  RTCM结构体变量，数据来源类型，文件指针
*    输出参数:  RTCM结构体变量
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S InputFile(T_RTCM *ptRtcm,INT32U iFormat,FILE *fp)
{
	INT32S iType;

	if (iFormat==STRFMT_RTCM2)
	{
		iType=Input_RTCM2f(ptRtcm,fp);
	}
	else if (iFormat==STRFMT_RTCM3)
	{
		iType=Input_RTCM3f(ptRtcm,fp);
	}
	else
	{
		return 0;
	}
    
	return iType;
}

/******************************************************************************
*    函数名称:  ConvRnx
*    功能描述:  RTCM电文转换成RINEX格式
*    输入参数:  配置结构体变量，RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
/*RTCM电文转换成RINEX格式-----------------------------------------------------*/
extern void ConvRnx(T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	FILE *fp;//,*fp1
	FILE *ofp[MAXOUTFILE]={NULL};
	INT32S iType;
	INT32U i,iNumErr=0;
	T_WEEKSEC tObsTime;

    T_RTCM tRtcm;
    memset(&tRtcm,0,sizeof(T_RTCM));
	Init_RTCM(&tRtcm);

    ptRtcm->tTime=ptOpt->tTrtcm;
	ptRtcm->tSta =ptOpt->tSta;
	fp=fopen((char *)ptOpt->cInfile,"rb");

	/*打开输出文件*/
	for (i=0;i<MAXOUTFILE;i++)
	{
		if (strcmp((char *)ptOpt->cOutfile[i],""))
		{
			ofp[i]=fopen((char *)ptOpt->cOutfile[i],"w");
		}
	}
	/*RINEX输出文件头文件编写*/
	for (i=0;i<MAXOUTFILE;i++)
	{
		if (ofp[i])
		{
			switch (i)
			{
				case 0: OutRnxObsH(ofp[0],ptOpt,ptRtcm);
						break;
				case 1: OutRnxNavH(ofp[1],ptOpt,ptRtcm);
						break;
				case 2: OutRnxCnavH(ofp[2],ptOpt,ptRtcm);
						break;
				case 3: OutRnxGnavH(ofp[3],ptOpt);
						break;
				default:break;
			}
		}
		else
		{
			continue;
		}
	}
	//fp1=fopen("TEST_O.rtcm3","wb");
	for (i=0;((iType=InputFile(ptRtcm,ptOpt->iFormat,fp))>=-1);i++)
	{
		switch (iType)
		{
			case -1: iNumErr=iNumErr+1;
					 break;
			case  1: fprintf(stderr,"the observation data\n");

				     tObsTime=ptRtcm->tObs.ptData[0].tObsTime;
					 if ((ptOpt->tTstart.iWeek==0)&&(ptOpt->tTstart.dSow==0))
					 {
						 ptOpt->tTstart=tObsTime;
					 }
					 ptOpt->tTend=tObsTime;

					 OutRnxObsB(ofp[0],ptOpt,ptRtcm);

				     /*tRtcm.tObs=ptRtcm->tObs;
					 tRtcm.tTime=ptRtcm->tTime;
				     Output_RTCM3(&tRtcm,4092,1,fp1);*/
				  	 break;
			case  2: fprintf(stderr,"the navigation data\n");
				     OutRnxNavGNSS(ofp,ptOpt,ptRtcm);

				     tRtcm.tNav=ptRtcm->tNav;
				     tRtcm.iEphsat=ptRtcm->iEphsat;
				     Output_RTCM3(&tRtcm,4095,0);
					 break;
			case  5: fprintf(stderr,"the station and antenna message\n");
				     /*tRtcm.tSta=ptRtcm->tSta;
					 tRtcm.iStaid=ptRtcm->iStaid;
				     Output_RTCM3(&tRtcm,1006,0);*/
					 break;
			case  7: fprintf(stderr,"the dgnss corrections message\n");
				     break;

			default: break;
		}
	}

	/*根据RTCM电文更新RINEX输出文件头*/
    for (i=0;i<MAXOUTFILE;i++)
	{
		if (ofp[i])
		{
			rewind(ofp[i]);/*光标回到文件起始位置*/
			switch (i)
			{
				case 0: OutRnxObsH(ofp[0],ptOpt,ptRtcm);
						break;
				case 1: OutRnxNavH(ofp[1],ptOpt,ptRtcm);
						break;
				case 2: OutRnxCnavH(ofp[2],ptOpt,ptRtcm);
						break;
				case 3: OutRnxGnavH(ofp[3],ptOpt);
						break;
				default:break;
			}
			fclose(ofp[i]);
		}
		else
		{
			continue;
		}
	}

	fclose(fp);
}






