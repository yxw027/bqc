/********************************************************************
* 版权所有（C）2015，广州海格通信集团股份有限公司
*
* 文件名称：def_convrnx.h
* 内容摘要：常量、静态数据变量、数据类型定义，函数原型声明
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

#ifndef DEF_CONVRNX_H
#define DEF_CONVRNX_H

/******************************************************************************
*                               包含的头文件                                  *
******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//#define OUTPUT_FILE

typedef unsigned char    BOOLEAN;
typedef          char    CHAR;      /* 字符类型 */
typedef unsigned char    INT8U;
typedef   signed char    INT8S;
typedef unsigned short   INT16U;
typedef   signed short   INT16S;
typedef unsigned int     INT32U;
typedef   signed int     INT32S;
typedef unsigned long    INT40U;
typedef   signed long    INT40S;
typedef          float   FP32;        /*single precision floating point*/
typedef          double  FP64;        /*double precision floating point*/

/******************************************************************************
*                               宏定义                                        *
******************************************************************************/
#define PI           (4.0*atan(1.0))  /*pi*/
#define CLIGHT       (299792458.0)    /*speed of light (m/s)*/
#define SC2RAD       (4.0*atan(1.0))  /*semi-circle to radian (IS-GPS)*/
#define PRUNIT_GPS   (299792.458)     /*rtcm ver.3 unit of gps pseudorange (m)*/
#define PRUNIT_GLO   (599584.916)     /*rtcm ver.3 unit of glonass pseudorange (m)*/
#define PRUNIT_BDS   (299792.458)     /*rtcm ver.3 unit of bds pseudorange (m)*/

#define GPST0        (2444244.5)      /*origin of GPS time in jd, 00:00:00, Jan. 6, 1980 UTC*/
#define BDT0         (2453736.5)      /*origin of BDS time in jd, 00:00:00, Jan. 1, 2006 UTC*/
#define BDT2GPST     (14)             /*offset between BDS time and GPS time, GPST=BDT+14s*/
#define LEAPS        (18)             /*2017年GPS时和UTC之间的跳秒*/
#define D2S          (86400)          /*天内秒*/

#define STRFMT_RTCM2 (0)              /*stream format: RTCM 2*/
#define STRFMT_RTCM3 (1)              /*stream format: RTCM 3*/
#define STRFMT_RINEX (2)              /*stream format: RINEX*/

#define RTCM2PREAMB  (0x66)           /*rtcm 2 frame preamble*/
#define RTCM3PREAMB  (0xD3)           /*rtcm 3 frame preamble*/

#define FREQ1_GPS    (1.57542E9)      /*L1 frequency (Hz)*/
#define FREQ2_GPS    (1.22760E9)      /*L2 frequency (Hz)*/
#define FREQ5_GPS    (1.17645E9)      /*L5 frequency (Hz)*/
#define FREQ6       1.27875E9           /* E6/LEX frequency (Hz) */
#define FREQ7       1.20714E9           /* E5b    frequency (Hz) */
#define FREQ8       1.191795E9          /* E5a+b  frequency (Hz) */
#define FREQ9       2.492028E9          /* S      frequency (Hz) */
#define FREQ1_GLO    (1.60200E9)      /*GLONASS G1 base frequency (Hz)*/
#define DFRQ1_GLO    (0.56250E6)      /*GLONASS G1 bias frequency (Hz)*/
#define FREQ2_GLO    (1.24600E9)      /*GLONASS G2 base frequency (Hz)*/
#define DFRQ2_GLO    (0.43750E6)      /*GLONASS G2 bias frequency (Hz)*/
#define FREQ3_GLO    (1.202025E9)     /*GLONASS G3 frequency (Hz)*/
#define FREQ2_BDS    (1.561098E9)     /*BDS B1 frequency (Hz)*/
#define FREQ7_BDS    (1.20714E9)      /*BDS B2 frequency (Hz)*/
#define FREQ6_BDS    (1.26852E9)      /*BDS B3 frequency (Hz)*/

#define SYS_NONE     (0x00)           /*navigation system: none*/
#define SYS_GPS      (0x01)           /*navigation system: GPS        1*/
//#define SYS_SBS      (0x02)                /* navigation system: SBAS */
#define SYS_GLO      (0x04)           /*navigation system: GLONASS    4*/
//#define SYS_GAL      (0x08)                /* navigation system: Galileo */
//#define SYS_QZS      (0x10)                /* navigation system: QZSS */
#define SYS_BDS      (0x20)           /*navigation system: BDS       32*/
//#define SYS_IRN      (0x40)                /* navigation system: IRNS */
//#define SYS_LEO      (0x80)                /* navigation system: LEO */

