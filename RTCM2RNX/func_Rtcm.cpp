/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
*
* 文件名称：func_Rtcm.cpp
* 内容摘要：RTCM控制变量初始化
*           RTCM 3.1电文编码和解码
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
*    函数名称:  Init_RTCM
*    功能描述:  初始化RTCM控制变量
*    输入参数:  RTCM结构体变量
*    输出参数:  RTCM结构体变量
*    返 回 值:  初始化标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32U Init_RTCM(T_RTCM *ptRtcm)
{
    T_OBSSAT tData0={{0}};
    T_EPH    tEph0 ={0};
    T_GLOEPH tGeph0={0};
    INT32U i;

    ptRtcm->outtype=0;
    ptRtcm->cMsgtype[0]='\0';
    for (i=0;i<6;i++)
    {
        ptRtcm->cMsmtype[i][0]='\0';
    }

    /*对卫星观测值和导航星历变量初始化*/
    for (i=0;i<MAXOBS; i++)
    {
        ptRtcm->tObs.ptData[i]=tData0;
    }
    for (i=0;i<MAXSAT; i++)
    {
        ptRtcm->tNav.ptEph [i]=tEph0;
    }
    for (i=0;i<NSATGLO;i++)
    {
        ptRtcm->tNav.ptGeph[i]=tGeph0;
    }
    return 1;
}

/******************************************************************************
*    函数名称:  Input_RTCM2f
*    功能描述:  文件中RTCM2电文数据
*    输入参数:  RTCM结构体变量，文件指针
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
INT32U iC=0;
extern INT32S Input_RTCM2f(T_RTCM *ptRtcm, FILE *fp)
{
    INT32S iData=0,iMark;
    INT32U i;

    for (i=0;i<4096;i++)
    {
        iC=iC+1;
        /*逐个字符读取电文*/
        if ((iData=fgetc(fp))==EOF)
        {
            return -2;
        }
        if ((iMark=Input_RTCM2(ptRtcm,(unsigned char)iData)))
        {
            return iMark;
        }
    }

    return 0;
}

/******************************************************************************
*    函数名称:  Input_RTCM2
*    功能描述:  实时流中RTCM2电文数据
*    输入参数:  RTCM结构体变量，电文数据
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    注    意： 使用前需给T_RTCM结构体赋时间初值
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S Input_RTCM2(T_RTCM *ptRtcm,INT8U cData)
{
    INT8U iPreAmb;
    INT32U i;

    // ignore if upper 2bit != 01
    if ((cData&0xC0)!=0x40)
    {
        return 0;
    }

    //解码“6/8”格式
    for (i=0;i<6;i++,cData>>=1)
    {
        ptRtcm->iWord=(ptRtcm->iWord<<1)+(cData&1);

        //帧同步
        if (ptRtcm->iNbyte==0)
        {
            iPreAmb=(unsigned char)(ptRtcm->iWord>>22);

            //解码引导字
            if (ptRtcm->iWord&0x40000000)
            {
                iPreAmb^=0xFF;
            }
            if (iPreAmb!=RTCM2PREAMB)
            {
                continue;
            }

            //奇偶校验
            if (!Decode_Word(ptRtcm->iWord,ptRtcm->cBuff))
            {
                ptRtcm->iWord&=0x3;
                continue;
            }
            ptRtcm->iNbyte=3; ptRtcm->iNbit=0;
            continue;
        }

        ptRtcm->iNbit=ptRtcm->iNbit+1;
        if (ptRtcm->iNbit<30)
        {
            continue;
        }
        else
        {
            ptRtcm->iNbit=0;
        }

        //奇偶校验
        if (!Decode_Word(ptRtcm->iWord,ptRtcm->cBuff+ptRtcm->iNbyte))
        {
            ptRtcm->iNbyte=0;
            ptRtcm->iWord&=0x3;
            continue;
        }
        ptRtcm->iNbyte=ptRtcm->iNbyte+3;

        //电文长度
        if (ptRtcm->iNbyte==6)
        {
            ptRtcm->iLen=(ptRtcm->cBuff[5]>>3)*3+6;
        }

        if (ptRtcm->iNbyte<ptRtcm->iLen)
        {
            continue;
        }

        ptRtcm->iNbyte=0;
        ptRtcm->iWord&=0x3;

        //解析RTCM3电文
        return Decode_RTCM2(ptRtcm);
    }

    return 0;
}

/******************************************************************************
*    函数名称:  Input_RTCM3f
*    功能描述:  文件中RTCM3电文数据
*    输入参数:  RTCM结构体变量，文件指针
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S Input_RTCM3f(T_RTCM *ptRtcm, FILE *fp)
{
    INT32S iData=0,iMark;
    INT32U i;

    for (i=0;i<4096;i++)
    {
        /*逐个字符读取电文*/
        if ((iData=fgetc(fp))==EOF)
        {
            return -2;
        }
        if ((iMark=Input_RTCM3(ptRtcm,(unsigned char)iData)))
        {
            return iMark;
        }
    }

    return 0;
}

