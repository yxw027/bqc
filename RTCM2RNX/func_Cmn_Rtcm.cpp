/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
* 
* 文件名称：func_Cmn.cpp
* 内容摘要：
* 其他说明：程序中卫星编号如下：系统     PRN    编号
*                               GPS      1-32   1 -32
*                               BDS      1-35   33-67
*                               GLONASS  1-24   68-91
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
*
*******************************************************************/

/******************************************************************************
*                          包含的头文件                                       *
******************************************************************************/
#include "def_convrnx.h"

/******************************************************************************
*                          静态数据变量                                       *
******************************************************************************/
static CHAR cObsCodes[][3]=               /*观测值编码字符*/
{       
    ""  ,"1C","1P","1W","1Y", "1M","1N","1S","1L","1E", /*  0- 9 */
    "1A","1B","1X","1Z","2C", "2D","2S","2L","2X","2P", /* 10-19 */
    "2W","2Y","2M","2N","5I", "5Q","5X","7I","7Q","7X", /* 20-29 */
    "6A","6B","6C","6X","6Z", "6S","6L","8L","8Q","8X", /* 30-39 */
    "2I","2Q","6I","6Q","3I", "3Q","3X",""              /* 40-49 */
};

static CHAR cObsFreqs[]=                 /*观测值编码对应的频率 
										    1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,7:L3*/
{ 
    0, 1, 1, 1, 1,  1, 1, 1, 1, 1, /*  0- 9 */
    1, 1, 1, 1, 2,  2, 2, 2, 2, 2, /* 10-19 */
    2, 2, 2, 2, 3,  3, 3, 2, 5, 5, /* 20-29 */
    4, 4, 4, 4, 4,  4, 4, 6, 6, 6, /* 30-39 */
    1, 2, 3, 4, 3,  3, 3, 0        /* 40-49 */
};

//static INT8U cCodePris[3][MAXFREQ][16]=   /*GNSS不同频率编码优先级表*/
//{  
//   /* L1,G1E1a   L2,G2,B1     L5,G3,E5a L6,LEX,B3 E5a,B2    E5a+b */
//    {"CPYWMNSL","PYWCMNDSLX","IQX"     ,""       ,""       ,""   }, /* GPS */
//    {"PC"      ,"PC"        ,"IQX"     ,""       ,""       ,""   }, /* GLO */
//    {""        ,"IQX"       ,""        ,"IQX"    ,"IQX"    ,""   }  /* BDS */
//};

static CHAR codepris[7][MAXFREQ][16]={  /* code priority table */

	/* L1/E1      L2/B1        L5/E5a/L3 L6/LEX/B3 E5b/B2    E5(a+b)  S */
	{"CPYWMNSL","PYWCMNDSLX","IQX"     ,""       ,""       ,""      ,""    }, /* GPS */
	{"PC"      ,"PC"        ,"IQX"     ,""       ,""       ,""      ,""    }, /* GLO */
	{"CABXZ"   ,""          ,"IQX"     ,"ABCXZ"  ,"IQX"    ,"IQX"   ,""    }, /* GAL */
	{"CSLXZ"   ,"SLX"       ,"IQX"     ,"SLX"    ,""       ,""      ,""    }, /* QZS */
	{"C"       ,""          ,"IQX"     ,""       ,""       ,""      ,""    }, /* SBS */
	{"IQX"     ,"IQX"       ,"IQX"     ,"IQX"    ,"IQX"    ,""      ,""    }, /* BDS */
	{""        ,""          ,"ABCX"    ,""       ,""       ,""      ,"ABCX"}  /* IRN */
};