#define TSYS_GPS     (0)              /*time system: GPS time*/
#define TSYS_UTC     (1)              /*time system: UTC*/
#define TSYS_BDS     (5)              /*time system: BDS time*/

#define MINFREQ_GLO -7                  /* min frequency number glonass */
#define MAXFREQ_GLO 13                  /* max frequency number glonass */

#ifndef NFREQ
#define NFREQ        (3)              /*number of carrier frequencies*/
#endif
#define NFREQ_GLO    (2)              /*number of carrier frequencies of GLONASS*/

#ifndef NEXOBS
#define NEXOBS       (0)              /*number of extended obs codes*/
#endif

#define MINPRNGPS    (1)              /*min satellite PRN number of GPS*/
#define MAXPRNGPS    (32)             /*max satellite PRN number of GPS*/
#define NSATGPS      (32)             /*number of GPS satellites*/

#define MINPRNGLO    (1)              /*min satellite PRN number of GLONASS*/
#define MAXPRNGLO    (24)             /*max satellite PRN number of GLONASS*/
#define NSATGLO      (24)             /*number of GLONASS satellites*/

#define MINPRNBDS    (1)              /*min satellite PRN number of BDS*/
#define MAXPRNBDS    (35)             /*max satellite PRN number of BDS*/
#define NSATBDS      (35)             /*number of BDS satellites*/

#define MAXSAT       (NSATGPS+NSATGLO+NSATBDS)

#define MAXFREQ      (7)              /*max NFREQ*/
#define MAXOBS       (128)             /*max number of obs satellites in an epoch*/
#define MAXOBSTYPE   (64)             /*max number of obs type in RINEX*/
#define MAXANT       (64)             /*max length of station name/antenna type*/
#define MAXCOMMENT   (10)             /*max number of RINEX comments*/
#define MAXINFILE    (8)              /*max number of input files*/
#define MAXOUTFILE   (8)              /*max number of output files*/


#define OBSTYPE_PR   (0x01)           /*观测值类型: pseudorange*/
#define OBSTYPE_CP   (0x02)           /*观测值类型: carrier-phase*/
#define OBSTYPE_DOP  (0x04)           /*观测值类型: doppler-freq*/
#define OBSTYPE_SNR  (0x08)           /*观测值类型: SNR*/
#define OBSTYPE_ALL  (0xFF)           /*观测值类型: all*/

