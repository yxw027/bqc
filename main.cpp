#include <QCoreApplication>
#include <QDateTime>
#include <QTextCodec>

#include <RTCM2RNX/def_convrnx.h>
#include <RTCM2RNX/rtcm3dec.h>
#include <NMEA0183/nmea0183dec.h>
#include "OracleWrapper/corsdbmanager.h"

#include "util.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include<stdio.h>
#include<stdlib.h>

const int BUFF_SIZE=4096;
const char serverIP[20]="172.17.223.105";
const int nmeaPort=2112;
const int rtcmPort=2111;

CorsDBManager g_corsDbManager;      // Cors数据库管理类
Nmea0183Dec g_nmea0183Dec;           // Nmea0183解码器
Rtcm3Dec g_rtcm3Dec;                           // Rtcm3解码器

sem_t g_sem_rtcm;                                   // rtcm数据资源的信号量
sem_t g_sem_nmea;                                 // nmea数据资源的信号量
sem_t g_sem_sync;                                   // 同步资源的信号量

char szBuffNmea0183[BUFF_SIZE];       // Nmea0183数据缓冲区
char szBuffRtcm3[BUFF_SIZE];               // Rtcm3数据缓冲器

int nFdNmea0183 = 0;                            // 接收Nmea0183数据的socket句柄
int nFdRtcm3 = 0;                                    // 接收Rtcm3数据的socket句柄

// Nmea0183数据接收、解码线程
void* thread_nmea0183(void* )
{
    memset(szBuffNmea0183,0,BUFF_SIZE);
    while(TRUE)
    {
        if(!nFdNmea0183) break;
        int nReadLen=read(nFdNmea0183,szBuffNmea0183,BUFF_SIZE) ;
        if (nReadLen> 0)
        {
            // 解码 NMEA0183 数据
            for (int i=0;i<nReadLen;i++)
            {
                /*逐个字符读取0183电文*/
                if(g_nmea0183Dec.Decode(szBuffNmea0183[i]))
                {
                    // 将数据nmea数据写入未同步的缓存队列中
                    sem_post(&g_sem_nmea);// 生产Nmea0183数据
                }
            }
        }
    }
}

// Rtcm3数据接收、解码线程
void* thread_rtcm3(void* )
{
    memset(szBuffRtcm3,0,BUFF_SIZE);
    while(TRUE)
    {
        if(!nFdRtcm3) break;
        int nReadLen=read(nFdRtcm3,szBuffRtcm3,BUFF_SIZE);
        if (nReadLen > 0)
        {
            // 解码 RTCM3 数据
            for (int i=0;i<nReadLen;i++)
            {
                /*逐个字符读取rtcm3电文*/
                if(g_rtcm3Dec.Decode(szBuffRtcm3[i]))
                {
                    sem_post(&g_sem_rtcm);
                }
            }
        }
    }
}