static const INT32U tbl_CRC24Q[]=         /*RTCM3电文中CRC24Q表*/
{
    0x000000,0x864CFB,0x8AD50D,0x0C99F6,0x93E6E1,0x15AA1A,0x1933EC,0x9F7F17,
    0xA18139,0x27CDC2,0x2B5434,0xAD18CF,0x3267D8,0xB42B23,0xB8B2D5,0x3EFE2E,
    0xC54E89,0x430272,0x4F9B84,0xC9D77F,0x56A868,0xD0E493,0xDC7D65,0x5A319E,
    0x64CFB0,0xE2834B,0xEE1ABD,0x685646,0xF72951,0x7165AA,0x7DFC5C,0xFBB0A7,
    0x0CD1E9,0x8A9D12,0x8604E4,0x00481F,0x9F3708,0x197BF3,0x15E205,0x93AEFE,
    0xAD50D0,0x2B1C2B,0x2785DD,0xA1C926,0x3EB631,0xB8FACA,0xB4633C,0x322FC7,
    0xC99F60,0x4FD39B,0x434A6D,0xC50696,0x5A7981,0xDC357A,0xD0AC8C,0x56E077,
    0x681E59,0xEE52A2,0xE2CB54,0x6487AF,0xFBF8B8,0x7DB443,0x712DB5,0xF7614E,
    0x19A3D2,0x9FEF29,0x9376DF,0x153A24,0x8A4533,0x0C09C8,0x00903E,0x86DCC5,
    0xB822EB,0x3E6E10,0x32F7E6,0xB4BB1D,0x2BC40A,0xAD88F1,0xA11107,0x275DFC,
    0xDCED5B,0x5AA1A0,0x563856,0xD074AD,0x4F0BBA,0xC94741,0xC5DEB7,0x43924C,
    0x7D6C62,0xFB2099,0xF7B96F,0x71F594,0xEE8A83,0x68C678,0x645F8E,0xE21375,
    0x15723B,0x933EC0,0x9FA736,0x19EBCD,0x8694DA,0x00D821,0x0C41D7,0x8A0D2C,
    0xB4F302,0x32BFF9,0x3E260F,0xB86AF4,0x2715E3,0xA15918,0xADC0EE,0x2B8C15,
    0xD03CB2,0x567049,0x5AE9BF,0xDCA544,0x43DA53,0xC596A8,0xC90F5E,0x4F43A5,
    0x71BD8B,0xF7F170,0xFB6886,0x7D247D,0xE25B6A,0x641791,0x688E67,0xEEC29C,
    0x3347A4,0xB50B5F,0xB992A9,0x3FDE52,0xA0A145,0x26EDBE,0x2A7448,0xAC38B3,
    0x92C69D,0x148A66,0x181390,0x9E5F6B,0x01207C,0x876C87,0x8BF571,0x0DB98A,
    0xF6092D,0x7045D6,0x7CDC20,0xFA90DB,0x65EFCC,0xE3A337,0xEF3AC1,0x69763A,
    0x578814,0xD1C4EF,0xDD5D19,0x5B11E2,0xC46EF5,0x42220E,0x4EBBF8,0xC8F703,
    0x3F964D,0xB9DAB6,0xB54340,0x330FBB,0xAC70AC,0x2A3C57,0x26A5A1,0xA0E95A,
    0x9E1774,0x185B8F,0x14C279,0x928E82,0x0DF195,0x8BBD6E,0x872498,0x016863,
    0xFAD8C4,0x7C943F,0x700DC9,0xF64132,0x693E25,0xEF72DE,0xE3EB28,0x65A7D3,
    0x5B59FD,0xDD1506,0xD18CF0,0x57C00B,0xC8BF1C,0x4EF3E7,0x426A11,0xC426EA,
    0x2AE476,0xACA88D,0xA0317B,0x267D80,0xB90297,0x3F4E6C,0x33D79A,0xB59B61,
    0x8B654F,0x0D29B4,0x01B042,0x87FCB9,0x1883AE,0x9ECF55,0x9256A3,0x141A58,
    0xEFAAFF,0x69E604,0x657FF2,0xE33309,0x7C4C1E,0xFA00E5,0xF69913,0x70D5E8,
    0x4E2BC6,0xC8673D,0xC4FECB,0x42B230,0xDDCD27,0x5B81DC,0x57182A,0xD154D1,
    0x26359F,0xA07964,0xACE092,0x2AAC69,0xB5D37E,0x339F85,0x3F0673,0xB94A88,
    0x87B4A6,0x01F85D,0x0D61AB,0x8B2D50,0x145247,0x921EBC,0x9E874A,0x18CBB1,
    0xE37B16,0x6537ED,0x69AE1B,0xEFE2E0,0x709DF7,0xF6D10C,0xFA48FA,0x7C0401,
    0x42FA2F,0xC4B6D4,0xC82F22,0x4E63D9,0xD11CCE,0x575035,0x5BC9C3,0xDD8538
};

