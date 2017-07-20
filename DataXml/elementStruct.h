#ifndef ELEMENTSTRUCT_H
#define ELEMENTSTRUCT_H
#include<string>
using namespace std;

struct Object
{
    virtual string toXml() = 0;
};

struct CorCalculateRec: public Object{
    long corCalcId;
    long corId;
    long regId;
    int calcState;
    double satCount;
    double pointLong;
    double pointLat;
    double pointHeight;
    double wgsX;
    double wgsY;
    double wgsZ;
    double diffX;
    double diffY;
    double diffZ;
    string createTime;
    double satSignal;
    double signalPre;
    double dataErrorPre;
    double goodPer;
    double singlePre;
    double shamRange;

    virtual string toXml();
};

struct CorSatDet :public Object{
    long satDetailId;
    long corId;
    long regId;
    string satCd;
    long satChannel;
    string satStatus;
    long satType;
    double satHeight;
    double satAzimuth;
    double satSignal;
    double shamRange;
    double shamRangePrec;
    string createTime;

    virtual string toXml();
};

struct CorSatTotal : public Object{
    long satTotalId;
    string objectLevel;
    long objectId;
    long regId;
    string visSatInterval;
    string visSatValue;
    string createTime;

    virtual string toXml();
};

struct CorStatusRec : public Object{
    long corStatusId;
    long corId;
    long regId;
    string corStatus;
    string corAlarmLevel;
    string createTime;

    virtual string toXml();
};

struct MntAlarmDet : public Object{
    long alarmDetailId;
    long regId;
    string objectType;
    long objectId;
    string alarmLevel;
    string alarmType;
    string objectValue;
    string alarmDesc;
    string createTime;

    virtual string toXml();
};

struct MntAlarmTal : public Object{
    long alarmTotalId;
    long regId;
    string objectType;
    long objectId;
    string alarmLevel;
    long countNumber;
    string createTime;

    virtual string toXml();
};

struct MntTimestamp : public Object{
    string updateTimestamp;

    virtual string toXml();
};

struct MntCorExt: public Object {
    long corExtId;
    long corId;
    long regId;
    string corExtType;
    string corExtValue;
    string createTime;

   virtual string toXml();
};

#endif // ELEMENTSTRUCT_H
