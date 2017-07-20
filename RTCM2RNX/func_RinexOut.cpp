/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：func_Rinex.cpp
* 内容摘要：GNSS观测文件和导航文件输出 （RINEX格式）
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
static const double URA_EPH[]=      /*URA值 (m)*/
{
	2.40, 3.40, 4.85, 6.85, 9.65, 13.65, 24.00, 48.00, 96.00, 192.00, 384.00,
	768.00, 1536.00, 3072.00, 6144.00
};

/******************************************************************************
*    函数名称:  OutRnxObsH
*    功能描述:  输出观测文件文件头
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void OutRnxObsH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT8U cSys[15]="";
	T_YMD tYmd={0};
	INT32U i,j;

	/*获得导航系统*/
	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		if (ptOpt->iNavSys==SYS_GPS)
		{
			strcpy((char *)cSys,"G (GPS)");
		}
		else
		{
			strcpy((char *)cSys,"M (MIXED)");
		}
	}
	else /*ver 3*/
	{
		if (ptOpt->iNavSys==SYS_GPS)
		{
			strcpy((char *)cSys,"G: GPS");
		}
		else if (ptOpt->iNavSys==SYS_BDS)
		{
			strcpy((char *)cSys,"C: BeiDou");
		}
		else if (ptOpt->iNavSys==SYS_GLO)
		{
			strcpy((char *)cSys,"R: GLONASS");
		}
		else
		{
			strcpy((char *)cSys,"M: Mixed");
		}
	}

	fprintf(ofp,"%9.2f           %-20s%-20s%20s\n",ptOpt->dRnxver,
			"OBSERVATION DATA",cSys,"RINEX VERSION / TYPE");
	fprintf(ofp,"%-20s%-20s%-20s%-20s\n",ptOpt->tSta.cProg,ptOpt->tSta.cRunBy,
		    ptOpt->tSta.cDate,"PGM / RUN BY / DATE");

	/*注释*/
	for (i=0;i<MAXCOMMENT;i++)
	{
		if (strcmp((char *)ptOpt->tSta.cComment[i],""))
		{
			fprintf(ofp,"%-60s%-20s\n",ptOpt->tSta.cComment[i],"COMMENT");
		}
		else
		{
			continue;
		}
	}

	fprintf(ofp,"%-60s%-20s\n",ptRtcm->tSta.cMarker,"MARKER NAME");
	fprintf(ofp,"%-60s%-20s\n",ptRtcm->tSta.cMarkerno,"MARKER NUMBER");

	if (ptOpt->dRnxver>2.99) /*ver 3*/
	{
		fprintf(ofp,"%-60s%-20s\n",ptRtcm->tSta.cMarkertype,"MARKER TYPE");
	}

	fprintf(ofp,"%-20s%-40s%-20s\n",ptRtcm->tSta.cObser[0],
		    ptRtcm->tSta.cObser[1],"OBSERVER / AGENCY");
	fprintf(ofp,"%-20s%-20s%-20s%-20s\n",ptRtcm->tSta.cRec[0],
		    ptRtcm->tSta.cRec[1],ptRtcm->tSta.cRec[2],"REC # / TYPE / VERS");
	fprintf(ofp,"%-20s%-40s%-20s\n",ptRtcm->tSta.cAnt[0],
		    ptRtcm->tSta.cAnt[1],"ANT # / TYPE");
	fprintf(ofp,"%14.4f%14.4f%14.4f%18s%-20s\n",ptRtcm->tSta.dAppPos[0],
		    ptRtcm->tSta.dAppPos[1],ptRtcm->tSta.dAppPos[2],"","APPROX POSITION XYZ");
	fprintf(ofp,"%14.4f%14.4f%14.4f%18s%-20s\n",ptRtcm->tSta.dAntDel[0],
		    ptRtcm->tSta.dAntDel[1],ptRtcm->tSta.dAntDel[2],"","ANTENNA: DELTA H/E/N");

	/*观测值类型*/
	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%6d%6d%48s%-20s\n",1,1,"","WAVELENGTH FACT L1/2");
		fprintf(ofp,"%6d",ptOpt->iNobs[0]);
		for (i=0;i<ptOpt->iNobs[0];i++)
		{
			/*每行只能包括9种观测值类型*/
			if ((i>0)&&(i%9==0))
			{
				fprintf(ofp,"%6s","");
			}
			fprintf(ofp,"%6s",ptOpt->cObsType[0][i]);
			if (i%9==8)
			{
				fprintf(ofp,"%-20s\n","# / TYPES OF OBSERV");
			}
		}
		if ((ptOpt->iNobs[0]==0)||i==ptOpt->iNobs[0])
		{
			fprintf(ofp,"%*s%-20s\n",6*(9-i%9),"","# / TYPES OF OBSERV");
		}
	}
	else /*ver 3*/
	{
		for (i=0;iNavSys[i];i++)
		{
			if ((ptOpt->iNavSys&iNavSys[i])&&(ptOpt->iNobs[i]!=0))
			{
				fprintf(ofp,"%c  %3d",cSysCodes[i],ptOpt->iNobs[i]);
				for (j=0;j<ptOpt->iNobs[i];j++)
				{
					/*每行只能包括13种观测值类型*/
					if ((j>0)&&(j%13==0))
					{
						fprintf(ofp,"%6s","");
					}
					fprintf(ofp,"%4s",ptOpt->cObsType[i][j]);
					if (j%13==12)
					{
						fprintf(ofp,"  %-20s\n","SYS / # / OBS TYPES");
					}
				}
				if (j=ptOpt->iNobs[i])
				{
					fprintf(ofp,"%*s  %-20s\n",4*(13-j%13),"","SYS / # / OBS TYPES");
				}
			}
			else
			{
				continue;
			}
		}
	}

	tYmd=WeekSec2YMD(ptOpt->tTstart);
	fprintf(ofp,"%6d%6d%6d%6d%6d%13.7f%5sGPS%9s%-20s\n",tYmd.iYear,tYmd.iMon,
		    tYmd.iDay,tYmd.iHor,tYmd.iMin,floor(tYmd.dSec+0.5),"","","TIME OF FIRST OBS");
	tYmd=WeekSec2YMD(ptOpt->tTend);
	fprintf(ofp,"%6d%6d%6d%6d%6d%13.7f%5sGPS%9s%-20s\n",tYmd.iYear,tYmd.iMon,
		    tYmd.iDay,tYmd.iHor,tYmd.iMin,floor(tYmd.dSec+0.5),"","","TIME OF LAST OBS");
	fprintf(ofp,"%6d%54s%-20s\n",LEAPS,"","LEAP SECONDS");
	fprintf(ofp,"%60s%-20s\n","","END OF HEADER");
}

