/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：RTCM2RNX.cpp
* 内容摘要：项目配置文件
*           RINEX输出文件
* 其他说明：usage:RxCOM4_20170313085705.rtcm3 -tr 2017/3/13 00:59:44 -v 3.01 -r rtcm3 -f 2
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

/******************************************************************************
*                          静态数据变量                                       *
******************************************************************************/
/*帮助文档--------------------------------------------------------------------*/
static const char *help[]=
{
	"",
	" convbin [option ...] file", 
	"",
	" Description",
	"",
	" RTCM电文解析及编码",
	"",
	" RTCM 2                : Type 1, 3, 9, 14, 16, 17, 18, 19, 22",
	" RTCM 3                : Type 1001, 1002, 1003, 1004, 1005, 1006, 1009, 1010, 1011, 1012, 1019, 1020",
	" RINEX                 : OBS, NAV, GNAV, HNAV, LNAV, QNAV",
	"",
	" Options [default]",
	"",
	"     file      input receiver binary log file",
	"     -ts y/m/d h:m:s  start time [all]",
	"     -te y/m/d h:m:s  end time [all]",
	"     -tr y/m/d h:m:s  approximated time for rtcm messages",
	"     -ti tint  observation data interval (s) [all]",
	"     -r format log format type",
	"               rtcm2= RTCM 2",
	"               rtcm3= RTCM 3",
	"               rinex= RINEX",
	"     -f freq   number of frequencies [2]",
	"     -hc comment  rinex header: comment line",
	"     -hm marker   rinex header: marker name",
	"     -hn markno   rinex header: marker number",
	"     -ht marktype rinex header: marker type",
	"     -ho observ   rinex header: oberver name and agency separated by /",
	"     -hr rec      rinex header: receiver number, type and version separated by /",
	"     -ha ant      rinex header: antenna number and type separated by /",
	"     -hp pos      rinex header: approx position x/y/z separated by /",
	"     -hd delta    rinex header: antenna delta h/e/n separated by /",
	"     -v ver    rinex version [2.11]",
	"",
	"     -od       include doppler frequency in rinex obs [off]",
	"     -d dir    output directory [same as input file]",
	"     -c staid  use RINEX file name convention with staid [off]",
	"     -o ofile  output RINEX OBS file",
	"     -n nfile  output RINEX NAV file",
	"     -g gfile  output RINEX GNAV file",
	"",
	" If any output file specified, default output files (<file>.obs,",
	" <file>.nav, <file>.gnav) are used.",
	"",
	" If receiver type is not specified, type is recognized by the input",
	" file extension as follows.",
	"     *.rtcm2    RTCM 2",
	"     *.rtcm3    RTCM 3",
	"     *.obs,*.*o RINEX OBS"
};

static const INT8U FormatStr[3][8]=
{     
    "RTCM 2",                   /* 0 */
    "RTCM 3",                   /* 1 */
    "RINEX"                     /* 3 */
};

