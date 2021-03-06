#include "corsdbmanager.h"
#include "oci_wrapper.h"
#include "sql_wrapper.h"
#include <QDateTime>
#include "util.h"
#include <unistd.h>
#define BUFF_SIZE_SEND 8192             // 向数据处理服务器发送XML数据的缓存大小

const int g_corTH = 42;
const int g_corBD=49;
const int g_regTH = 2364;
const int g_regHP=2366;
const int g_regGZ=2359;
const int g_regGD=2358;
// values (2358, 234, '广东省', '4', null, '440000');
// values (2359, 2358, '广州市', '2', null, '440100');
// values (2364, 2359, '天河区', '1', null, '440106');
//values (2366, 2359, '黄埔区', '1', null, '440112');
//values (49, 2366, '北斗产业园', null, 113.44896390, 23.15670278, null, 'B');
//values (42, 2364, '天河站', null, 113.36161110, 23.16228333, null, 'B');

using namespace std;
pthread_mutex_t mutex_lock;                             // 互斥锁

CorsDBManager::CorsDBManager():m_xmlMode(false),m_xmlServer(-1)
{

}

CorsDBManager::~CorsDBManager()
{
    if(m_xmlServer>0)close(m_xmlServer);
}

bool CorsDBManager::Init(bool xml)
{
    // 通过参数初始化数据库管理类
    //QString dbPara[]={"172.17.223.101", "orcl", "gnss", "gnss"};
   QString dbPara[]={"14.23.115.222", "gnss", "cors", "yW123456"};
    if(!OCI_Wrapper::GetInstance()->Init(dbPara[0],dbPara[1],dbPara[2],dbPara[3]))
    {
        return false;
    }
    // 获取各表的最大Id
    m_nSatDetID = OCI_Wrapper::GetInstance()->GetLastId("cor_sat_det","sat_detail_id");
    m_nSatCalcID = OCI_Wrapper::GetInstance()->GetLastId("cor_calculate_rec","cor_calc_id");
    m_nAlarmDetID = OCI_Wrapper::GetInstance()->GetLastId("mnt_alarm_det","alarm_detail_id");
    m_nCorStateID = OCI_Wrapper::GetInstance()->GetLastId("cor_status_rec","cor_status_id");
    m_nSatTotalID = OCI_Wrapper::GetInstance()->GetLastId("cor_sat_total","sat_total_id");
    m_nAlarmTotID = OCI_Wrapper::GetInstance()->GetLastId("mnt_alarm_tal","alarm_total_id");
    m_nSatRecID = OCI_Wrapper::GetInstance()->GetLastId("cor_sat_rec","sat_rec_id");
    m_xmlMode=xml;
    if(m_xmlMode)
     {
         m_xmlServer=UtilAPB::ConnectTcpServer ("172.17.223.101",2222);
        if(m_xmlServer<=0)
        {
            UnInit();
            return false;
        }
    }
    return true;
}

void CorsDBManager::UnInit()
{
    if(m_xmlServer>0)
    {
        close(m_xmlServer);
    }
    OCI_Wrapper::GetInstance()->UnInit();
}

void CorsDBManager::Write(const COR_SAT_ALL& cal)
{
    QDateTime dt = QDateTime::currentDateTime();
    m_CreateTime = dt.toString("yyyy-MM-dd hh:mm:ss");
    if(m_xmlMode)
    {
        WriteToXML (cal);
    }
    else
    {
        insertAlarmTal (cal);
        insertCalcuRec (cal);
        insertCorSatDet (cal);
        //insertCorSatRec (cal);
        //更新时间戳
        OCI_Wrapper::GetInstance() ->Execute( QString("update mnt_timestamp "
                                               "set update_timestamp="
                                               "to_date('%1','YYYY-MM-DD HH24:MI:SS')")
                                              .arg(m_CreateTime));
        OCI_Wrapper::GetInstance()->Commit();
    }
    UtilAPB::MyDebug(QString("esp*****************%1")
                     .arg(dt.msecsTo(QDateTime::currentDateTime())));
}

