///////////////////////////////////////////////////////////////////////////////////////////////////
// Cors数据库管理类，负责Cors数据库的读写

#ifndef CORSDBMANAGER_H
#define CORSDBMANAGER_H
#include "def_cor_sat.h"
#include <QString>


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
     bool Init(bool xml=false);
     // 反初始化
     void UnInit();
     // 写入
     void Write(const COR_SAT_ALL&);

private:
     void WriteToXML(const COR_SAT_ALL& cal);
     bool insertCorSatDet(const COR_SAT_ALL &cal);
     bool insertCorSatRec(const COR_SAT_ALL &cal);
     bool insertCalcuRec(const COR_SAT_ALL &cal);
     bool insertAlarmTal(const COR_SAT_ALL &cal);
};

#endif // CORSDBMANAGER_H