/******************************************************************************
*    函数名称:  printhelp
*    功能描述:  输出帮助文档函数
*    输入参数:  无
*    输出参数:  无
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
void printhelp(void)
{
	INT32U i,iLen;

	iLen=(unsigned int)(sizeof(help)/sizeof(*help));
	for (i=0;i<iLen;i++) 
	{
		fprintf(stderr,"%s\n",help[i]);
	}
	exit(0);
}

/******************************************************************************
*    函数名称:  GetOpt
*    功能描述:  读取项目配置项
*    输入参数:  
*    输出参数:  
*    返 回 值:  
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
INT32S main_back(INT32U argc, INT8S **argv)
{
	T_CONVOPT tOpt={{0}};
	T_YMD tEps={1980,1,1,0,0,0},tEpe={2037,12,31,0,0,0};
	T_YMD tEpr={2015,1,1,0,0,0};
	INT32U i,j,iNc=2u;
	INT8U *pcP=NULL,cFmt[10]="";
    T_RTCM tRtcm;
    memset(&tRtcm,0,sizeof(T_RTCM));

	Init_RTCM(&tRtcm);
	//tRtcm.iStaid = 5;//从多站数据中解其中一个测站
	
	InitialQC();//初始化周跳探测

	tOpt.dRnxver =2.10;
	tOpt.iObsType=OBSTYPE_PR|OBSTYPE_CP|OBSTYPE_SNR;
	tOpt.iNavSys =SYS_GPS|SYS_GLO|SYS_BDS;//
	tOpt.iNfreq=2;

	for (i=1;i<argc;i++)
	{
		if (!strcmp((char *)argv[i],"-ts")&&(i+2<argc))
		{
			sscanf((char *)argv[++i],"%ld/%ld/%ld",&(tEps.iYear),&(tEps.iMon),&(tEps.iDay));
			sscanf((char *)argv[++i],"%ld:%ld:%lf",&(tEps.iHor),&(tEps.iMin),&(tEps.dSec));
			tOpt.tTs=YMD2WeekSec(tEps);
		}
		else if (!strcmp((char *)argv[i],"-te")&&(i+2<argc))
		{
			sscanf((char *)argv[++i],"%ld/%ld/%ld",&(tEpe.iYear),&(tEpe.iMon),&(tEpe.iDay));
			sscanf((char *)argv[++i],"%ld:%ld:%lf",&(tEpe.iHor),&(tEpe.iMin),&(tEpe.dSec));
			tOpt.tTe=YMD2WeekSec(tEpe);
		}
		else if (!strcmp((char *)argv[i],"-tr")&&(i+2<argc))
		{
			sscanf((char *)argv[++i],"%ld/%ld/%ld",&(tEpr.iYear),&(tEpr.iMon),&(tEpr.iDay));
			sscanf((char *)argv[++i],"%ld:%ld:%lf",&(tEpr.iHor),&(tEpr.iMin),&(tEpr.dSec));
			tOpt.tTrtcm=YMD2WeekSec(tEpr);
		}
		else if (!strcmp((char *)argv[i],"-ti")&&(i+1<argc))
		{
			tOpt.dTint=atof((char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-r")&&(i+1<argc))
		{
			strcpy((char *)cFmt,(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-f")&&(i+1<argc))
		{
			tOpt.iNfreq=atoi((char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-hc")&&(i+1<argc))
		{
			if(iNc<MAXCOMMENT)
			{
				strcpy((char *)tOpt.tSta.cComment[iNc],(char *)argv[++i]);
				iNc=iNc+1;
			}
		}
		else if (!strcmp((char *)argv[i],"-hm")&&(i+1<argc))
		{
			strcpy((char *)tOpt.tSta.cMarker,(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-hn")&&(i+1<argc))
		{
			strcpy((char *)tOpt.tSta.cMarkerno,(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-ht")&&(i+1<argc))
		{
			strcpy((char *)tOpt.tSta.cMarkertype,(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-ho")&&i+1<argc) 
		{
			for (j=0,pcP=(unsigned char *)strtok((char *)argv[++i],"/");j<2&&pcP;j++,pcP=(unsigned char *)strtok(NULL,"/")) 
			{
				strcpy((char *)tOpt.tSta.cObser[j],(char *)pcP);
			}
		}
		else if (!strcmp((char *)argv[i],"-hr")&&i+1<argc)
		{
			for (j=0,pcP=(unsigned char *)strtok((char *)argv[++i],"/");j<3&&pcP;j++,pcP=(unsigned char *)strtok(NULL,"/")) 
			{
				strcpy((char *)tOpt.tSta.cRec[j],(char *)pcP);
			}
		}
		else if (!strcmp((char *)argv[i],"-ha")&&i+1<argc) 
		{
			for (j=0,pcP=(unsigned char *)strtok((char *)argv[++i],"/");j<2&&pcP;j++,pcP=(unsigned char *)strtok(NULL,"/")) 
			{
				strcpy((char *)tOpt.tSta.cAnt[j],(char *)pcP);
			}
		}
		else if (!strcmp((char *)argv[i],"-hp")&&i+1<argc) 
		{
			for (j=0,pcP=(unsigned char *)strtok((char *)argv[++i],"/");j<3&&pcP;j++,pcP=(unsigned char *)strtok(NULL,"/")) 
			{
				tOpt.tSta.dAppPos[j]=atof((char *)pcP);
			}      
		}
		else if (!strcmp((char *)argv[i],"-hd")&&i+1<argc) 
		{
			for (j=0,pcP=(unsigned char *)strtok((char *)argv[++i],"/");j<3&&pcP;j++,pcP=(unsigned char *)strtok(NULL,"/")) 
			{
				tOpt.tSta.dAntDel[j]=atof((char *)pcP);
			}
		}
		else if (!strcmp((char *)argv[i],"-v")&&(i+1<argc))
		{
			tOpt.dRnxver=atof((char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-od"))
		{
			tOpt.iObsType|=OBSTYPE_DOP;
		}
		else if (!strcmp((char *)argv[i],"-c" )&&i+1<argc) 
		{
			strcpy((char *)tOpt.cStanam,(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-o" )&&i+1<argc)
		{
			strcpy((char *)tOpt.cOutfile[0],(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-n" )&&i+1<argc) 
		{
			strcpy((char *)tOpt.cOutfile[1],(char *)argv[++i]);
		}
		else if (!strcmp((char *)argv[i],"-g" )&&i+1<argc) 
		{
			strcpy((char *)tOpt.cOutfile[2],(char *)argv[++i]);
		}
		else if (!strncmp((char *)argv[i],"-",1))
		{
			printhelp();
		}
		else 
		{
            strcpy((char *)tOpt.cInfile,(char *)argv[i]);
		}
	}

	if (strcmp((char *)cFmt,"")) /*确定输入流格式*/
	{
		if (!strcmp((char *)cFmt,"rtcm2"))
		{
			tOpt.iFormat=STRFMT_RTCM2;
		}
		else if (!strcmp((char *)cFmt,"rtcm3"))
		{
			tOpt.iFormat=STRFMT_RTCM3;
		}
		else if (!strcmp((char *)cFmt,"rinex"))
		{
			tOpt.iFormat=STRFMT_RINEX;
		}
	}
	else if ((pcP=(unsigned char *)strrchr((char *)tOpt.cInfile,'.')))
	{
		if (!strcmp((char *)pcP,".rtcm2"))  
		{
			tOpt.iFormat=STRFMT_RTCM2;
		}
		else if (!strcmp((char *)pcP,".rtcm3")) 
		{
			tOpt.iFormat=STRFMT_RTCM3;
		}
		else if (!strcmp((char *)pcP,".obs")) 
		{
			tOpt.iFormat=STRFMT_RINEX;
		}
		else if (!strcmp((char *)(pcP+3),"o")) 
		{
			tOpt.iFormat=STRFMT_RINEX;
		}
		else if (!strcmp((char *)(pcP+3),"O"))
		{
			tOpt.iFormat=STRFMT_RINEX;
		}
	}

	sprintf((char *)tOpt.tSta.cProg,"%s","program by RTCM2RNX");
	sprintf((char *)tOpt.tSta.cComment[0],"log: %s",tOpt.cInfile);
	sprintf((char *)tOpt.tSta.cComment[1],"format: %s",FormatStr[tOpt.iFormat]);

	if (!SetOutFileName(&tOpt))
	{
		return 0; 
	}

	SetObsType(&tOpt); /*设置RINEX观测值类型*/
	
	ConvRnx(&tOpt,&tRtcm);   /*数据解码或编码*/

	return 1;
}