/******************************************************************************
*    函数名称:  Input_RTCM3
*    功能描述:  实时流中RTCM3电文数据
*    输入参数:  RTCM结构体变量，电文数据
*    输出参数:  RTCM结构体变量
*    返 回 值:  解码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*
******************************************************************************/
extern INT32S Input_RTCM3(T_RTCM *ptRtcm,INT8U cData)
{
    if (ptRtcm->iNbyte==0)
    {
        if (cData!=RTCM3PREAMB) /*判断是否是RTCM3的引言*/
        {
            return 0;
        }
        else
        {
            ptRtcm->cBuff[0]=cData;
            ptRtcm->iNbyte=1;
            return 0;
        }
    }

    ptRtcm->cBuff[ptRtcm->iNbyte]=cData;
    ptRtcm->iNbyte=ptRtcm->iNbyte+1;

    if (ptRtcm->iNbyte==3) /*电文中消息长度*/
    {
        ptRtcm->iLen=GetBitu(ptRtcm->cBuff,14,10);
    }
    if (ptRtcm->iNbyte<3||ptRtcm->iNbyte<ptRtcm->iLen+6)
    {
        return 0;
    }

    ptRtcm->iNbyte=0;

    if (CRC24Q(ptRtcm->cBuff,ptRtcm->iLen+3)!=GetBitu(ptRtcm->cBuff,ptRtcm->iLen*8+24,24)) /*CRC检验*/
    {
        return 0;
    }

    return Decode_RTCM3(ptRtcm); /*解析RTCM3电文*/
}

/******************************************************************************
*    函数名称:  Output_RTCM3
*    功能描述:  生成RTCM3电文
*    输入参数:  RTCM结构体变量，电文类型，同步标识
*    输出参数:  RTCM结构体变量
*    返 回 值:  编码标识
*    修改日期          版本号          修改人      修改内容
* -----------------------------------------------------------------------------
*,FILE *fp
******************************************************************************/
extern INT32U Output_RTCM3(T_RTCM *ptRtcm,INT32U iType,INT32U iSync)
{
    INT32U iCrc;
    INT32U i=0;

    ptRtcm->iLen=ptRtcm->iNbit=ptRtcm->iNbyte=0;

    /*生成导言，预留和消息长度*/
    SetBitu(ptRtcm->cBuff, 0, 8,RTCM3PREAMB);
    SetBitu(ptRtcm->cBuff, 8, 6,0          );
    SetBitu(ptRtcm->cBuff,14,10,0          );
    ptRtcm->iNbit=24;

    /*编码电文内容*/
    if (!(Encode_RTCM3(ptRtcm,iType,iSync)))
    {
        return 0;
    }

    /*电文内容的位数需是8的倍数*/
    i=8-ptRtcm->iNbit%8;
    if (i!=8)
    {
        SetBitu(ptRtcm->cBuff,ptRtcm->iNbit,i,0);
        ptRtcm->iNbit=ptRtcm->iNbit+i;
    }

    /*更新消息长度*/
    ptRtcm->iLen=ptRtcm->iNbit/8-3;
    if (ptRtcm->iLen>=1024)
    {
        fprintf(stderr,"generate RTCM3 message length error len=%d",ptRtcm->iLen);
        ptRtcm->iLen=ptRtcm->iNbit=0;
        return 0;
    }

    SetBitu(ptRtcm->cBuff,14,10,ptRtcm->iLen);

    /*CRC校验码*/
    iCrc=CRC24Q(ptRtcm->cBuff,ptRtcm->iLen+3);
    SetBitu(ptRtcm->cBuff,ptRtcm->iNbit,24,iCrc);

    ptRtcm->iNbyte=ptRtcm->iLen+6;

    /*if (!fp) return 0;

    fwrite(ptRtcm->cBuff,1,ptRtcm->iNbyte,fp);*/

    return 1;
}






