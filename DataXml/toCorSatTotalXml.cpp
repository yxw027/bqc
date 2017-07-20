#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string CorSatTotal :: toXml(){

    string corSatTotalElement = "<corSatTotal>";
    string endCorSatTotalElement = "</corSatTotal>";
    string satTotalIdElement = "<satTotalId>";
    string endSatTotalIdElement = "</satTotalId>";
    string objectIdElement = "<objectId>";
    string endObjectIdElement = "</objectId>";
    string objectLevelElement = "<objectLevel>";
    string endObjectLevelElement = "</objectLevel>";
    string regIdElement = "<regId>";
    string endRegIdElement = "</regId>";
    string visSatIntervalElement = "<visSatInterval>";
    string endVisSatIntervalElement = "</visSatInterval>";
    string visSatValueElement = "<visSatValue>";
    string endVisSatValueElement = "</visSatValue>";
    string createTimeElement = "<createTime>";
    string endCreateTimeElement = "</createTime>";

    satTotalIdElement.append(ltos(satTotalId)).append(endSatTotalIdElement);
    objectIdElement.append(ltos(objectId)).append(endObjectIdElement);
    objectLevelElement.append(objectLevel).append(endObjectLevelElement);
    regIdElement.append(ltos(regId)).append(endRegIdElement);
    visSatIntervalElement.append(visSatInterval).append(endVisSatIntervalElement);
    visSatValueElement.append(visSatValue).append(endVisSatValueElement);
    createTimeElement.append(createTime).append(endCreateTimeElement);

    corSatTotalElement.append(satTotalIdElement)
            .append(objectIdElement)
            .append(objectLevelElement)
            .append(regIdElement)
            .append(visSatIntervalElement)
            .append(visSatValueElement)
            .append(createTimeElement)
            .append(endCorSatTotalElement);

    return corSatTotalElement;
}
