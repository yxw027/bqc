#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string CorCalculateRec::toXml(){

    string corCalculateRecXml = "<corCalculateRec>";
    string endCorCalculateRecXml = "</corCalculateRec>";
    string corCalcIdXml = "<corCalcId>";
    string endCorCalcIdXml = "</corCalcId>";
    string corIdXml = "<corId>";
    string endCorIdXml = "</corId>";
    string regIdXml = "<regId>";
    string endRegIdXml = "</regId>";
    string calcStateXml = "<calcState>";
    string endCalcStateXml = "</calcState>";
    string satCountXml = "<satCount>";
    string endSatCountXml = "</satCount>";
    string pointLongXml = "<pointLong>";
    string endPointLongXml = "</pointLong>";
    string pointLatXml = "<pointLat>";
    string endPointLatXml = "</pointLat>";
    string pointHeightXml = "<pointHeight>";
    string endPointHeightXml = "</pointHeight>";
    string wgsXXml = "<wgsX>";
    string endWgsXXml = "</wgsX>";
    string wgsYXml = "<wgsY>";
    string endWgsYXml = "</wgsY>";
    string wgsZXml = "<wgsZ>";
    string endWgsZXml = "</wgsZ>";
    string diffXXml = "<diffX>";
    string endDiffXXml = "</diffX>";
    string diffYXml = "<diffY>";
    string endDiffYXml = "</diffY>";
    string diffZXml = "<diffZ>";
    string endDiffZXml = "</diffZ>";
    string satSignalXml = "<satSignal>";
    string endSatSignalXml = "</satSignal>";
    string signalPreXml = "<signalPre>";
    string endsignalPreXml = "</signalPre>";
    string dataErrorPreXml = "<dataErrorPre>";
    string endDataErrorPreXml = "</dataErrorPre>";
    string goodPerXml = "<goodPer>";
    string endGoodPerXml = "</goodPer>";
    string singlePreXml = "<singlePre>";
    string endSinglePreXml = "</singlePre>";
    string shamRangeXml = "<shamRange>";
    string endShamRangeXml = "</shamRange>";
    string createTimeXml = "<createTime>";
    string endCreateTimeXml = "</createTime>";

    corCalcIdXml.append(dtos(corCalcId)).append(endCorCalcIdXml);
    corIdXml.append(dtos(corId)).append(endCorIdXml);
    regIdXml.append(dtos(regId)).append(endRegIdXml);
    calcStateXml.append(itos(calcState)).append(endCalcStateXml);
    satCountXml.append(dtos(satCount)).append(endSatCountXml);
    pointLongXml.append(dtos(pointLong)).append(endPointLongXml);
    pointLatXml.append(dtos(pointLat)).append(endPointLatXml);
    pointHeightXml.append(dtos(pointHeight)).append(endPointHeightXml);
    wgsXXml.append(dtos(wgsX)).append(endWgsXXml);
    wgsYXml.append(dtos(wgsY)).append(endWgsYXml);
    wgsZXml.append(dtos(wgsZ)).append(endWgsZXml);
    diffXXml.append(dtos(diffX)).append(endDiffXXml);
    diffYXml.append(dtos(diffY)).append(endDiffYXml);
    diffZXml.append(dtos(diffZ)).append(endDiffZXml);
    satSignalXml.append(dtos(satSignal)).append(endSatSignalXml);
    signalPreXml.append(dtos(signalPre)).append(endsignalPreXml);
    dataErrorPreXml.append(dtos(dataErrorPre)).append(endDataErrorPreXml);
    goodPerXml.append(dtos(goodPer)).append(endGoodPerXml);
    singlePreXml.append(dtos(singlePre)).append(endSinglePreXml);
    shamRangeXml.append(dtos(shamRange)).append(endShamRangeXml);
    createTimeXml.append(createTime).append(endCreateTimeXml);

    corCalculateRecXml.append(corCalcIdXml)
            .append(corIdXml)
            .append(regIdXml)
            .append(calcStateXml)
            .append(satCountXml)
            .append(pointLongXml)
            .append(pointLatXml)
            .append(pointHeightXml)
            .append(wgsXXml)
            .append(wgsYXml)
            .append(wgsZXml);
    corCalculateRecXml.append(diffXXml)
            .append(diffYXml)
            .append(diffZXml)
            .append(satSignalXml)
            .append(signalPreXml)
            .append(dataErrorPreXml)
            .append(goodPerXml)
            .append(singlePreXml)
            .append(shamRangeXml)
            .append(createTimeXml)
            .append(endCorCalculateRecXml);

    return corCalculateRecXml;

}
