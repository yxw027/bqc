#ifndef DBSTRUCT_H
#define DBSTRUCT_H
#include <DataXml/tableStruct.h>

struct gnssDb
{
    vector<Table> tables;
    void add(Table tab);
    string toXml();
};

#endif // DBSTRUCT_H