/******************************************************************************
*    函数名称:  OutRnxNavH
*    功能描述:  输出导航文件文件头
*               ver 2输出GPS导航文件文件头
*               ver 3输出GNSS导航文件文件头
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void OutRnxNavH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT32U i;
	INT8U cSys[15]="";

	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%9.2f           %-20s%-20s%20s\n",ptOpt->dRnxver,
			    "N: GPS NAV DATA","","RINEX VERSION / TYPE");
	}
	else  /*ver 3*/
	{
		if (ptOpt->iNavSys==SYS_GPS)
		{
			strcpy((char *)cSys,"G: GPS");
		}
		else if (ptOpt->iNavSys==SYS_BDS)
		{
			strcpy((char *)cSys,"C: BeiDou");
		}
		else if (ptOpt->iNavSys==SYS_GLO)
		{
			strcpy((char *)cSys,"R: GLONASS");
		}
		else
		{
			strcpy((char *)cSys,"M: Mixed");
		}

		fprintf(ofp,"%9.2f           %-20s%-20s%-20s\n",ptOpt->dRnxver,
			    "N: GNSS NAV DATA",cSys,"RINEX VERSION / TYPE");
	}

	fprintf(ofp,"%-20s%-20s%-20s%-20s\n",ptOpt->tSta.cProg,ptOpt->tSta.cRunBy,
		    ptOpt->tSta.cDate,"PGM / RUN BY / DATE");
    
	/*注释*/
	for (i=0;i<MAXCOMMENT;i++)
	{
		if (strcmp((char *)ptOpt->tSta.cComment[i],""))
		{
			fprintf(ofp,"%-60s%-20s\n",ptOpt->tSta.cComment[i],"COMMENT");
		}
		else
		{
			continue;
		}
	}

	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",ptRtcm->tNav.dIon_gps[0],
				ptRtcm->tNav.dIon_gps[1],ptRtcm->tNav.dIon_gps[2],
				ptRtcm->tNav.dIon_gps[3],"","ION ALPHA");
		fprintf(ofp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",ptRtcm->tNav.dIon_gps[4],
				ptRtcm->tNav.dIon_gps[5],ptRtcm->tNav.dIon_gps[6],
				ptRtcm->tNav.dIon_gps[7],"","ION BETA");
		fprintf(ofp,"   %19.12E%19.12E%9.0f%9.0f %-20s\n",ptRtcm->tNav.dUtc_gps[0],
				ptRtcm->tNav.dUtc_gps[1],ptRtcm->tNav.dUtc_gps[2],
				ptRtcm->tNav.dUtc_gps[3],"DELTA-UTC: A0,A1,T,W");
	}
	else /*ver 3*/
	{
		if (ptOpt->iNavSys&SYS_GPS)
		{
			fprintf(ofp,"GPSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",ptRtcm->tNav.dIon_gps[0],
				    ptRtcm->tNav.dIon_gps[1],ptRtcm->tNav.dIon_gps[2],
				    ptRtcm->tNav.dIon_gps[3],"","IONOSPHERIC CORR");
			fprintf(ofp,"GPSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",ptRtcm->tNav.dIon_gps[4],
				    ptRtcm->tNav.dIon_gps[5],ptRtcm->tNav.dIon_gps[6],
				    ptRtcm->tNav.dIon_gps[7],"","IONOSPHERIC CORR");
		}
		if (ptOpt->iNavSys&SYS_BDS)
		{
			fprintf(ofp,"BDSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",ptRtcm->tNav.dIon_bds[0],
				    ptRtcm->tNav.dIon_bds[1],ptRtcm->tNav.dIon_bds[2],
				    ptRtcm->tNav.dIon_bds[3],"","IONOSPHERIC CORR");
			fprintf(ofp,"BDSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",ptRtcm->tNav.dIon_bds[4],
				    ptRtcm->tNav.dIon_bds[5],ptRtcm->tNav.dIon_bds[6],
				    ptRtcm->tNav.dIon_bds[7],"","IONOSPHERIC CORR");
		}
		if(ptOpt->iNavSys&SYS_GPS)
		{
			fprintf(ofp,"GPUT %17.10E%16.9E%7.0f%5.0f%10s%-20s\n",ptRtcm->tNav.dUtc_gps[0],
			     	ptRtcm->tNav.dUtc_gps[1],ptRtcm->tNav.dUtc_gps[2],
				    ptRtcm->tNav.dUtc_gps[3],"","TIME SYSTEM CORR");
		}
		if(ptOpt->iNavSys&SYS_BDS)
		{
			fprintf(ofp,"BDUT %17.10E%16.9E%7.0f%5.0f%10s%-20s\n",ptRtcm->tNav.dUtc_bds[0],
			     	ptRtcm->tNav.dUtc_bds[1],ptRtcm->tNav.dUtc_bds[2],
				    ptRtcm->tNav.dUtc_bds[3],"","TIME SYSTEM CORR");
		}
	}
	fprintf(ofp,"%6d%54s%-20s\n",LEAPS,"","LEAP SECONDS");
    fprintf(ofp,"%60s%-20s\n","","END OF HEADER");
}