/******************************************************************************
*    函数名称:  SetOutFileName
*    功能描述:  输出文件名设置
*    输入参数:  配置结构体变量
*    输出参数:  配置结构体变量
*    返 回 值:  文件名设置标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U SetOutFileName(T_CONVOPT *ptOpt)
{
	INT8U cExtNav[2],*p;
	T_YEARDAY tDoy;
	T_WEEKSEC tWS;

	/*判断输出导航文件文件名的后缀*/
	if ((ptOpt->dRnxver<=2.99)||(ptOpt->iNavSys==SYS_GPS)) 
	{
		strcpy((char *)cExtNav,"n");
	}
	else
	{
		strcpy((char *)cExtNav,"P");
	}

	if(ptOpt->iFormat==STRFMT_RTCM2||ptOpt->iFormat==STRFMT_RTCM3) 
	{
		tWS=ptOpt->tTrtcm;
	}
	tDoy=WeekSec2DOY(tWS); /*周秒时转换为年积日*/
    
	if(!strcmp((char *)ptOpt->cOutfile[0],"")&&strcmp((char *)ptOpt->cStanam,"")) /*观测文件*/
	{
		sprintf((char *)ptOpt->cOutfile[0],"%s%3d0.%02.0fO",ptOpt->cStanam,tDoy.iDoy,fmod(tDoy.iYear,100.0));
	}
	else if(!strcmp((char *)ptOpt->cOutfile[0],"")&&!strcmp((char *)ptOpt->cStanam,""))
	{
		strcpy((char *)ptOpt->cOutfile[0],(char *)ptOpt->cInfile);
		if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[0],'.'))) 
		{
			strcpy((char *)p,".obs");
		}
		else
		{
			strcat((char *)ptOpt->cOutfile[0],".obs");
		}
	}

	if(!strcmp((char *)ptOpt->cOutfile[1],"")&&strcmp((char *)ptOpt->cStanam,"")) /*导航文件*/
	{
		sprintf((char *)ptOpt->cOutfile[1],"%s%3d0.%02.0f%s",ptOpt->cStanam,tDoy.iDoy,fmod(tDoy.iYear,100.0),cExtNav);
	}
	else if(!strcmp((char *)ptOpt->cOutfile[1],"")&&!strcmp((char *)ptOpt->cStanam,""))
	{
		strcpy((char *)ptOpt->cOutfile[1],(char *)ptOpt->cInfile);
		if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[1],'.'))) 
		{
			strcpy((char *)p,".nav");
		}
		else 
		{
			strcat((char *)ptOpt->cOutfile[1],".nav");
		}
	}

	if(!strcmp((char *)ptOpt->cOutfile[2],"")&&strcmp((char *)ptOpt->cStanam,"")&&ptOpt->dRnxver<=2.99) /*BDS导航文件*/
	{
		sprintf((char *)ptOpt->cOutfile[2],"%s%3d0.%02.0fc",ptOpt->cStanam,tDoy.iDoy,fmod(tDoy.iYear,100.0));
	}
	else if(!strcmp((char *)ptOpt->cOutfile[2],"")&&!strcmp((char *)ptOpt->cStanam,"")&&ptOpt->dRnxver<=2.99)
	{
		strcpy((char *)ptOpt->cOutfile[2],(char *)ptOpt->cInfile);
		if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[2],'.')))
		{
			strcpy((char *)p,".cnav");
		}
		else 
		{
			strcat((char *)ptOpt->cOutfile[2],".cnav");
		}
	}
	else if (!strcmp((char *)ptOpt->cOutfile[2],"")&&ptOpt->dRnxver>2.99)
	{
		strcpy((char *)ptOpt->cOutfile[2],"");
	}

	if(!strcmp((char *)ptOpt->cOutfile[3],"")&&strcmp((char *)ptOpt->cStanam,"")&&ptOpt->dRnxver<=2.99) /*GLO导航文件*/
	{
		sprintf((char *)ptOpt->cOutfile[3],"%s%3d0.%02.0fg",ptOpt->cStanam,tDoy.iDoy,fmod(tDoy.iYear,100.0));
	}
	else if(!strcmp((char *)ptOpt->cOutfile[3],"")&&!strcmp((char *)ptOpt->cStanam,"")&&ptOpt->dRnxver<=2.99)
	{
		strcpy((char *)ptOpt->cOutfile[3],(char *)ptOpt->cInfile);
		if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[3],'.')))
		{
			strcpy((char *)p,".gnav");
		}
		else 
		{
			strcat((char *)ptOpt->cOutfile[3],".gnav");
		}
	}
	else if (!strcmp((char *)ptOpt->cOutfile[3],"")&&ptOpt->dRnxver>2.99)
	{
		strcpy((char *)ptOpt->cOutfile[3],"");
	}

	fprintf(stderr,"input file  : %s \n",ptOpt->cInfile);

	if (strcmp((char *)ptOpt->cOutfile[0],"")) 
	{
		fprintf(stderr,"->rinex obs : %s\n",ptOpt->cOutfile[0]); /*观测文件*/
	}
	if (strcmp((char *)ptOpt->cOutfile[1],""))
	{
		fprintf(stderr,"->rinex nav : %s\n",ptOpt->cOutfile[1]); /*ver 2 GPS导航文件，ver 3 GNSS导航文件*/
	}
	if (strcmp((char *)ptOpt->cOutfile[2],""))
	{
		fprintf(stderr,"->rinex cnav: %s\n",ptOpt->cOutfile[2]); /*BDS导航文件*/
	}
	if (strcmp((char *)ptOpt->cOutfile[3],""))
	{
		fprintf(stderr,"->rinex gnav: %s\n",ptOpt->cOutfile[3]); /*GLO导航文件*/
	}
	
	strcpy((char *)ptOpt->cOutfile[4],(char *)ptOpt->cInfile);/*周跳探测结果文件*/
	if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[4],'.'))) 
	{
		strcpy((char *)p,".CS");
	}
	else
	{
		strcat((char *)ptOpt->cOutfile[4],".CS");
	}	

	strcpy((char *)ptOpt->cOutfile[5],(char *)ptOpt->cInfile);/*多路径分析结果文件*/
	if ((p=(unsigned char *)strrchr((char *)ptOpt->cOutfile[5],'.'))) 
	{
		strcpy((char *)p,".MP");
	}
	else
	{
		strcat((char *)ptOpt->cOutfile[5],".MP");
	}	

	return 1;
}