#define CODE_NONE    (0)              /*obs code: none or unknown*/
#define CODE_L1C     (1)              /*obs code: L1C/A,G1C/A,E1C    (GPS,GLO,GLA,QZS,SBS)*/
#define CODE_L1P     (2)              /*obs code: L1P,G1P            (GPS,GLO)*/
#define CODE_L1W     (3)              /*obs code: L1 Z-track         (GPS)*/
#define CODE_L1Y     (4)              /*obs code: L1Y                (GPS)*/
#define CODE_L1M     (5)              /*obs code: L1M                (GPS)*/
#define CODE_L1N     (6)              /*obs code: L1codeless         (GPS)*/
#define CODE_L1S     (7)              /*obs code: L1C(D)             (GPS,QZS)*/
#define CODE_L1L     (8)              /*obs code: L1C(P)             (GPS,QZS)*/
#define CODE_L1E     (9)              /*obs code: L1-SAIF            (QZS)*/
#define CODE_L1A     (10)             /*obs code: E1A                (GAL)*/
#define CODE_L1B     (11)             /*obs code: E1B                (GAL)*/
#define CODE_L1X     (12)             /*obs code: E1B+C,L1C(D+P)     (GAL,QZS)*/
#define CODE_L1Z     (13)             /*obs code: E1A+B+C,L1-SAIF    (GAL,QZS)*/
#define CODE_L2C     (14)             /*obs code: L2C/A,G1C/A        (GPS,GLO)*/
#define CODE_L2D     (15)             /*obs code: L2 L1C/A-(P2-P1)   (GPS)*/
#define CODE_L2S     (16)             /*obs code: L2C(M)             (GPS,QZS)*/
#define CODE_L2L     (17)             /*obs code: L2C(L)             (GPS,QZS)*/
#define CODE_L2X     (18)             /*obs code: L2C(M+L),B1I+Q     (GPS,QZS,BDS)*/
#define CODE_L2P     (19)             /*obs code: L2P,G2P            (GPS,GLO)*/
#define CODE_L2W     (20)             /*obs code: L2 Z-track         (GPS)*/
#define CODE_L2Y     (21)             /*obs code: L2Y                (GPS)*/
#define CODE_L2M     (22)             /*obs code: L2M                (GPS)*/
#define CODE_L2N     (23)             /*obs code: L2codeless         (GPS)*/
#define CODE_L5I     (24)             /*obs code: L5/E5aI            (GPS,GAL,QZS,SBS)*/
#define CODE_L5Q     (25)             /*obs code: L5/E5aQ            (GPS,GAL,QZS,SBS)*/
#define CODE_L5X     (26)             /*obs code: L5/E5aI+Q          (GPS,GAL,QZS,SBS)*/
#define CODE_L7I     (27)             /*obs code: E5bI,B2I           (GAL,BDS)*/
#define CODE_L7Q     (28)             /*obs code: E5bQ,B2Q           (GAL,BDS)*/
#define CODE_L7X     (29)             /*obs code: E5bI+Q,B2I+Q       (GAL,BDS)*/
#define CODE_L6A     (30)             /*obs code: E6A                (GAL)*/
#define CODE_L6B     (31)             /*obs code: E6B                (GAL)*/
#define CODE_L6C     (32)             /*obs code: E6C                (GAL)*/
#define CODE_L6X     (33)             /*obs code: E6B+C,LEXS+L,B3I+Q (GAL,QZS,BDS)*/
#define CODE_L6Z     (34)             /*obs code: E6A+B+C            (GAL)*/
#define CODE_L6S     (35)             /*obs code: LEXS               (QZS)*/
#define CODE_L6L     (36)             /*obs code: LEXL               (QZS)*/
#define CODE_L8I     (37)             /*obs code: E5(a+b)I           (GAL)*/
#define CODE_L8Q     (38)             /*obs code: E5(a+b)Q           (GAL)*/
#define CODE_L8X     (39)             /*obs code: E5(a+b)I+Q         (GAL)*/
#define CODE_L2I     (40)             /*obs code: B1I                (BDS)*/
#define CODE_L2Q     (41)             /*obs code: B1Q                (BDS)*/
#define CODE_L6I     (42)             /*obs code: B3I                (BDS)*/
#define CODE_L6Q     (43)             /*obs code: B3Q                (BDS)*/
#define CODE_L3I     (44)             /*obs code: G3I                (GLO)*/
#define CODE_L3Q     (45)             /*obs code: G3Q                (GLO)*/
#define CODE_L3X     (46)             /*obs code: G3I+Q              (GLO)*/
#define MAXCODE      (46)             /*max number of obs code*/

#define P2_5         (0.03125)                /*2^-5 */
#define P2_6         (0.015625)               /*2^-6 */
#define P2_10        (0.0009765625)           /*2^-10*/
#define P2_11        (4.882812500000000E-04)  /*2^-11*/
#define P2_15        (3.051757812500000E-05)  /*2^-15*/
#define P2_17        (7.629394531250000E-06)  /*2^-17*/
#define P2_19        (1.907348632812500E-06)  /*2^-19*/
#define P2_20        (9.536743164062500E-07)  /*2^-20*/
#define P2_21        (4.768371582031250E-07)  /*2^-21*/
#define P2_23        (1.192092895507810E-07)  /*2^-23*/
#define P2_24        (5.960464477539063E-08)  /*2^-24*/
#define P2_27        (7.450580596923828E-09)  /*2^-27*/
#define P2_29        (1.862645149230957E-09)  /*2^-29*/
#define P2_30        (9.313225746154785E-10)  /*2^-30*/
#define P2_31        (4.656612873077393E-10)  /*2^-31*/
#define P2_32        (2.328306436538696E-10)  /*2^-32*/
#define P2_33        (1.164153218269348E-10)  /*2^-33*/
#define P2_34        (5.820766091346740E-11)  /*2^-34*/
#define P2_35        (2.910383045673370E-11)  /*2^-35*/
#define P2_38        (3.637978807091710E-12)  /*2^-38*/
#define P2_39        (1.818989403545856E-12)  /*2^-39*/
#define P2_40        (9.094947017729280E-13)  /*2^-40*/
#define P2_43        (1.136868377216160E-13)  /*2^-43*/
#define P2_46        (1.421085471520200E-14)  /*2^-46*/
#define P2_48        (3.552713678800501E-15)  /*2^-48*/
#define P2_50        (8.881784197001252E-16)  /*2^-50*/
#define P2_55        (2.775557561562891E-17)  /*2^-55*/
#define P2_59        (1.734723475976810E-18)  /*2^-59*/
#define P2_66       1.355252715606880E-20 /* 2^-66 */
#define RANGE_MS    (CLIGHT*0.001)      /* range in 1 ms */