//卫星明细
//(sat_detail_id, cor_id, reg_id,
//sat_cd, sat_type, sat_channel, sat_status,
//sat_height, sat_azimuth, sat_signal,
//sham_range, sham_range_prec,create_time)
bool CorsDBManager::insertCorSatDet(const COR_SAT_ALL& cal)
{
    QString cmd="insert ALL ";
    for(int i = 0;  i < cal.vSatDets.size(); i++)
    {
        const COR_SAT_DET& det=cal.vSatDets.at (i);

        cmd+=QString(" INTO cor_sat_det VALUES (%1,%2,%3,'%4','%5','%6','%7'"
                     ",%8,%9,'%10', %11,%12,to_date('%13','YYYY-MM-DD HH24:MI:SS'))")
                                   .arg(m_nSatDetID++).arg(g_corBD).arg(g_regHP)
                                   .arg(det.nSatNum).arg(det.nSatType).arg(det.SatChannel).arg(det.nSatState)
                                   .arg(det.dYangjiao).arg(det.dFangweijiao).arg(det.dSNR)
                                   .arg(det.shamRange).arg(det.shamRangePrec).arg(m_CreateTime);
    }
    cmd.append (" select 1 from dual");
    OCI_Wrapper::GetInstance()->Execute(cmd);
}

//卫星数据完好明细
//"(sat_rec_id,cor_id, sat_id,sat_elevation, sat_azimuth"
//",sat_frequency_one, sat_signal_one, sat_carrier_one, SAT_NOISE_ONE"
//",sat_frequency_two, sat_signal_two, sat_carrier_two, SAT_NOISE_two"
//",sat_frequency_three, sat_signal_three, sat_carrier_three, SAT_NOISE_three"
//",sat_status, spp_status, ephemeris_status"
//",all_epoch_count,valid_epoch_count"
//",cycle_slip_one,multipath_quota_one"
//",cycle_slip_two,multipath_quota_two"
//",CALC_OBSERVE_SUB, OBSERVE_CARRIER_SUB, SAT_SIGNAL_STATUS"
//",NEW_EPHEMERIS_TELEGRAM, NEW_OLD_TELEGRAM, SAT_LOCATION_STATUS"
//",IOD_CHANGE,create_time)"
bool CorsDBManager::insertCorSatRec(const COR_SAT_ALL& cal)
{
    QString cmd="insert ALL ";
    for(int i = 0;  i < cal.vSatDets.size(); i++)
    {
        const COR_SAT_DET& det=cal.vSatDets.at (i);
        cmd+=QString(" INTO cor_sat_rec values (%1,%2,%3,%4,%5, %6,%7,%8,%9"
                     ",%10,%11,%12,%13, %14,%15,%16,%17, '%18','%19','%20', %21,%22"
                     ",%23,%24,%25,%26, %27,%28,'%29', %30,%31,'%32',%33"
                     ",to_date('%34','YYYY-MM-DD HH24:MI:SS'))")
                .arg(m_nSatRecID++).arg(g_corBD).arg(det.nSatNum).arg(det.dYangjiao).arg(det.dFangweijiao)
                .arg(det.tch[0].SatChannel).arg(det.tch[0].dP).arg(det.tch[0].dL).arg(det.tch[0].dCNR)
                .arg(det.tch[1].SatChannel).arg(det.tch[1].dP).arg(det.tch[1].dL).arg(det.tch[1].dCNR)
                .arg(det.tch[2].SatChannel).arg(det.tch[2].dP).arg(det.tch[2].dL).arg(det.tch[2].dCNR)
                .arg(det.tssCal.iSatFlg).arg(det.tssCal.iSPPFlg).arg(det.tssCal.iEphFlg)
                .arg(det.tssCal.iEpochNum).arg(det.tssCal.iEpochValid)
                .arg(det.tssCal.iCycleslipNum[0]).arg(det.tssCal.fMP[0])
                .arg(det.tssCal.iCycleslipNum[1]).arg(det.tssCal.fMP[1])
                .arg(det.dCalcObsSus).arg(det.dObsCarrSus).arg(det.bSatSignalStatus)
                .arg(det.dNerPheTele).arg(det.dNerOldTele)
                .arg(det.bLocStatus).arg(det.dIodChange).arg(m_CreateTime);
    }
    cmd.append (" select 1 from dual");
    OCI_Wrapper::GetInstance()->Execute(cmd);
}