/******************************************************************************
*    函数名称:  Prn2Sat
*    功能描述:  根据卫星prn和卫星系统得到卫星编号
*    输入参数:  卫星系统，卫星PRN号
*    输出参数:  无
*    返 回 值:  卫星编号
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U Prn2Sat(INT32U iSys,INT32U iPrn)
{
	switch (iSys)
	{
	    case SYS_GPS:  /*GPS系统*/
			if ((iPrn>=MINPRNGPS)&&(iPrn<=MAXPRNGPS))
			{
                return iPrn-MINPRNGPS+1;
			}
			else
			{
				return 0;
			}
	    case SYS_BDS:  /*BDS系统*/
			if ((iPrn>=MINPRNBDS)&&(iPrn<=MAXPRNBDS))
			{
                return NSATGPS+iPrn-MINPRNBDS+1;
			}
			else
			{
				return 0;
			}
		case SYS_GLO:  /*GLONASS系统*/
			if ((iPrn>=MINPRNGLO)&&(iPrn<=MAXPRNGLO))
			{
                return NSATGPS+NSATBDS+iPrn-MINPRNGLO+1;
			}
			else
			{
				return 0;
			}
		default: return 0;
	}
}

/******************************************************************************
*    函数名称:  Sat2Sys
*    功能描述:  根据卫星编号得到卫星系统及卫星PRN
*    输入参数:  卫星编号，卫星PRN号
*    输出参数:  卫星PRN号
*    返 回 值:  卫星系统
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U Sat2Sys(INT32U iSat,INT32U *piPrn)
{
	INT32U iSys=SYS_NONE;

	if (iSat>MAXSAT)
	{
		iSat=0;
	}
	else if (iSat<=NSATGPS)  /*GPS系统*/
	{
		iSys=SYS_GPS;
		iSat=iSat+MINPRNGPS-1;
    }
	else if (iSat<=(NSATGPS+NSATBDS))  /*BDS系统*/
	{
		iSys=SYS_BDS;
		iSat=iSat-NSATGPS+MINPRNBDS-1;
	}
	else if (iSat<=(NSATGPS+NSATBDS+NSATGLO))  /*GLONASS系统*/
	{
		iSys=SYS_GLO;
		iSat=iSat-NSATGPS-NSATBDS+MINPRNGLO-1;
	}
	else
	{
		iSat=0;
	}

	if (piPrn)
	{
		*piPrn=iSat;
	}

	return iSys;
}

/******************************************************************************
*    函数名称:  Code2Obs
*    功能描述:  观测值编码转换成观测值编码字符
*    输入参数:  观测值编码，观测值频率
*    输出参数:  观测值频率
*    返 回 值:  观测值编码字符
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern CHAR *Code2Obs(INT8U iCode,INT32U *piFreq)
{
	if (iCode<=CODE_NONE||iCode>MAXCODE)
	{
		return cObsCodes[0];
	}
	else
	{
	    if (piFreq)
	    {
		    *piFreq=cObsFreqs[iCode]; /*观测值编码对应的卫星频率*/	
	    }
		return cObsCodes[iCode];
	}
}