#define COORD_DIM           (3)                        /*坐标维数*/
#define EPSILON             (1e-13)                    /*最小的正数*/
#define Freq1      (0)  //GPS L1
#define Freq2      (1)   //GPS L2
#define Freq3      (2)   //GPS L5
#define SlidWinNum      (600)   //多路径移动窗口历元数
#define MAXDIM   (MAXSAT*MAXSAT)
/******************************************************************************
*                              静态数据变量                                   *
******************************************************************************/

static FP64 dLam_Carr[]=                  /*载波波长(m) */
{
    CLIGHT/FREQ1_GPS,CLIGHT/FREQ2_GPS,CLIGHT/FREQ5_GPS,
    CLIGHT/FREQ2_BDS,CLIGHT/FREQ7_BDS,CLIGHT/FREQ6_BDS
};

static INT32U iNavSys[]=                 /*导航系统*/
{
    SYS_GPS,SYS_GLO,SYS_BDS,0
};

static const INT8U cSysCodes[]="GRC";    /*卫星导航系统编码*/

static FP64 dFreqBDS[NFREQ]={FREQ2_BDS,FREQ7_BDS,FREQ6_BDS};//频率
static FP64 dFreqGPS[NFREQ]={FREQ1_GPS,FREQ2_GPS,FREQ5_GPS};//频率

static FP64 dLamBDS[NFREQ]={CLIGHT/FREQ2_BDS,CLIGHT/FREQ7_BDS,CLIGHT/FREQ6_BDS};//波长
static FP64 dLamGPS[NFREQ]={CLIGHT/FREQ1_GPS,CLIGHT/FREQ2_GPS,CLIGHT/FREQ5_GPS};//波长

/******************************************************************************
*                               数据类型定义                                  *
******************************************************************************/
typedef struct                                /*GPS周+秒*/
{
    INT32U     iWeek;
    FP64       dSow;
}T_WEEKSEC;

typedef struct                                /*通用时*/
{
    INT32U     iYear;
    INT32U     iMon;
    INT32U     iDay;
    INT32U     iHor;
    INT32U     iMin;
    FP64       dSec;
}T_YMD;

typedef struct                                 /*年积日*/
{
    INT32U     iYear;
    INT32U     iDoy;
}T_YEARDAY;

typedef struct                                 /*观测文件的文件头*/
{
    INT8U      cProg      [MAXANT];            /*创建本数据文件所采用的程序名称*/
    INT8U      cRunBy     [MAXANT];            /*创建本数据文件单位的名称*/
    INT8U      cDate      [MAXANT];            /*创建本数据文件的日期*/
    INT8U      cComment   [MAXCOMMENT][MAXANT];/*注释*/
    INT8U      cMarker    [MAXANT];            /*点名*/
    INT8U      cMarkerno  [MAXANT];            /*点号*/
    INT8U      cMarkertype[MAXANT];            /*点类型 (ver.3)*/
    INT8U      cObser     [2][MAXANT];         /*观测者姓名/观测单位名称*/
    INT8U      cRec       [3][MAXANT];         /*接收机序列号/类型/版本号*/
    INT8U      cAnt       [2][MAXANT];         /*天线序列号/类型*/
    FP64       dAppPos    [3];                 /*测站点近似坐标*/
    FP64       dAntDel    [3];                 /*天线H/E/N*/
    INT32U     iAntSetup;                      /*天线创建ID*/
    INT32U     iItrf;                          /*ITRF实现的年份*/
}T_STA;

typedef struct                                 /*卫星观测值*/
{
        T_WEEKSEC  tObsTime;                       /*卫星观测时间 (GPST)*/
        INT32U     iSat;                           /*卫星编号*/
        INT32S     iFrq;                           /*卫星频率数*/
        INT8U      cSNR [NFREQ+NEXOBS];            /*信号强度 (0.25 dBHz)*/
        INT8U      cLLI [NFREQ+NEXOBS];            /*失锁标识*/
        INT8U      cCode[NFREQ+NEXOBS];            /*编码标识 (CODE_???)*/
        FP64       dFrq[NFREQ+NEXOBS];  /*卫星频率*/
        FP64       dCNR [NFREQ+NEXOBS];            /*载噪比*/
        FP64       dL   [NFREQ+NEXOBS];            /*载波观测值 (cycle)*/
        FP64       dP   [NFREQ+NEXOBS];            /*伪距观测值 (m)*/
        FP32       fD   [NFREQ+NEXOBS];            /*多普勒观测值 (Hz)*/
}T_OBSSAT;