//结算结果
//select * from all_tab_columns where Table_Name='cor_calculate_rec';
//" (cor_calc_id, reg_id,cor_id,calc_state,sat_count"
//",point_long,point_lat,point_height"
//",wgs_x, wgs_y, wgs_z,diff_x, diff_y, diff_z"
//",sat_signal,data_error_per,single_pre,signal_pre"
//",sham_range,good_per,create_time)"
bool CorsDBManager::insertCalcuRec(const COR_SAT_ALL& cal)
{

    OCI_Wrapper::GetInstance()->Execute(QString("insert into cor_calculate_rec"
                                                " values (%1,%2,%3,'%4'"
                                                ",%5,%6,%7,%8"
                                                ",%9,%10,%11,%12,%13,%14"
                                                ",%15,%16,%17,%18"
                                                ",%19,%20,to_date('%21','YYYY-MM-DD HH24:MI:SS'))")
                               .arg(m_nSatCalcID++).arg(g_regHP).arg(g_corBD).arg(cal.nCalState)
                               .arg(cal.nSatCount).arg(cal.dLng).arg(cal.dLat).arg(cal.dHeight)
                               .arg(cal.dWgsX). arg(cal.dWgsY).arg(cal.dWgsZ).arg(cal.dDiffX). arg(cal.dDiffY).arg(cal.dDiffZ)
                              .arg(cal.dSignal).arg(cal.dErrorPer).arg(cal.dSinglePre).arg(cal.dSinglePre)
                              .arg(cal.dShamRange).arg(cal.dGoodPer).arg(m_CreateTime));
}

//告警明细
//"(alarm_detail_id,reg_id,object_type,object_id"
//",alarm_level,alarm_type"
//",object_value,alarm_desc,create_time)
//卫星汇总
//"(sat_total_id,object_level,object_id,reg_id"
//",vis_sat_interval,vis_sat_value,create_time)"
//监控汇总
//"(alarm_total_id,reg_id,object_type,object_id"
//",alarm_level,count_number,create_time"
//cor状态
//"(cor_status_id,cor_id,reg_id,cor_status, cor_alarm_level ,create_time)
bool CorsDBManager::insertAlarmTal(const COR_SAT_ALL& cal)
{
    static QString type[3]={"B","S","S"};
    static int cid[3]={g_corBD,g_regHP,g_regGZ};
    static int pid[3]={g_regHP,g_regGZ,g_regGD};
    const int count = UtilAPB::RandomRange(1,10);
    const int level=UtilAPB::RandomRange(0,3);
    //告警明细
    QString cmd="insert ALL ";
    for(int i=0;i<count;i++)
    {
        cmd+=QString(" INTO mnt_alarm_det values (%1,%2,'%3',%4,'%5','%6','%7','%8'"
                     ",to_date('%9','YYYY-MM-DD HH24:MI:SS'))")
                .arg(m_nAlarmDetID++).arg(g_regHP).arg ("B").arg(g_corBD)
                .arg(UtilAPB::RandomRange(0,level)).arg(UtilAPB::RandomRange(1,7))
                .arg ("test value!").arg ("test desc!").arg(m_CreateTime);
    }
    cmd.append (" select 1 from dual");
    OCI_Wrapper::GetInstance()->Execute(cmd);

    //卫星汇总
    cmd.clear ();
    cmd="insert ALL ";
    for(int i=0;i<3;i++)
    {
        cmd+=QString("INTO cor_sat_total values (%1,'%2',%3,%4"
                      ",'%5','%6',to_date('%7','YYYY-MM-DD HH24:MI:SS'))")
                .arg(m_nSatTotalID++).arg(type[i]).arg(cid[i]).arg(pid[i])
                .arg(UtilAPB::RandomRange(0,3)).arg(UtilAPB::RandomRange(0,32)).arg(m_CreateTime);
    }
    cmd.append (" select 1 from dual");
    OCI_Wrapper::GetInstance()->Execute(cmd);

    //监控汇总
    cmd.clear ();
    cmd="insert ALL ";
    for(int i=0;i<3;i++)
    {
        cmd+=QString(" INTO mnt_alarm_tal values (%1,%2,'%3',%4,'%5',%6,to_date('%7','YYYY-MM-DD HH24:MI:SS'))")
                .arg(m_nAlarmTotID++).arg(pid[i]).arg(type[i]).arg(cid[i]).arg(level).arg(count).arg(m_CreateTime);
    }
    cmd.append (" select 1 from dual");
    OCI_Wrapper::GetInstance()->Execute(cmd);

    //cor状态
    OCI_Wrapper::GetInstance()->Execute(QString("insert into cor_status_rec values (%1,%2,%3,'%4','%5'"
                                               ",to_date('%6','YYYY-MM-DD HH24:MI:SS'))")
                                        .arg(m_nCorStateID++).arg(g_corBD).arg(g_regHP).arg(cal.nCorState).arg(level).arg(m_CreateTime));
    return true;
}

