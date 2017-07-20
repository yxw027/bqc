#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string CorStatusRec :: toXml(){

    string corStatusRecElement = "<corStatusRec>";
    string endCorstatusRecElement = "</corStatusRec>";
    string corStatusIdElement = "<corStatusId>";
    string endCorStatusIdElement = "</corStatusId>";
    string corIdElement = "<corId>";
    string endCorIdElement = "</corId>";
    string regIdElement = "<regId>";
    string endRegIdElement = "</regId>";
    string corStatusElement = "<corStatus>";
    string endCorStatusElement = "</corStatus>";
    string corAlarmLevelElement = "<corAlarmLevel>";
    string endCorAlarmLevelElement = "</corAlarmLevel>";
    string createTimeElement = "<createTime>";
    string endCreateTimeElement= "</createTime>";

    corStatusIdElement.append(ltos(corStatusId)).append(endCorStatusIdElement);
    corIdElement.append(ltos(corId)).append(endCorIdElement);
    regIdElement.append(ltos(regId)).append(endRegIdElement);
    corStatusElement.append(corStatus).append(endCorStatusElement);
    corAlarmLevelElement.append(corAlarmLevel).append(endCorAlarmLevelElement);
    createTimeElement.append(createTime).append(endCreateTimeElement);

    corStatusRecElement.append(corStatusIdElement)
            .append(corIdElement)
            .append(regIdElement)
            .append(corStatusElement)
            .append(corAlarmLevelElement)
            .append(createTimeElement)
            .append(endCorstatusRecElement);

    return corStatusRecElement;
}