// Rtcm3和Nmea0183数据同步线程
void* thread_sync(void* )
{
    while(TRUE)
    {
        sem_wait(&g_sem_nmea);      // 消费Nmea3数据
        sem_wait(&g_sem_rtcm);        // 消费Rtcm3数据
        if(g_nmea_cache_cor_dat.size() > 0 && g_rtcm_cache_cor_dat.size() > 0)
        {
            COR_SAT_ALL &nmeaDet = g_nmea_cache_cor_dat.front();
            COR_SAT_ALL &rtcmDet = g_rtcm_cache_cor_dat.front();
            // 没有卫星
            if(nmeaDet.vSatDets.size() == 0)
            {
                g_nmea_cache_cor_dat.pop();
                UtilAPB::MyDebug ("nmeaDets no sat!");
                continue;
            }
            // 没有卫星
            if(rtcmDet.vSatDets.size() == 0)
            {
                g_rtcm_cache_cor_dat.pop();
                UtilAPB::MyDebug ("rtcmDets no sat!");
                continue;
            }

            // 同步Rtcm3数据和Nmea0183数据
            if(nmeaDet.dUTCTime > rtcmDet.dUTCTime)
            {// 清除时间较早的数据
                g_rtcm_cache_cor_dat.pop();
                UtilAPB::MyDebug ("dTimeCalcuRec > dTimeRtcmDets!");
                continue;
            }
            else if(nmeaDet.dUTCTime < rtcmDet.dUTCTime)
            {// 清除时间较早的数据
                g_nmea_cache_cor_dat.pop();
                UtilAPB::MyDebug ("dTimeCalcuRec < dTimeRtcmDets!");
                continue;
            }
            else
            {
                COR_SAT_ALL calDet=nmeaDet;
                calDet.dSignal=100;
                calDet.dWgsX = UtilAPB::RandomRange(2230000,2239999);
                calDet.dWgsY = UtilAPB::RandomRange(5380000,5389999);
                calDet.dWgsZ = UtilAPB::RandomRange(2490000,2499999);
                calDet.nCalState=2;
                calDet.vSatDets.clear ();
                for(int i = 0; i < nmeaDet.vSatDets.size(); i++)
                {
                    COR_SAT_DET &nmea=nmeaDet.vSatDets[i];
                    for(int j = 0; j < rtcmDet.vSatDets.size(); j++)
                    {
                        COR_SAT_DET &rtcm=rtcmDet.vSatDets[j];
                        if(nmea.nSatNum == rtcm.nSatNum)
                        {
                            COR_SAT_DET cal=nmea;
                            cal.shamRangePrec=rtcm.shamRangePrec;
                            cal.shamRange=rtcm.shamRange;
                            cal.nSatState=0;
                            cal.tch[0] = rtcm.tch[0];
                            cal.tch[1] = rtcm.tch[1];
                            cal.tch[2] = rtcm.tch[2];
                            cal.tssCal=rtcm.tssCal;
                            calDet.vSatDets.push_back (cal);
                        }
                    }
                }
                g_sync_cache_cal_rec.push(calDet);                       // 同步后的解算数据的缓存队列
                sem_post(&g_sem_sync);                                              // 生产同步后的数据

                // 将时间同步的数据出队
                g_nmea_cache_cor_dat.pop();
                g_rtcm_cache_cor_dat.pop();
            }
        }
    }
}

//
void* thread_db(void*)
{
    while(TRUE)
    {
        sem_wait(&g_sem_sync);// 消费同步后的数据
        // 从同步后的队列中取数据写入数据库
        while(g_sync_cache_cal_rec.size())
        {
            g_corsDbManager.Write(g_sync_cache_cal_rec.front());
            g_sync_cache_cal_rec.pop();
        }
    }
}

// 主函数
// 开启4个线程接收、处理、同步和写入数据
// 两个线程分别接收Nmea0183和Rtcm3数据
// 一个线程负责同步数据
// 一个线程负责将同步后的数据写入或发送到数据库服务器

int main(int , char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    pthread_mutex_init(&mutex_lock,NULL);

    // 连接Rtcm3数据源、Nmea数据源
    if((nFdNmea0183 = UtilAPB::ConnectTcpServer(serverIP, nmeaPort))<=0)
    {
        g_corsDbManager.UnInit();
        return -1;
    }

    if((nFdRtcm3 = UtilAPB::ConnectTcpServer(serverIP, rtcmPort))<=0)
    {
        g_corsDbManager.UnInit();
        return -1;
    }

    // 通过参数初始化数据库管理类
    QString ip = argv[1];
    QString sid = argv[2];
    QString user = argv[3];
    QString password = argv[4];
    UtilAPB::MyDebug(QString("ServerIP:%1 SID:%2 User:%3 Password:%4").arg(ip,sid,user,password));
    if(!g_corsDbManager.Init(ip,sid,user,password))
    {
        g_corsDbManager.UnInit();
        return -1;
    }

    // 初始化信号量
    sem_init(&g_sem_nmea,0,0);
    sem_init(&g_sem_rtcm,0,0);
    sem_init(&g_sem_sync,0,0);

    pthread_t threadNmea0183 = NULL;
    pthread_t threadRtcm3 = NULL;
    pthread_t threadSync = NULL;
    pthread_t threadDb = NULL;

    // 开启Nmea0183数据接收处理线程、
    //Rtcm3数据接收处理线程、数据同步线程和数据写入或发送线程
    pthread_create(&threadRtcm3, NULL, thread_rtcm3, NULL);
    pthread_create(&threadNmea0183, NULL, thread_nmea0183, NULL);
    pthread_create(&threadSync, NULL, thread_sync, NULL);
    pthread_create(&threadDb, NULL, thread_db, NULL);

    // 等待各线程结束
    pthread_join(threadNmea0183,NULL);
    pthread_join(threadRtcm3,NULL);
    pthread_join(threadSync,NULL);
    pthread_join(threadDb, NULL);

    g_corsDbManager.UnInit();
    close(nFdNmea0183);
    close(nFdRtcm3);
    return 0;
}