/******************************************************************************
*    函数名称:  OutRnxCnavH
*    功能描述:  输出BDS导航文件文件头
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void OutRnxCnavH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT32U i;

	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%9.2f           %-20s%-20s%20s\n",ptOpt->dRnxver,
			    "N: BDS NAV DATA","","RINEX VERSION / TYPE");
	}
	else /*ver 3*/
	{
		fprintf(ofp,"%9.2f           %-20s%-20s%-20s\n",ptOpt->dRnxver,
			    "N: GNSS NAV DATA","C: BeiDou","RINEX VERSION / TYPE");
	}

	fprintf(ofp,"%-20s%-20s%-20s%-20s\n",ptOpt->tSta.cProg,ptOpt->tSta.cRunBy,
		    ptOpt->tSta.cDate,"PGM / RUN BY / DATE");
    
	/*注释*/
	for (i=0;i<MAXCOMMENT;i++)
	{
		if (strcmp((char *)ptOpt->tSta.cComment[i],""))
		{
			fprintf(ofp,"%-60s%-20s\n",ptOpt->tSta.cComment[i],"COMMENT");
		}
		else
		{
			continue;
		}
	}

	fprintf(ofp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",ptRtcm->tNav.dIon_bds[0],
		    ptRtcm->tNav.dIon_bds[1],ptRtcm->tNav.dIon_bds[2],
		    ptRtcm->tNav.dIon_bds[3],"","ION ALPHA");
	fprintf(ofp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",ptRtcm->tNav.dIon_bds[4],
		    ptRtcm->tNav.dIon_bds[5],ptRtcm->tNav.dIon_bds[6],
		    ptRtcm->tNav.dIon_bds[7],"","ION BETA");
	fprintf(ofp,"   %19.12E%19.12E%9.0f%9.0f %-20s\n",ptRtcm->tNav.dUtc_bds[0],
		    ptRtcm->tNav.dUtc_bds[1],ptRtcm->tNav.dUtc_bds[2],
		    ptRtcm->tNav.dUtc_bds[3],"DELTA-UTC: A0,A1,T,W");
	fprintf(ofp,"%6d%54s%-20s\n",LEAPS,"","LEAP SECONDS");
	fprintf(ofp,"%60s%-20s\n","","END OF HEADER");
}

