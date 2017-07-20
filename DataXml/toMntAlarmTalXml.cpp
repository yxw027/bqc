#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string MntAlarmTal :: toXml(){

    string mntAlarmTalElement = "<mntAlarmTal>";
    string endMntAlarmTalElement = "</mntAlarmTal>";
    string alarmTotalIdElement = "<alarmTotalId>";
    string endAlarmTotalIdElement = "</alarmTotalId>";
    string alarmLevelElement = "<alarmLevel>";
    string endAlarmLevelElement = "</alarmLevel>";
    string objectIdElement = "<objectId>";
    string endObjectIdElement = "</objectId>";
    string objectTypeElement = "<objectType>";
    string endObjectTypeElement = "</objectType>";
    string countNumberElement = "<countNumber>";
    string endCountNumberElement = "</countNumber>";
    string regIdElement = "<regId>";
    string endRegIdElement = "</regId>";
    string createTimeElement = "<createTime>";
    string endCreateTimeElement = "</createTime>";

    alarmTotalIdElement.append(ltos(alarmTotalId)).append(endAlarmTotalIdElement);
    alarmLevelElement.append(alarmLevel).append(endAlarmLevelElement);
    objectIdElement.append(ltos(objectId)).append(endObjectIdElement);
    objectTypeElement.append(objectType).append(endObjectTypeElement);
    countNumberElement.append(ltos(countNumber)).append(endCountNumberElement);
    regIdElement.append(ltos(regId)).append(endRegIdElement);
    createTimeElement.append(createTime).append(endCreateTimeElement);

    mntAlarmTalElement.append(alarmTotalIdElement)
            .append(alarmLevelElement)
            .append(objectIdElement)
            .append(objectTypeElement)
            .append(countNumberElement)
            .append(regIdElement)
            .append(createTimeElement)
            .append(endMntAlarmTalElement);

    return mntAlarmTalElement;
}