typedef struct                                 /*观测值*/
{
    INT32U     iN,iNmax;                       /*观测值个数及分配个数*/
    T_OBSSAT   ptData[MAXOBS];                 /*观测值数据*/
}T_OBS;

typedef struct                                 /*一个历元的导航电文 GPS/BDS*/
{
    INT32U     iSat;                           /*卫星编号*/
    INT32U     iIode,iIodc;                    /*IODE,IODC*/
    INT32U     iSva;                           /*卫星精度 (URA index)*/
    INT32U     iSvh;                           /*卫星健康状态(0:ok)*/
    INT32U     iWeek;                          /*GPS/QZS: GPS周, GAL: GALILEO周*/
    INT32U     iCode;                          /*GPS/QZS: L2上的码, GAL/BDS: 数据源*/
    INT32U     iFlag;                          /*GPS/QZS: L2 P码数据标记, BDS: 导航类型*/
    T_WEEKSEC  tToe,tToc,tTtr;                 /*Toe,Toc,T_trans（电文发送时刻）*/

                                               /*卫星轨道参数*/
    FP64       dSqrtA,dE,dI0,dOMG0,dOmg,dM0,dDeln,dOMGd,dIdot;
    FP64       dCrc,dCrs,dCuc,dCus,dCic,dCis;
    FP64       dToes;                          /*Toe (s) 周内秒*/
    FP64       dFit;                           /*拟合区间(h)*/
    FP64       dF0,dF1,dF2;                    /*卫星钟参数 (af0,af1,af2)*/
    FP64       dTgd[4];                        /*群延迟参数*/
                                               /*GPS/QZS:tgd[0]=TGD*/
                                               /*GAL    :tgd[0]=BGD E5a/E1,tgd[1]=BGD E5b/E1*/
                                               /*BDS    :tgd[0]=BGD1,tgd[1]=BGD2*/
}T_EPH;

typedef struct                                 /*一个历元的导航电文 GLO*/
{
    INT32U     iSat;                           /*卫星编号*/
    INT32U     iIode;                          /*IODE*/
    INT32S     iFrq;                           /*卫星频率数*/
    INT32U     iSvh,iSva,iAge;                 /*卫星健康状态、精度和运行年限信息*/
    T_WEEKSEC  tToc;                           /*历元时间 (GPST)*/
    T_WEEKSEC  tTof;                           /*电文帧时间 (GPST)*/
    FP64       dPos[3];                        /*卫星位置 (ECEF) (m)*/
    FP64       dVel[3];                        /*卫星速度 (ECEF) (m/s)*/
    FP64       dAcc[3];                        /*卫星加速度 (ECEF) (m/s^2)*/
    FP64       dTaun,dGamn;                    /*卫星钟偏差 (s)，卫星相对频率偏差*/
    FP64       dDtaun;                         /*L1与L2延迟 (s)*/
}T_GLOEPH;

typedef struct                                 /*广播星历*/
{
    INT32U     iN,iNmax;                       /*广播星历个数*/
    INT32U     iNg,iNgmax;                     /*GLO广播星历个数*/
    T_EPH      ptEph[MAXSAT];                  /*GPS/QZS/GAL/BDS星历*/
    T_GLOEPH   ptGeph[NSATGLO];                /*GLO星历*/
    FP64       dUtc_gps[4];                    /*GPS delta-UTC参数 {A0,A1,T,W} */
    FP64       dUtc_glo[4];                    /*GLO UTC GPST参数*/
    FP64       dUtc_gal[4];                    /*GAL UTC GPST参数*/
    FP64       dUtc_qzs[4];                    /*QZS UTC GPST参数*/
    FP64       dUtc_bds[4];                    /*BDS UTC参数*/
    FP64       dUtc_sbs[4];                    /*SBAS UTC参数*/
    FP64       dIon_gps[8];                    /*GPS电离层参数 {a0,a1,a2,a3,b0,b1,b2,b3}*/
    FP64       dIon_gal[4];                    /*GAL电离层参数 {ai0,ai1,ai2,0}*/
    FP64       dIon_qzs[8];                    /*QZS电离层参数 {a0,a1,a2,a3,b0,b1,b2,b3}*/
    FP64       dIon_bds[8];                    /*BDS电离层参数 {a0,a1,a2,a3,b0,b1,b2,b3}*/
    INT32U     iLeaps;                         /*跳秒 (s)*/
    FP64       dLam[MAXSAT][NFREQ];            /*载波波长 (m)*/
}T_NAV;

