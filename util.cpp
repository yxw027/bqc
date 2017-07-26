#include "util.h"
#include <QDateTime>
#include <stdio.h>
#include <QDebug>

UtilAPB::UtilAPB()
{
}

// 自定义调试输出
void UtilAPB::MyDebug(const char* msg)
{
#ifdef MY_DEBUG
    int nLen = strlen(msg);
    char strMsgSend[nLen + 3];
    memcpy(strMsgSend, msg, nLen);
    strMsgSend[nLen] = '\r';
    strMsgSend[nLen+1] = '\n';
    strMsgSend[nLen+2] = '\0';
    printf(strMsgSend);
#endif
}
void UtilAPB::MyDebug(const QString msg)
{
#ifdef MY_DEBUG
    qDebug()<<msg;
#endif
}

void UtilAPB::MyDebug(const QSqlError &msg)
{
#ifdef MY_DEBUG
    qDebug()<<msg;
#endif
}

int UtilAPB::ConnectTcpServer(const char *ipServer, const int port)
{
    struct sockaddr_in serverAddr;
    int nRet = 0;

    /* 创建套接字描述符 */
    int nFd = socket(AF_INET,SOCK_STREAM,0);
    if (-1 == nFd)
    {
        perror("socket err:");
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ipServer);                   //IP
    serverAddr.sin_port = htons(port);                                              //端口

    /* 和服务器端建立连接 */
    nRet = connect(nFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if (nRet == -1)
    {
        printf("connect faild!");
        return -1;
    }

    return nFd;
}

// get a random value in a range
int UtilAPB::RandomRange(int min, int max)
{
    if(min >= max)
        return 0;
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int range = max - min;
    int ret = min + qrand()%range;
    return ret;
}