#include <DataXml/dbStruct.h>
#include <DataXml/toString.h>
void CorsDBManager::WriteToXML(const COR_SAT_ALL& cal)
{
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    gnssDb db;
    Table tables;
    for(int i = 0; i < cal.vSatDets.size(); i++)
    {
        const COR_SAT_DET& det=cal.vSatDets[i];
        CorSatDet* corSatDet = new CorSatDet();
        corSatDet->createTime = strDateTime.toStdString();
        corSatDet->satDetailId = 0;
        corSatDet->corId = g_corBD;
        corSatDet->regId = g_regHP;
        corSatDet->satAzimuth = det.dFangweijiao;
        corSatDet->satHeight = det.dYangjiao;
        corSatDet->satCd = itos(det.nSatNum);
        corSatDet->satStatus =itos(det.nSatState);
        corSatDet->satSignal = det.dSNR;
        corSatDet->satType = det.nSatType;
        corSatDet->satChannel = det.SatChannel;
        corSatDet->shamRange = det.shamRange;
        corSatDet->shamRangePrec = det.shamRangePrec;
        tables.add(corSatDet);
    }

    CorCalculateRec* calculateRec = new CorCalculateRec();
    calculateRec->corCalcId = 0;
    calculateRec->createTime = strDateTime.toStdString();
    calculateRec->corId = g_corBD;
    calculateRec->regId = g_regHP;
    calculateRec->calcState = cal.nCalState;
    calculateRec->pointLat = cal.dLat;
    calculateRec->pointLong = cal.dLng;
    calculateRec->pointHeight = cal.dHeight;
    calculateRec->wgsX = cal.dWgsX;
    calculateRec->wgsY = cal.dWgsY;
    calculateRec->wgsZ = cal.dWgsZ;
    calculateRec->diffX = cal.dDiffX;
    calculateRec->diffY = cal.dDiffY;
    calculateRec->diffZ = cal.dDiffZ;
    calculateRec->satCount = cal.nSatCount;
    calculateRec->satSignal = cal.dSignal;
    calculateRec->shamRange = 0.0;
    calculateRec->dataErrorPre = 0.0;
    calculateRec->goodPer = 0.0;
    calculateRec->signalPre = 0.0;
    calculateRec->singlePre = 0.0;           // dummy
    tables.add(calculateRec);

    MntTimestamp* mntTimestamp = new MntTimestamp();
    mntTimestamp->updateTimestamp = strDateTime.toStdString();
    tables.add(mntTimestamp);

    db.add(tables);

    string gnss = db.toXml();
    gnss.append("\n");

    static char sendbuf[BUFF_SIZE_SEND] = {'\0'};
    strcpy(sendbuf,gnss.data());
    int nLen = strlen(sendbuf);
    send(m_xmlServer, sendbuf, nLen,0);     //发送
    memset(sendbuf, 0, sizeof(sendbuf));

    for(vector<Object*>::iterator it = tables.objects.begin(); it != tables.objects.end();)
    {
       Object* pObject = *it;
       delete pObject;
       tables.objects.erase(it);
    }
}