/******************************************************************************
*    函数名称:  GetBitu
*    功能描述:  提取位 无符号型
*    输入参数:  缓存消息，提取位起始位置，提取位长度
*    输出参数:  无
*    返 回 值:  提取得到的无符号型整数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U GetBitu(const INT8U *pcBuff,INT32U iPos,INT32U iLen)
{
    INT32U iBits=0;
	INT32U i;

	for (i=iPos;i<iPos+iLen;i++)
	{
		iBits=(iBits<<1)+((pcBuff[i/8]>>(7-i%8))&1u);
	}

	return iBits;
}

/******************************************************************************
*    函数名称:  GetBits
*    功能描述:  提取位 有符号型
*    输入参数:  缓存消息，提取位起始位置，提取位长度
*    输出参数:  无
*    返 回 值:  提取得到的有符号型整数
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S GetBits(const INT8U *pcBuff,INT32U iPos,INT32U iLen)
{
	INT32U iBits;

	iBits=GetBitu(pcBuff,iPos,iLen);
    if (iLen<=0||iLen>=32||!(iBits&(1u<<(iLen-1))))
	{
		return (int)iBits;
	}
	else
	{
		return (int)(iBits|(~0u<<iLen));
	}
}

/******************************************************************************
*    函数名称:  SetBitu
*    功能描述:  生成无符号型位
*    输入参数:  缓存消息，存储位起始位置，生成位长度，生成位的数据
*    输出参数:  缓存消息
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void SetBitu(INT8U *pcBuff,INT32U iPos,INT32U iLen,INT32U iData)
{
	INT32U iMask=1u<<(iLen-1);
	INT32U i;
	
	if ((iLen>0)&&(iLen<=32))
	{
		for (i=iPos;i<iPos+iLen;i++,iMask>>=1)
		{
			if (iData&iMask)
			{
				pcBuff[i/8]|=1u<<(7-i%8);
			}
			else
			{
				pcBuff[i/8]&=~(1u<<(7-i%8));
			}
		}
	}
	else
	{
		return;
	}
}

/******************************************************************************
*    函数名称:  SetBits
*    功能描述:  生成有符号型位
*    输入参数:  缓存消息，存储位起始位置，生成位长度，生成位的数据
*    输出参数:  缓存消息
*    返 回 值:  无
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern void SetBits(INT8U *pcBuff,INT32U iPos,INT32U iLen,INT32S iData)
{
	if (iData<0)
	{
		iData|=1<<(iLen-1);
	}
	else
	{
		iData&=~(1<<(iLen-1));
	}

	SetBitu(pcBuff,iPos,iLen,(unsigned int)iData);
}

/******************************************************************************
*    函数名称:  CRC24Q
*    功能描述:  crc-24q检验
*    输入参数:  缓存消息，消息长度
*    输出参数:  无
*    返 回 值:  CRC检验码
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U CRC24Q(INT8U *pcBuff,INT32U iLen)
{
	INT32U iCrc=0;
	INT32U i;

    for (i=0;i<iLen;i++)
	{
		iCrc=((iCrc<<8)&0xFFFFFF)^tbl_CRC24Q[(iCrc>>16)^pcBuff[i]];
	}
    
    return iCrc;
}


/***************************RTCM2.3 common function*****************************************/