typedef struct                                 /*RTCM2.3 GNSS伪距差分改正数信息*/
{
    T_WEEKSEC  tObsTime;                       /*卫星观测时间 (GPST)*/
    FP64       dPrc;                           /*伪距改正数 (PRC) (m)*/
    FP64       dRrc;                           /*伪距变化率改正 (RRC) (m/s)*/
    INT32S     iIod;                           /*数据发布时间 (IOD)*/
    FP64       iUdre;                          /*用户差分距离误差 (UDRE)*/

}T_DGNSS;

typedef struct {                    /* multi-signal-message header type */
    INT8U iIod;              /* issue of data station */
    INT8U iTime_s;           /* cumulative session transmitting time */
    INT8U iClk_str;          /* clock steering indicator */
    INT8U iClk_ext;          /* external clock indicator */
    INT8U iSmooth;           /* divergence free smoothing indicator */
    INT8U iTint_s;           /* soothing interval */
    INT8U iNsat,iNsig;        /* number of satellites/signals */
    INT8U iSats[64];         /* satellites */
    INT8U iSigs[32];         /* signals */
    INT8U iCellmask[64];     /* cell mask */
} T_MSM_H;

typedef struct                                 /*RTCM信息*/
{
    INT32U     iType;                         /*当前解析出的电文类型*/
    INT32U     iStaid;                         /*测站编号*/
    INT32U     iStah;                          /*测站健康状况*/
    INT32U     iSeqno;                         /*rtcm 2序列号*/
    int outtype;        /* output message type */
    T_WEEKSEC  tTime;                          /*信息时间*/
    T_WEEKSEC  tTime_s;                        /*信息起始时间*/
    T_OBS      tObs;                           /*一个历元的观测数据（未修正）*/
    T_NAV      tNav;                           /*卫星星历*/
    T_STA      tSta;                           /*测站参数*/
    T_DGNSS    tDGnss[MAXOBS];                 /*伪距差分改正数*/
    INT8U      cMsg[128];                      /*特殊消息*/
    CHAR       cMsgtype[256];  /* last message type */
    CHAR       cMsmtype[3][128]; /* msm signal types */
    INT32S     iObsflag;                       /*观测数据完成标识(1:ok,0:没有完成)*/
    INT32U     iEphsat;                        /*卫星编号（更新卫星星历）*/
    FP64       dCp    [MAXSAT][NFREQ+NEXOBS];  /*载波观测值*/
    INT8U      cLock  [MAXSAT][NFREQ+NEXOBS];  /*锁定时间*/
    INT8U      cLoss  [MAXSAT][NFREQ+NEXOBS];  /*失锁个数*/
    T_WEEKSEC  tLltime[MAXSAT][NFREQ+NEXOBS];  /*最后锁定时间*/
    INT32U     iNbyte;                         /*缓存消息中的字节数*/
    INT32U     iNbit;                          /*缓存字中的位数*/
    INT32U     iLen;                           /*消息长度 (bytes)*/
    INT8U      cBuff[1200];                    /*缓存消息*/
    INT32U     iWord;                          /*rtcm 2缓存的字数*/
    INT32U     iNmsg2[100];                    /*RTCM 2消息数 (1-99:1-99,0:other)*/
    INT32U     iNmsg3[300];                    /*RTCM 3消息数 (1-299:1001-1299,0:other)*/
    CHAR       cOpt[256];                      /*RTCM选项*/
}T_RTCM;

typedef struct                                 /*卫星状态信息*/
{
    //FP64     dEpochTime;
    INT8U     iSatFlg;                         /*卫星状态0-正常    1-异常*/
    INT8U     iSPPFlg;                          /*SPP状态      0-正常    1-异常*/
    INT8U     iEphFlg;                          /*星历状态  0-正常    1-异常*/
    INT32U   iEpochNum;                /* 累积历元数*/
    INT32U   iEpochValid;                /* 有效历元数*/
    INT32U   iCycleslipNum[NFREQ+NEXOBS];                /* 周跳次数*/
    FP64       fMP[NFREQ+NEXOBS];  /*多路径指标*/
    INT32U       iMPCounter[NFREQ+NEXOBS];
    FP64       fMPMean[NFREQ+NEXOBS];
}T_SatState;

struct T_CHLSAT          /*每个通道卫星观测值*/
{
    T_CHLSAT()
    {
        SatChannel=dP=dL=fD=dCNR=0;
    }
        FP64 SatChannel;
        FP64 dP;            /*伪距(m)*/
        FP64 dL;            /*载波(cycle)*/
        FP32 fD;            /*多普勒观测值 (Hz)*/
        FP64 dCNR;          /*载噪比*/
};

