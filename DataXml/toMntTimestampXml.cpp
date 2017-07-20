#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string MntTimestamp ::toXml(){
    string mntTimestampElement = "<mntTimestamp>";
    string endMntTimestampElement = "</mntTimestamp>";
    string updateTimestampElement = "<updateTimestamp>";
    string endUpdateTimestampElement = "</updateTimestamp>";

    updateTimestampElement.append(updateTimestamp).append(endUpdateTimestampElement);

    mntTimestampElement.append(updateTimestampElement).append(endMntTimestampElement);

    return mntTimestampElement;
}
