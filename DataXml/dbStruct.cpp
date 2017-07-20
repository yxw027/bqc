#include <DataXml/dbStruct.h>
#include <iostream>

void gnssDb::add(Table tab)
{
    tables.push_back(tab);
}

string gnssDb::toXml()
{
    string gnssDbXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    string sqlElement= "<gnss>";
    string endSqlElement = "</gnss>";
    gnssDbXml.append(sqlElement);
    for(vector<Table>::iterator it = tables.begin(); it != tables.end();)
    {
       gnssDbXml.append(it->toXml());
       tables.erase(it);
    }
    gnssDbXml.append(endSqlElement);
    cout<<gnssDbXml<<'\n';
    return gnssDbXml;
}
