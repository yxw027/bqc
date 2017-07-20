#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include <arpa/inet.h>
#define MY_DEBUG
//APB公共操作类封装
class UtilAPB
{
private:
    UtilAPB();
public:

    // 自定义调试输出
    static void MyDebug(const char* msg);
    static void MyDebug(const QString msg);
    // 在一定范围内获取一个随机数
    static int RandomRange(int min, int max);

    // 通过服务器ip和端口连接Tcp服务器，返回socket句柄
    static int ConnectTcpServer(char* ipServer, int port);
};

#endif // UTIL_H