/******************************************************************************
*    函数名称:  OutRnxGnavH
*    功能描述:  输出GLONASS导航文件文件头
*    输入参数:  文件指针，配置结构体指针
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void OutRnxGnavH(FILE *ofp,T_CONVOPT *ptOpt)
{
	INT32U i;

	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%9.2f           %-20s%-20s%20s\n",ptOpt->dRnxver,
			    "GLONASS NAV DATA","","RINEX VERSION / TYPE");
	}
	else /*ver 3*/
	{
		fprintf(ofp,"%9.2f           %-20s%-20s%-20s\n",ptOpt->dRnxver,
		    	"N: GNSS NAV DATA","R: GLONASS","RINEX VERSION / TYPE");
	}
	fprintf(ofp,"%-20s%-20s%-20s%-20s\n",ptOpt->tSta.cProg,ptOpt->tSta.cRunBy,
		    ptOpt->tSta.cDate,"PGM / RUN BY / DATE");
    
	for (i=0;i<MAXCOMMENT;i++)
	{
		if (strcmp((char *)ptOpt->tSta.cComment[i],""))
		{
			fprintf(ofp,"%-60s%-20s\n",ptOpt->tSta.cComment[i],"COMMENT");
		}
		else
		{
			continue;
		}
	}
	fprintf(ofp,"%60s%-20s\n","","END OF HEADER");
}

