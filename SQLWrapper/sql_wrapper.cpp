#include "sql_wrapper.h"
#include "util.h"

SQL_Wrapper* SQL_Wrapper::instance = nullptr;

SQL_Wrapper::SQL_Wrapper()
{
}

SQL_Wrapper *SQL_Wrapper::GetInstance()
{// 单例
    if(nullptr==instance)
        instance = new SQL_Wrapper();
    return instance;
}

bool SQL_Wrapper::Init(QString ip, QString sid, QString user, QString password)
{
    m_db = QSqlDatabase::addDatabase("QOCI");
    m_db.setHostName(ip);
    m_db.setDatabaseName(sid);
    m_db.setUserName(user);
    m_db.setPassword(password);
    if (!m_db.open())
    {
        UtilAPB::MyDebug ("SQLDB CONN FAILED!");
        return false;
    }
    else
    {
        m_result=QSqlQuery(m_db);
        m_result.setForwardOnly(true);
        if (m_db.driver()->hasFeature(QSqlDriver::Transactions))
        {
            m_db.transaction();
        }
        return true;
    }
}

void SQL_Wrapper::UnInit()
{
    m_db.close();
    m_result.clear ();
}

bool SQL_Wrapper::Execute(const QString &cmd)
{// 执行Sql
    if(m_result.exec (cmd))
    {
        return true;
    }
    else
    {
        UtilAPB::MyDebug (m_result.lastError());
        UtilAPB::MyDebug (m_result.lastQuery());
        return false;
    }
}

void SQL_Wrapper::Commit()
{// 提交修改
    if(m_db.commit ())
    {
        return true;
    }
    else
    {
        UtilAPB::MyDebug (m_db.lastError());
        return false;
    }
}
#include <QVariant>
// get last id from a table
int SQL_Wrapper::GetLastId(QString strTableName, QString strIdName)
{
    SQL_Wrapper::GetInstance()->Execute(QString("select max(%1) as id from %2").arg(strIdName,strTableName));
    SQL_Wrapper::GetInstance()->Commit ();
    if(m_result.next ())
    {
        QVariant qv=m_result.value(0);
        return qv.toInt()+1;
    }
    return 0;
}
