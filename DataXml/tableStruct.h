#ifndef TABLESTRUCT_H
#define TABLESTRUCT_H
#include<vector>
#include<DataXml/elementStruct.h>
using namespace std;

struct Table{
    vector<Object*> objects;
    void add(Object* obj);
    virtual string toXml();
};

/*struct Table{
   string toXml();
};
struct MntCorExtTable : public Table{
    vector<MntCorExt> corExts;
    void add(MntCorExt corExt);
    string toXml();
};

struct CorCalculateRecTable : public Table{
    vector<CorCalculateRec> calulateRecs;
    void add(CorCalculateRec calculateRec);
    string toXml();

};

struct CorSatDetTable :public Table{
    vector<CorSatDet> corSatDets;
    void add(CorSatDet corSatDet);
    string toXml();
}
*/
#endif // TABLESTRUCT_H
