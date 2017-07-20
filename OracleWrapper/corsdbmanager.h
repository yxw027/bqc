///////////////////////////////////////////////////////////////////////////////////////////////////
// Cors数据库管理类，负责Cors数据库的读写

#ifndef CORSDBMANAGER_H
#define CORSDBMANAGER_H

#include <QString>
#include <vector>
#include <queue>
#include <OracleWrapper/corsdbmanager.h>
#include <OracleWrapper/oci_wrapper.h>
#include "def_convrnx.h"

using namespace std;
extern pthread_mutex_t mutex_lock;

// 卫星明细表数据
struct COR_SAT_DET      /*Cor sat detail*/
{
    COR_SAT_DET()
    {
        nSatType=nSatNum=SatChannel=nSatState=0;
        dUTCTime=shamRangePrec=shamRange=dYangjiao=dFangweijiao=dSNR=0;
        dCalcObsSus=dObsCarrSus=bSatSignalStatus=dNerPheTele=dNerOldTele=bLocStatus=dIodChange=0;
    }
    int nSatType;             // 卫星类型
    int nSatNum;              // 卫星编号
    int nSatState;
    double dUTCTime;

    int SatChannel;
    double shamRangePrec;
    double shamRange;

    double dYangjiao;                // 仰角
    double dFangweijiao;          // 方位角
    double dSNR;            /*xin噪比*/

    double dCalcObsSus;		// 计算伪距与观测伪距之差
    double dObsCarrSus;		// 观测伪距与载波平滑伪距之差
    bool bSatSignalStatus;		// 伪距之差状态
    double dNerPheTele;     //新卫星星历和电文计算位置比较, 单位米
    double dNerOldTele;     //新旧电文计算卫星位置比较, 单位米
    bool bLocStatus;        //位置是否有效
    double dIodChange;  //电离层变化

    T_SatState tssCal;  //calcu result
    T_CHLSAT tch[3];
};

//卫星数据
struct COR_SAT_ALL      /*Cor sat detail*/
{
    void clear()
    {
        nCorId=nSatCount=nCalState=nCorState=0;
        dUTCTime=dDiffX=dDiffY=dDiffZ=0;
        dLng=dLat=dHeight=0;
        dSignal=dWgsX=dWgsY=dWgsZ=0;
        dErrorPer=dSinglePre=dSignalPre=dShamRange=dGoodPer=0;
        vSatDets.clear ();
    }
    int nCorId;                            // CORS站标识
     int nSatCount;                      // 卫星数
    int nCalState;                          //jiesuan zhuangtai
    int nCorState;

    double dDiffX;                      // X误差
    double dDiffY;                      // Y误差
    double dDiffZ;                      // Z误差
    double dUTCTime;

    double dLng;                        // 经度
    double dLat;                         // 纬度
    double dHeight;                   // 高度

    double dWgsX;                     // WGS84-X
    double dWgsY;                     // WGS84-Y
    double dWgsZ;                     // WGS84-Z

    double dSignal;                    //信号强度
    double dErrorPer;//错误估计
    double dSinglePre;//单点精度
    double dSignalPre;//载波精度
    double dShamRange;//伪距精度
    double dGoodPer;//完好率
    vector<COR_SAT_DET>     vSatDets;//
};

extern queue<COR_SAT_ALL> g_nmea_cache_cor_dat;    //  同步前nmea的卫星数据的缓存队列
extern queue<COR_SAT_ALL> g_rtcm_cache_cor_dat;     //  同步前rtcm的卫星数据的缓存队列
extern queue<COR_SAT_ALL> g_sync_cache_cal_rec;     //  同步后解算数据的缓存队列

class CorsDBManager
{
private:
    int m_nSatDetID;
    int m_nSatCalcID;
    int m_nAlarmDetID;
    int m_nCorStateID;
    int m_nSatTotalID;
    int m_nAlarmTotID;
    int m_nSatRecID;
    QString m_CreateTime;
    bool m_xmlMode;
    int m_xmlServer;                               // liwei服务器的socket句柄
public:
    CorsDBManager();
    ~CorsDBManager();

     // 初始化
     bool Init(QString ip, QString sid, QString user, QString password,bool xml=false);
     // 反初始化
     void UnInit();
     // 写入
     void Write(const COR_SAT_ALL&);

private:
     bool WriteToDB(const COR_SAT_ALL& cal);
     void WriteToXML(const COR_SAT_ALL& cal);
};

#endif // CORSDBMANAGER_H
