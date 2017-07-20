#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string MntCorExt::toXml(){
    string mntCorExtElement = "<mntCorExt>";
    string endMntCorExtElement = "</mntCorExt>";
    string corExtIdElement = "<corExtId>";
    string endCorExtIdElement = "</corExtId>";
    string corIdElement = "<corId>";
    string endCorIdElement = "</corId>";
    string regIdElement = "<regId>";
    string endRegIdElement = "</regId>";
    string corExtTypeElement = "<corExtType>";
    string endExtTypeElement = "</corExtType>";
    string corExtValueElement = "<corExtValue>";
    string endCorExtValueElement = "</corExtValue>";
    string createTimeElement = "<createTime>";
    string endCreateTimeElement = "</createTime>";

    corExtIdElement.append(ltos(corExtId)).append(endCorExtIdElement);
    corIdElement.append(ltos(corId)).append(endCorIdElement);
    regIdElement.append(ltos(regId)).append(endRegIdElement);
    corExtTypeElement.append(corExtType).append(endExtTypeElement);
    corExtValueElement.append(corExtValue).append(endCorExtValueElement);
    createTimeElement.append(createTime).append(endCreateTimeElement);

    mntCorExtElement.append(corExtIdElement)
            .append(corIdElement)
            .append(regIdElement)
            .append(corExtTypeElement)
            .append(corExtValueElement)
            .append(createTimeElement)
            .append(endMntCorExtElement);

   // cout<<mntCorExtElement;

    return mntCorExtElement;

}
