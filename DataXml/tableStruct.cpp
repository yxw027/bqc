#include <QCoreApplication>
#include <iostream>
#include <string>
#include <DataXml/tableStruct.h>
#include <DataXml/toString.h>
using namespace std;

/*
void MntCorExtTable::add(MntCorExt corExt){
    corExts.push_back(corExt);
}

string MntCorExtTable::toXml()
{
     string mntCorExtXml = "";
     for(vector<MntCorExt>::iterator it  = corExts.begin(); it != corExts.end(); )
     {

        mntCorExtXml.append(it->toXml());
        corExts.erase(it);
     }
     cout<<mntCorExtXml<<'\n';
     return mntCorExtXml;
}

void CorCalculateRecTable::add(CorCalculateRec calculateRec){
    calulateRecs.push_back(calculateRec);
}

string CorCalculateRecTable::toXml(){
    string corCalculateRecXml = "";
    for(vector<CorCalculateRec>::iterator it = calulateRecs.begin(); it != calulateRecs.end();)
    {
        corCalculateRecXml.append(it->toXml);
        calulateRecs.erase(it);
    }
    cout<<corCalculateRecXml<<'\n';
    return corCalculateRecXml;
}

void CorSatDetTable::add(CorSatDet corSatDet){
    corSatDets.push_back(corSatDet);
}

string CorSatDetTable::toXml(){
    string corSatDetXml = "";
    for(vector<CorCalculateRec>::iterator it = corSatDets.begin(); it != corSatDets.end();){
        corSatDetXml.append(it->toXml);
        corSatDets.erase(it);
    }
    cout<<corSatDetXml<<'\n';
    return corSatDetXml;
}
*/
void Table::add(Object* obj){
    objects.push_back(obj);
}
string Table::toXml(){
    string tableXml = "";
    for(vector<Object*>::iterator it = objects.begin(); it != objects.end();){
        tableXml.append((*it)->toXml());
        objects.erase(it);
    }
    //cout<<tableXml<<'\n';
    return tableXml;
}
