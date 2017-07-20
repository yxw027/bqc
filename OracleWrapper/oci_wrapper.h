///////////////////////////////////////////////////////////////////////////////////////////////////
// OCILib包装类，包装基本操作

#ifndef OCI_WRAPPER_H
#define OCI_WRAPPER_H
#include <QString>
#include <ocilib.h>

class OCI_Wrapper
{
public:
    virtual ~OCI_Wrapper();

    static OCI_Wrapper* GetInstance();

    // 随机获取表中的一个整形字段值
    int GetRandomId(QString strTableName, QString strIdName);
    // 获取表中的一个整形字段最大值
    int GetLastId(QString strTableName, QString strIdName);
    // 通过Cor站Id获取区域Id
    int GetRegIdFromCorId(int nCorId);
    // 通过SatDetailId获取区域Id
    int GetRegIdFromSatDetailId(int nSatDetailId);

    void GetRegNameFromRegId(int nRegId);

    // 初始化
    bool Init(QString ip, QString sid, QString user, QString password, QString port = "1521");
    // 反初始化
    void UnInit();

    // 执行sql
    bool Execute(const QString&);
    // 提交
    void Commit();


private:
    static OCI_Wrapper* instance;         // 单例
    OCI_Wrapper();
    OCI_Connection* m_conn = NULL;  // 数据库连接
    OCI_Statement* m_st = NULL;         //
    OCI_Resultset* m_rs = NULL;           // 结果集
};

#endif // OCI_WRAPPER_H