typedef struct          /*每颗卫星频率情况*/
{
        INT8U iNum;            /*频率个数*/
        INT8U dFreqFlg[NFREQ];            //0,1,2 -> B1,B2,B3  //-1表示该标志位无效
}T_FREQ;

typedef struct                                /*周跳探测*/
{
        FP64     dEpochTime;
        INT8U validSatNum;
        INT8U validSatflg[MAXSAT];
        INT8U detValidflg[MAXSAT];
        INT8U slipflg[MAXSAT];
        FP64 phaseValue[NFREQ][MAXSAT];
        FP64 rangeValue[NFREQ][MAXSAT];
        T_FREQ iFreq[MAXSAT];       /*频率个数*/
        INT32S     iGFrq[MAXSAT];                           /*卫星频率数*/
}ObserDataEpoch;

typedef struct                                                 /*历元观测数据*/
{
        FP64     dEpochTime;                                       /*时间(s)*/
        INT8U    iNumOfValidSat;                                   /*有效卫星个数*/
        INT8U    iSatID[MAXOBS];                           /*有效卫星ID*/
        FP64     dElev[MAXOBS];                            /*卫星高度角*/
        FP64     dRovElev[MAXOBS];                            /*流动站卫星高度角*/
        FP64     dAmith[MAXOBS];                            /*卫星方位角*/
        FP64     dRovAmith[MAXOBS];                            /*流动站卫星方位角*/
        T_CHLSAT tData[MAXOBS][NFREQ]; /*卫星观测数据*/
        INT8U iFreq[MAXOBS];       /*频率个数*/
        FP64     dPosOfSat[MAXOBS][COORD_DIM];             /*卫星位置*/
        FP64     dPosOfBas[COORD_DIM];                             /*基准站位置*/
}T_EPOCHDATA;

typedef struct                                 /*转换配置*/
{
    T_WEEKSEC  tTs,tTe;                        /*起始和结束时间*/
    FP64       dTint;                          /*观测值间隔 (s)*/
    FP64       dRnxver;                        /*RINEX版本*/
    INT32U     iFormat;                        /*RTCM版本*/
    INT32U     iNavSys;                        /*导航系统*/
    INT32U     iObsType;                       /*观测值类型*/
    INT32U     iNfreq;                         /*频率个数*/
    INT8U      cStanam    [MAXANT];            /*测站名,生成RINEX文件名*/
    T_STA      tSta;                           /*测站信息*/
    T_WEEKSEC  tTstart;                        /*第一个观测历元时间*/
    T_WEEKSEC  tTend;                          /*最后一个观测历元时间*/
    T_WEEKSEC  tTrtcm;                         /*RTCM近似时间*/
    INT8U      cObsType [3][MAXOBSTYPE][4];    /*观测值类型{GPS,GLO,BDS}*/
    INT32U     iNobs[3];                       /*观测值类型个数{GPS,GLO,BDS}*/
    INT8U      cInfile[1024];                  /*输入文件*/
    INT8U      cOutfile[MAXOUTFILE][1024];     /*输出文件*/
}T_CONVOPT;

/******************************************************************************
*                          外部函数声明                                       *
******************************************************************************/
/*时间函数------------------------------------------------------------*/
extern FP64      YMD2JDay(T_YMD tYmd);                   /*通用时转换为儒略日*/
extern T_YMD     JDay2YMD(FP64 dJd);                     /*儒略日转换为通用时*/
extern FP64      WeekSec2JDay(T_WEEKSEC tWSec);          /*GPS时转换为儒略日*/
extern T_WEEKSEC JDay2WeekSec(FP64 dJd);                 /*儒略日转换为GPS时*/
extern T_YMD     WeekSec2YMD(T_WEEKSEC tWSec);           /*GPS时转换为通用时*/
extern T_WEEKSEC YMD2WeekSec(T_YMD tYmd);                /*通用时转换为GPS时*/
extern T_YEARDAY YMD2DOY(T_YMD tYmd);                    /*通用时转换为年积日*/
extern T_YEARDAY WeekSec2DOY(T_WEEKSEC tWSec);           /*GPS时转换为年积日*/
extern T_WEEKSEC TimeAdd(T_WEEKSEC tWSec,FP64 dSec);     /*时间加*/
extern FP64 TimeDiff(T_WEEKSEC tWSec1,T_WEEKSEC tWSec2); /*时间做差*/