/******************************************************************************
*    函数名称:  Sat2Code
*    功能描述:  对卫星PRN及卫星系统进行编码
*    输入参数:  卫星编号，卫星编码
*    输出参数:  卫星编码
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U Sat2Code(INT32U iSat,INT8U *cCode)
{
	INT32U iSys,iPrn;

	iSys=Sat2Sys(iSat,&iPrn);

	switch (iSys)
	{
		case SYS_GPS: sprintf((char *)cCode,"G%02d",iPrn);
					  break;
		case SYS_BDS: sprintf((char *)cCode,"C%02d",iPrn);
					  break;
		case SYS_GLO: sprintf((char *)cCode,"R%02d",iPrn);
					  break;
		default:      return 0;
	}

    return 1;
}

/******************************************************************************
*    函数名称:  ObsIndex
*    功能描述:  根据卫星系统、观测值类型、观测值编码，找到观测类型对应的频率
*    输入参数:  RINEX版本类型，卫星系统，观测值编码，观测值类型
*    输出参数:  无
*    返 回 值:  观测值频率
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32S ObsIndex(FP64 dRnxVer,INT32U iSys,const INT8U *pcCode,const INT8U *pcObsType)
{
	//INT8U *pcID;
	char *pcID;
	INT32U i;

	for (i=0;i<NFREQ+NEXOBS;i++)
	{
		if (dRnxVer<=2.99) /*ver 2*/
		{
			if (!strcmp((char *)pcObsType,"C1"))
			{
				if (pcCode[i]==CODE_L1C)
				{
					return i;
				}
			}
			else if (!strcmp((char *)pcObsType,"P1"))
			{
				if ((pcCode[i]==CODE_L1P)||(pcCode[i]==CODE_L1W)||
					(pcCode[i]==CODE_L1Y)||(pcCode[i]==CODE_L1N))
				{
					return i;
				}
			}
			else if((!strcmp((char *)pcObsType,"C2"))&&(iSys==SYS_GPS))
			{
				if ((pcCode[i]==CODE_L2S)||(pcCode[i]==CODE_L2L)||
					(pcCode[i]==CODE_L2X))
				{
					return i;
				}
			}
			else if ((!strcmp((char *)pcObsType,"C2"))&&(iSys==SYS_GLO))
			{
				if (pcCode[i]==CODE_L2C)
				{
					return i;
				}
			}
			else if (!strcmp((char *)pcObsType,"P2"))
			{
				if ((pcCode[i]==CODE_L2P)||(pcCode[i]==CODE_L2W)||
					(pcCode[i]==CODE_L2Y)||(pcCode[i]==CODE_L2N)||
					(pcCode[i]==CODE_L2D))
				{
					return i;
				}
			}
            else if ((dRnxVer>=2.12)&&(pcObsType[1]=='A')) /*L1C/A*/
			{ 
                if (pcCode[i]==CODE_L1C)
				{
					return i;
				}
            }
            else if ((dRnxVer>=2.12)&&(pcObsType[1]=='B')) /*L1C*/ 
			{
                if ((pcCode[i]==CODE_L1S)||(pcCode[i]==CODE_L1L)||
					(pcCode[i]==CODE_L1X))
				{
                    return i;
				}
            }
            else if ((dRnxVer>=2.12)&&(pcObsType[1]=='C')) /*L2C*/ 
			{
                if ((pcCode[i]==CODE_L2S)||(pcCode[i]==CODE_L2L)||
					(pcCode[i]==CODE_L2X))
				{
                    return i;
				}
            }
            else if ((dRnxVer>=2.12)&&(pcObsType[1]=='D')&&(iSys==SYS_GLO)) /*GLO L2C/A*/
			{ 
                if (pcCode[i]==CODE_L2C) 
				{
					return i;
				}
            }
            else 
			{
                pcID=Code2Obs(pcCode[i],NULL);
                if (pcID[0]==pcObsType[1]) 
				{
					return i;
				}
            }
		}
		else /*ver 3*/
		{
			pcID=Code2Obs(pcCode[i],NULL);
			if (!strcmp((char *)pcID,(char *)(pcObsType+1)))
			{
				return i;
			}
		}
	}

	return -1;
}

/******************************************************************************
*    函数名称:  OutRnxObsData
*    功能描述:  输出观测值数据
*    输入参数:  文件指针，数据，失锁标识
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static void OutRnxObsData(FILE *ofp,FP64 dData,INT32S iLLI)
{
	/*观测值*/
	if ((dData==0.0)||(dData<=1E-9)||dData>=1E9)
	{
		fprintf(ofp,"%-14s","");
	}
	else
	{
		fprintf(ofp,"%14.3f",dData);
	}
	/*失锁标识符*/
	if (iLLI<=0)
	{
		fprintf(ofp,"%-2s","");
	}
	else
	{
		fprintf(ofp,"%1d ",iLLI);
	}
}

