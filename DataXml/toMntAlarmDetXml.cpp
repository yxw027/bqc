#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string MntAlarmDet :: toXml(){

    string mntAlarmDetElement = "<mntAlarmDet>";
    string endMntAlarmDetElement = "</mntAlarmDet>";
    string alarmDetailIdElement = "<alarmDetailId>";
    string endAlarmDetailIdElement = "</alarmDetailId>";
    string alarmDescElement = "<alarmDesc>";
    string endAlarmDescElement = "</alarmDesc>";
    string alarmTypeElement = "<alarmType>";
    string endAlarmTypeElement = "</alarmType>";
    string alarmLevelElement = "<alarmLevel>";
    string endAlarmLevelElement = "</alarmLevel>";
    string objectIdElement = "<objectId>";
    string endObjectIdElement = "</objectId>";
    string objectTypeElement = "<objectType>";
    string endObjectTypeElement = "</objectType>";
    string objectValueElement = "<objectValue>";
    string endObjectValueElement = "</objectValue>";
    string regIdElement = "<regId>";
    string endRegIdElement = "</regId>";
    string createTimeElement = "<createTime>";
    string endCreateTimeElement = "</createTime>";

    alarmDetailIdElement.append(ltos(alarmDetailId)).append(endAlarmDetailIdElement);
    alarmDescElement.append(alarmDesc).append(endAlarmDescElement);
    alarmTypeElement.append(alarmType).append(endAlarmTypeElement);
    alarmLevelElement.append(alarmLevel).append(endAlarmLevelElement);
    objectIdElement.append(ltos(objectId)).append(endObjectIdElement);
    objectTypeElement.append(objectType).append(endObjectTypeElement);
    objectValueElement.append(objectValue).append(endObjectValueElement);
    regIdElement.append(ltos(regId)).append(endRegIdElement);
    createTimeElement.append(createTime).append(endCreateTimeElement);

    mntAlarmDetElement.append(alarmDetailIdElement)
            .append(alarmDescElement)
            .append(alarmTypeElement)
            .append(alarmLevelElement)
            .append(objectIdElement)
            .append(objectTypeElement)
            .append(objectValueElement)
            .append(regIdElement)
            .append(createTimeElement)
            .append(endMntAlarmDetElement);

    return mntAlarmDetElement;


}
