#include "oci_wrapper.h"
#include "util.h"

OCI_Wrapper* OCI_Wrapper::instance = nullptr;

OCI_Wrapper::OCI_Wrapper()
{
}

OCI_Wrapper::~OCI_Wrapper()
{

}

OCI_Wrapper *OCI_Wrapper::GetInstance()
{// 单例
    if(nullptr==instance)
        instance = new OCI_Wrapper();
    return instance;
}

void err_handler(OCI_Error *err)
{
    UtilAPB::MyDebug (QString("msg  : %1").arg (OCI_ErrorGetString(err)));
    UtilAPB::MyDebug (QString("sql  :%1").arg (OCI_GetSql(OCI_ErrorGetStatement(err))));
}

bool OCI_Wrapper::Init(QString ip, QString sid, QString user, QString password, QString)
{
    // OCILib 初始化
    if ( !OCI_Initialize(err_handler, NULL,OCI_ENV_DEFAULT))// OCI_ENV_CONTEXT|OCI_ENV_THREADED) )
    {
        return EXIT_FAILURE;
    }
    OCI_EnableWarnings(TRUE);

    // OCILib 建立连接
    QString strDb = ip + "/" + sid;
    m_conn = OCI_ConnectionCreate(strDb.toLatin1().data(),
                                  user.toLatin1().data(),
                                  password.toLatin1().data(),
                                  OCI_SESSION_DEFAULT);

    if ( m_conn )
    {
        UtilAPB::MyDebug(OCI_GetVersionServer(m_conn));
        m_st = OCI_StatementCreate(m_conn);
        return true;
    }
    return false;
}

void OCI_Wrapper::UnInit()
{// 释放资源
    if(m_rs)
        OCI_ReleaseResultsets(m_st);

    if(m_st)
        OCI_StatementFree(m_st);

    if(m_conn)
        OCI_ConnectionFree(m_conn);

    OCI_Cleanup();
}

bool OCI_Wrapper::Execute(const QString &cmd)
{// 执行Sql
        return(OCI_ExecuteStmt(m_st, cmd.toStdString ().c_str ()));
}

void OCI_Wrapper::Commit()
{// 提交修改
    if(m_conn) OCI_Commit(m_conn);
    else UtilAPB::MyDebug (QString("m_conn fald!"));
}

// get last id from a table
int OCI_Wrapper::GetLastId(QString strTableName, QString strIdName)
{
    OCI_Wrapper::GetInstance()->Execute(QString("select max(%1) as id from %2").arg(strIdName,strTableName));
    OCI_Resultset* rs = OCI_GetResultset(m_st);

    int nLastId = 0;
    if(OCI_FetchNext(rs))
        {
        int nMaxId = OCI_GetInt2(rs,"id") ;
        nLastId = nMaxId + 1;
    }
    OCI_ReleaseResultsets(m_st);
    return nLastId;
}

// get random id from a table
int OCI_Wrapper::GetRandomId(QString strTableName, QString strIdName)
{
    OCI_Wrapper::GetInstance()->Execute(QString
                                        ("select %1 as id from (select * from %2 order by dbms_random.value) where rownum=1")
                                        .arg(strIdName, strTableName));
    OCI_Resultset* rs = OCI_GetResultset(m_st);

    int nRandomId = 0;
    if(OCI_FetchNext(rs))
        {
        nRandomId = OCI_GetInt2(rs,"id") ;
    }
    OCI_ReleaseResultsets(m_st);
    return nRandomId;
}

// get reg_name from cor_id
void OCI_Wrapper::GetRegNameFromRegId(int nRegId)
{
    OCI_Wrapper::GetInstance()->Execute(
                QString("select reg_name from sys_region where reg_id=%1").arg(nRegId));
    OCI_Resultset* rs = OCI_GetResultset(m_st);

    if(OCI_FetchNext(rs))
    {
        UtilAPB::MyDebug(OCI_GetString2(rs,"reg_name"));
    }
    OCI_ReleaseResultsets(m_st);
}

// get reg_id from cor_id
int OCI_Wrapper::GetRegIdFromCorId(int nCorId)
{
    OCI_Wrapper::GetInstance()->Execute(
                QString("select reg_id from sys_cors where cor_id=%1").arg(nCorId));
    OCI_Resultset* rs = OCI_GetResultset(m_st);

    int nRegId = 0;
    if(OCI_FetchNext(rs))
        {
        nRegId = OCI_GetInt2(rs,"reg_id") ;
    }
    OCI_ReleaseResultsets(m_st);
    return nRegId;
}

// get reg_id from sat_detail_id
int OCI_Wrapper::GetRegIdFromSatDetailId(int nSatDetailId)
{
    OCI_Wrapper::GetInstance()->Execute(
                QString("select reg_id from cor_sat_det where sat_detail_id=%1").arg(nSatDetailId));
    OCI_Resultset* rs = OCI_GetResultset(m_st);

    int nRegId = 0;
    if(OCI_FetchNext(rs))
        {
        nRegId = OCI_GetInt2(rs,"reg_id") ;
    }
    OCI_ReleaseResultsets(m_st);
    return nRegId;
}