/******************************************************************************
*    函数名称:  OutRnxObsB
*    功能描述:  输出历元卫星观测值
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  输出标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U OutRnxObsB(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	T_OBSSAT tData;
	T_YMD tObsTime;
	INT32U iSys,iPrn,iNsat=0;
	INT8U cSat[MAXSAT][4]={""};
	INT32U iInd;
	INT32U i,j;
	INT32S k;

	/*观测历元时刻转换成通用时表示*/
	tObsTime=WeekSec2YMD(ptRtcm->tObs.ptData[0].tObsTime);

	/*统计满足要求的卫星个数及卫星号编码*/
	for (i=0;i<ptRtcm->tObs.iN;i++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[i].iSat,&iPrn);
		if (iSys&ptOpt->iNavSys)
		{
			if (Sat2Code(ptRtcm->tObs.ptData[i].iSat,cSat[iNsat])==0)
			{
				return 0;
			}	
			iNsat=iNsat+1;
		}
	}

	/*观测值历元头*/
	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp," %02d %2d %2d %2d %2d%11.7f  %d%3d",tObsTime.iYear%100,
				tObsTime.iMon,tObsTime.iDay,tObsTime.iHor,tObsTime.iMin,
				floor(tObsTime.dSec+0.5),0,iNsat);
		for (i=0;i<iNsat;i++)
		{
			if ((i>0)&&(i%12==0))
			{
				fprintf(ofp,"\n%32s","");
			}
			fprintf(ofp,"%-3s",cSat[i]);
		}
	}
	else /*ver 3*/
	{
		fprintf(ofp,"> %4d %02d %02d %02d %02d%11.7f  %d%3d%21s\n",
				tObsTime.iYear,tObsTime.iMon,tObsTime.iDay,tObsTime.iHor,
				tObsTime.iMin,floor(tObsTime.dSec+0.5),0,iNsat,"");
	}

	/*逐颗卫星输出观测值*/
	iNsat=0;
	for (i=0;i<ptRtcm->tObs.iN;i++)
	{
		iSys=Sat2Sys(ptRtcm->tObs.ptData[i].iSat,&iPrn);
		if (!(iSys&ptOpt->iNavSys))
		{
			continue;
		}

		/*判断不同导航系统对应的观测值类型存储位置*/
		if (ptOpt->dRnxver<=2.99) /*ver 2*/
		{
			iInd=0;
		}
		else
		{
			fprintf(ofp,"%-3s",cSat[iNsat]);
			iNsat=iNsat+1;
			switch (iSys)
			{
				case SYS_GPS: iInd=0;
					          break;
				case SYS_GLO: iInd=1;
					          break;
				case SYS_BDS: iInd=2;
					          break;
				default:      break;
			}
		}

		tData=ptRtcm->tObs.ptData[i];

		/*按照观测值类型逐个输出观测值*/
		for (j=0;j<ptOpt->iNobs[iInd];j++)
		{
			if ((ptOpt->dRnxver<=2.99)&&(j%5==0)) /*ver 2每行只有五个观测值*/
			{
				fprintf(ofp,"\n");
			}

			/*观测值类型索引输出观测值*/
			if ((k=ObsIndex(ptOpt->dRnxver,iSys,tData.cCode,ptOpt->cObsType[iInd][j]))<0)
			{
				fprintf(ofp,"%-16s","");
			}
			else
			{
				switch (ptOpt->cObsType[iInd][j][0])
				{
					case 'C': OutRnxObsData(ofp,tData.dP[k],-1);
							  break;
					case 'P': OutRnxObsData(ofp,tData.dP[k],-1);
							  break;
					case 'L': OutRnxObsData(ofp,tData.dL[k],tData.cLLI[k]);
							  break;
					case 'S': OutRnxObsData(ofp,tData.dCNR[k],-1);
							  break;
					case 'D': OutRnxObsData(ofp,tData.fD[k],-1);
							  break;
				}
			}
		}
		if (ptOpt->dRnxver>2.99)
		{
			fprintf(ofp,"\n");
		}
	}
	if (ptOpt->dRnxver<=2.99)
	{
		fprintf(ofp,"\n");
	}

	return 1;
}