/******************************************************************************
*    函数名称:  Decode_Word
*    功能描述:  解码30bit电文，并进行奇偶校验
*    输入参数:  一个字的电文，缓存消息
*    输出参数:  无
*    返 回 值: 奇偶校验状态 (1:ok,0:校验错误)
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U Decode_Word(INT32U iWord, INT8U *pcData)
{
	const INT32U iHamming[]=
	{
		0xBB1F3480,0x5D8F9A40,0xAEC7CD00,0x5763E680,0x6BB1F340,0x8B7A89C0
	};

	INT32U iParity=0,iW;
	INT32U i;

	if (iWord&0x40000000) 
	{
		iWord^=0x3FFFFFC0;
	}

	for (i=0;i<6;i++) 
	{
		iParity<<=1;
		for (iW=(iWord&iHamming[i])>>6;iW;iW>>=1) 
		{
			iParity^=iW&1;
		}
	}

	if (iParity!=(iWord&0x3F)) 
	{
		return 0;
	}

	for (i=0;i<3;i++) 
	{
		pcData[i]=(unsigned char)(iWord>>(22-i*8));
	}

	return 1;
}

/******************************************************************************
*    函数名称:  Obs2Code
*    功能描述:  观测值编码字符转换成观测值编码
*    输入参数:  观测值编码字符，观测值频率
*    输出参数:  观测值频率
*    返 回 值:  观测值编码
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT8U Obs2Code(const CHAR *obs, INT32U *freq)
{
    INT32U i;
    if (freq) *freq=0;
    for (i=1;*cObsCodes[i];i++) {
        if (strcmp(cObsCodes[i],obs)) continue;
        if (freq) *freq=cObsFreqs[i];
        return (INT8U)i;
    }
    return CODE_NONE;
}

/******************************************************************************
*    函数名称:  SatWavelen
*    功能描述:  计算各频率的波长
*    输入参数:  卫星号,频率下标，星历结构体变量
*    输出参数:  星历结构体变量
*    返 回 值:  频率波长
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern FP64 SatWavelen(INT32U sat, INT32U frq, const T_NAV *nav)
{
    const FP64 freq_glo[]={FREQ1_GLO,FREQ2_GLO};
    const FP64 dfrq_glo[]={DFRQ1_GLO,DFRQ2_GLO};
    INT32S i,sys=Sat2Sys(sat,NULL);
    
    if (sys==SYS_GLO) {
        if (0<=frq&&frq<=1) {
            for (i=0;i<nav->iNg;i++) {
                if (nav->ptGeph[i].iSat!=sat) continue;
                return CLIGHT/(freq_glo[frq]+dfrq_glo[frq]*nav->ptGeph[i].iFrq);
            }
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
        if      (frq==0) return CLIGHT/FREQ1_GPS; /* L1/E1 */
        else if (frq==1) return CLIGHT/FREQ2_GPS; /* L2 */
        else if (frq==2) return CLIGHT/FREQ5_GPS; /* L5/E5a */
        else if (frq==3) return CLIGHT/FREQ6; /* L6/LEX */
        else if (frq==4) return CLIGHT/FREQ7; /* E5b */
        else if (frq==5) return CLIGHT/FREQ8; /* E5a+b */
        else if (frq==6) return CLIGHT/FREQ9; /* S */
    }
    return 0.0;
}

/******************************************************************************
*    函数名称:  GetCodepri
*    功能描述:  获得观测值编码的优先级
*    输入参数:  卫星系统,观测值编码，观测值编码选项
*    输出参数:  观测值编码选项
*    返 回 值:  观测值编码优先级
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
/* get code priority -----------------------------------------------------------
* get code priority for multiple codes in a frequency
* args   : int    sys     I     system (SYS_???)
*          unsigned char code I obs code (CODE_???)
*          char   *opt    I     code options (NULL:no option)
* return : priority (15:highest-1:lowest,0:error)
*-----------------------------------------------------------------------------*/
extern INT32U GetCodepri(INT32U sys, INT8U code, const CHAR *opt)
{
    const CHAR *p,*optstr;
    CHAR *obs,str[8]="";
    INT32U i,j;
    
    switch (sys) {
        case SYS_GPS: i=0; optstr="-GL%2s"; break;
        case SYS_GLO: i=1; optstr="-RL%2s"; break;
        //case SYS_GAL: i=2; optstr="-EL%2s"; break;
        //case SYS_QZS: i=3; optstr="-JL%2s"; break;
        //case SYS_SBS: i=4; optstr="-SL%2s"; break;
        case SYS_BDS: i=5; optstr="-CL%2s"; break;
        //case SYS_IRN: i=6; optstr="-IL%2s"; break;
        default: return 0;
    }
    obs=Code2Obs(code,&j);
    
    /* parse code options */
    for (p=opt;p&&(p=strchr(p,'-'));p++) {
        if (sscanf(p,optstr,str)<1||str[0]!=obs[0]) continue;
        return str[1]==obs[1]?15:0;
    }
    /* search code priority */
    return (p=strchr(codepris[i][j-1],obs[1]))?14-(int)(p-codepris[i][j-1]):0;
}
