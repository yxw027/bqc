///////////////////////////////////////////////////////////////////////////////////////////////////
// OCILib包装类，包装基本操作

#ifndef SQL_WRAPPER_H
#define SQL_WRAPPER_H
#include <QString>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
class SQL_Wrapper
{
public:

    static SQL_Wrapper* GetInstance();

    // 获取表中的一个整形字段最大值
    int GetLastId(QString strTableName, QString strIdName);

    // 初始化
    bool Init(QString ip, QString sid, QString user, QString password);
    // 反初始化
    void UnInit();

    // 执行sql
    bool Execute(const QString&);
    // 提交
    void Commit();

private:
    static SQL_Wrapper* instance;         // 单例
    SQL_Wrapper();
    QSqlDatabase m_db;
    QSqlQuery m_result;           // 结果集
};

#endif // OCI_WRAPPER_H