/******************************************************************************
*    函数名称:  URA_VALUE
*    功能描述:  URA指数转换为URA值
*    输入参数:  卫星精度URA指数
*    输出参数:  无
*    返 回 值:  URA值
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static FP64 URA_VALUE(INT32U iSva)
{
	if ((iSva>=0)&&(iSva<15))
	{
		return URA_EPH[iSva];
	}
	else
	{
		return 32767.0;
	}
}

///* ura value (m) to ura index ------------------------------------------------*/
//static int uraindex(double value)
//{
//	int i;
//	for (i=0;i<15;i++) if (ura_eph[i]>=value) break;
//	return i;
//}

/******************************************************************************
*    函数名称:  OutRnxNavB
*    功能描述:  输出GPS/BDS导航星历
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  输出标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U OutRnxNavB(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT32U iSys,iPrn;
	T_EPH tEph;	
	T_YMD tToc;
	INT8U cSeg[5]="",cSat[4]="";

	iSys=Sat2Sys(ptRtcm->iEphsat,&iPrn);

	/*检查卫星一致性*/
	if (ptRtcm->iEphsat!=ptRtcm->tNav.ptEph[ptRtcm->iEphsat-1].iSat)
	{
		return 0;
	}

	tEph=ptRtcm->tNav.ptEph[ptRtcm->iEphsat-1]; /*卫星星历*/

	/*卫星星历TOC时间*/
	tToc=WeekSec2YMD(tEph.tToc);
	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%2d %02d %2d %2d %2d %2d %4.1f",iPrn,tToc.iYear%100,
			tToc.iMon,tToc.iDay,tToc.iHor,tToc.iMin,tToc.dSec);
		strcpy((char *)cSeg,"   ");
	}
	else /*ver 3*/
	{	
		if (Sat2Code(tEph.iSat,cSat)==0)
		{
			return 0;
		}

		fprintf(ofp,"%3s %4d %02d %02d %02d %02d %02d",cSat,tToc.iYear,
				tToc.iMon,tToc.iDay,tToc.iHor,tToc.iMin,(int)tToc.dSec);
		strcpy((char *)cSeg,"    ");
	}

	fprintf(ofp,"% 19.11E% 19.11E% 19.11E\n",tEph.dF0,tEph.dF1,tEph.dF2);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,(double)tEph.iIode,
		    tEph.dCrs,tEph.dDeln,tEph.dM0);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tEph.dCuc,
		    tEph.dE,tEph.dCus,tEph.dSqrtA);
    fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tEph.dToes,
		    tEph.dCic,tEph.dOMG0,tEph.dCis);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tEph.dI0,
		    tEph.dCrc,tEph.dOmg,tEph.dOMGd);

	if (iSys==SYS_GPS) /*GPS*/
	{
		fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tEph.dIdot,
				(double)tEph.iCode,(double)tEph.tToe.iWeek,(double)tEph.iFlag);
		fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,URA_VALUE(tEph.iSva),
				(double)tEph.iSvh,tEph.dTgd[0],(double)tEph.iIodc);
		fprintf(ofp,"%s% 19.11E% 19.11E% 19s% 19s\n",cSeg,tEph.tTtr.dSow,
				tEph.dFit,"","");
	}
	else if (iSys==SYS_BDS) /*BDS*/
	{
		fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tEph.dIdot,
				0.0,(double)tEph.tToe.iWeek,0.0);
		fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,URA_VALUE(tEph.iSva),
				(double)tEph.iSvh,tEph.dTgd[0],tEph.dTgd[1]);
		fprintf(ofp,"%s% 19.11E% 19.11E% 19s% 19s\n",cSeg,tEph.tTtr.dSow,
				(double)tEph.iIodc,"","");
	}

	return 1;
}