/*卫星、导航系统和编码函数--------------------------------------------*/
extern INT32U Prn2Sat(INT32U iSys,INT32U iPrn);/*根据卫星prn和卫星系统得到卫星编号*/
extern INT32U Sat2Sys(INT32U iSat,INT32U *piPrn);/*根据卫星编号得到卫星系统*/
extern CHAR *Code2Obs(INT8U iCode,INT32U *piFreq);/*观测值编码转换成观测值编码字符*/
extern INT32U GetBitu(const INT8U *pcBuff,INT32U iPos,INT32U iLen);/*提取位 无符号型*/
extern INT32S GetBits(const INT8U *pcBuff,INT32U iPos,INT32U iLen);/*提取位 有符号型*/
extern void SetBitu(INT8U *pcBuff,INT32U iPos,INT32U iLen,INT32U iData);/*生成无符号型位*/
extern void SetBits(INT8U *pcBuff,INT32U iPos,INT32U iLen,INT32S iData);/*生成有符号型位*/
extern INT32U CRC24Q(INT8U *pcBuff,INT32U iLen);/*crc-24q检验*/

extern INT32U Decode_Word(INT32U iWord, INT8U *pcData); /*解码30bit电文，并进行奇偶校验*/

/*RINEX文件输出函数-----------------------------------------------------------*/
extern void OutRnxObsH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*输出观测文件文件头*/
extern void OutRnxNavH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*输出导航文件文件头*/
extern void OutRnxCnavH(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*输出BDS导航文件文件头*/
extern void OutRnxGnavH(FILE *ofp,T_CONVOPT *ptOpt);/*输出GLONASS导航文件文件头*/
extern INT32U OutRnxObsB(FILE *ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*输出卫星观测值*/
extern void OutRnxNavGNSS(FILE **ofp,T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*输出卫星导航星历*/

/*RTCM函数------------------------------------------------------------*/
extern INT32U Init_RTCM(T_RTCM *ptRtcm);/*初始化RTCM控制变量*/
extern INT32S Input_RTCM3f(T_RTCM *ptRtcm, FILE *fp);/*文件中RTCM3电文数据*/
extern INT32S Input_RTCM3(T_RTCM *ptRtcm,INT8U cData);/*实时流中RTCM3电文数据*/
extern INT32S Decode_RTCM3(T_RTCM *ptRtcm);/*解析RTCM3电文*/
extern INT32U Output_RTCM3(T_RTCM *ptRtcm,INT32U iType,INT32U iSync);/*生成RTCM3电文,FILE *fp*/
extern INT32U Encode_RTCM3(T_RTCM *ptRtcm,INT32U iType,INT32U iSync);/*编码RTCM3电文*/

extern INT32S Input_RTCM2f(T_RTCM *ptRtcm, FILE *fp);/*文件中RTCM2电文数据*/
extern INT32S Input_RTCM2(T_RTCM *ptRtcm,INT8U cData);/*实时流中RTCM2电文数据*/
extern INT32S Decode_RTCM2(T_RTCM *ptRtcm);/*解析RTCM2电文*/
extern INT32U Output_RTCM2(T_RTCM *ptRtcm,INT32U iType,INT32U iSync);/*生成RTCM2电文,FILE *fp*/
extern INT32U Encode_RTCM2(T_RTCM *ptRtcm,INT32U iType,INT32U iSync);/*编码RTCM2电文*/


/*观测值类型函数------------------------------------------------------*/
extern void ConvCode322(FP64 dVer,INT32U iSys,INT8S *pcObsType);/*RINEX观测值类型转换 ver.3 -> ver.2*/
extern void SetOpt_ObsType(INT32U *piCodes,INT32U iSys, T_CONVOPT *ptOpt);/*生成RINEX文件中的观测值类型*/
extern void SetObsType(T_CONVOPT *ptOpt);/*设置观测值类型*/
extern INT8U Obs2Code(const CHAR *obs, INT32U *freq);
extern FP64 SatWavelen(INT32U sat, INT32U frq, const T_NAV *nav);
extern INT32U GetCodepri(INT32U sys, INT8U code, const CHAR *opt);

/*转换和配置函数------------------------------------------------------*/
extern INT32U SetOutFileName(T_CONVOPT *ptOpt);/*输出文件名设置*/
extern INT32S InputFile(T_RTCM *ptRtcm,INT32U iFormat,FILE *fp);/*根据电文格式选择电文解析函数*/
extern void   ConvRnx(T_CONVOPT *ptOpt,T_RTCM *ptRtcm);/*RTCM电文转换成RINEX格式*/

/*------------------------------------------------------------------*/
extern int Get_Type_From_RTCM_Num(int nSatNum);
extern int Get_Num_From_RTCM_Num(int nSatNum);

#endif