/******************************************************************************
*    函数名称:  OutRnxGnavB
*    功能描述:  输出GLONASS导航星历
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  输出标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
static INT32U OutRnxGnavB(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT32U iPrn;
	T_GLOEPH tGeph;
	T_WEEKSEC tUtc;
	T_YMD tToc;
	FP64 dTof;
	INT8U cSeg[5]="",cSat[4]="";

	Sat2Sys(ptRtcm->iEphsat,&iPrn);

    /*检查卫星一致性*/
	if (ptRtcm->iEphsat!=ptRtcm->tNav.ptGeph[iPrn-1].iSat)
	{
		return 0;
	}
	tGeph=ptRtcm->tNav.ptGeph[iPrn-1];

	/*toc和tof转换为UTC时*/
	tUtc=TimeAdd(tGeph.tToc,-LEAPS); /*GPS时转换成UTC*/
	tToc=WeekSec2YMD(tUtc);

	tUtc=TimeAdd(tGeph.tTof,-LEAPS); /*GPS时转换成UTC*/
	dTof=tUtc.dSow;
	if (ptOpt->dRnxver<=2.99)
	{
		dTof=fmod(tUtc.dSow,D2S);
	}

	/*卫星星历TOC时间*/
	if (ptOpt->dRnxver<=2.99) /*ver 2*/
	{
		fprintf(ofp,"%2d %02d %2d %2d %2d %2d %4.1f",iPrn,tToc.iYear%100,
			tToc.iMon,tToc.iDay,tToc.iHor,tToc.iMin,tToc.dSec);
		strcpy((char *)cSeg,"   ");
	}
	else /*ver 3*/
	{
		if (Sat2Code(tGeph.iSat,cSat)==0)
		{
			return 0;
		}

		fprintf(ofp,"%3s %4d %02d %02d %02d %02d %02d",cSat,tToc.iYear,
			    tToc.iMon,tToc.iDay,tToc.iHor,tToc.iMin,(int)tToc.dSec);
		strcpy((char *)cSeg,"    ");
	}

	fprintf(ofp,"% 19.11E% 19.11E% 19.11E\n",-tGeph.dTaun,tGeph.dGamn,dTof);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tGeph.dPos[0],
		    tGeph.dVel[0],tGeph.dAcc[0],(double)tGeph.iSvh);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tGeph.dPos[1],
		    tGeph.dVel[1],tGeph.dAcc[1],(double)tGeph.iFrq);
	fprintf(ofp,"%s% 19.11E% 19.11E% 19.11E% 19.11E\n",cSeg,tGeph.dPos[2],
		    tGeph.dVel[2],tGeph.dAcc[2],(double)tGeph.iAge);

	return 1;
}

/******************************************************************************
*    函数名称:  OutRnxNavGNSS
*    功能描述:  输出卫星导航星历
*    输入参数:  文件指针，配置结构体指针，RTCM结构体指针
*    输出参数:  文件指针
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void OutRnxNavGNSS(FILE **ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm)
{
	INT32U iSys,iPrn;

	iSys=Sat2Sys(ptRtcm->iEphsat,&iPrn);

	if (iSys==SYS_GPS)
	{
		if (ofp[1]) /*输出文件2存储GPS星历，ver 3 GNSS混合星历*/
		{
			OutRnxNavB(ofp[1],ptOpt,ptRtcm);
		}
	}
	else if (iSys==SYS_BDS)
	{
		if ((ptOpt->dRnxver<=2.99)&&ofp[2]) /*输出文件3存储BDS星历*/
		{
			OutRnxNavB(ofp[2],ptOpt,ptRtcm);
		}
		else if ((ptOpt->dRnxver>2.99)&&ofp[1])
		{
			OutRnxNavB(ofp[1],ptOpt,ptRtcm);
		}
	}
	else if (iSys==SYS_GLO)
	{
		if ((ptOpt->dRnxver<=2.99)&&ofp[3]) /*输出文件4存储GLONASS星历*/
		{
			OutRnxGnavB(ofp[3],ptOpt,ptRtcm);
		}
		else if ((ptOpt->dRnxver>2.99)&&ofp[1])
		{
			OutRnxGnavB(ofp[1],ptOpt,ptRtcm);
		}
	}
}





