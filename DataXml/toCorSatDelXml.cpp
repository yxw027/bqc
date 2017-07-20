#include <DataXml/elementStruct.h>
#include <DataXml/toString.h>
using namespace std;

string CorSatDet::toXml()
{
    string corSatDetXml = "<corSatDet>";
    string endCorSatDetXml = "</corSatDet>";
    string satDetailIdXml = "<satDetailId>";
    string endSatDetailIdXml = "</satDetailId>";
    string corIdXml = "<corId>";
    string endCorIdXml = "</corId>";
    string regIdXml = "<regId>";
    string endRegIdXml = "</regId>";
    string satCdXml = "<satCd>";
    string endSatCdXml = "</satCd>";
    string satChannelXml = "<satChannel>";
    string endSatChannelXml = "</satChannel>";
    string satStatusXml = "<satStatus>";
    string endSatStatusXml = "</satStatus>";
    string satTypeXml = "<satType>";
    string endSatTypeXml = "</satType>";
    string satHeightXml = "<satHeight>";
    string endSatHeightXml = "</satHeight>";
    string satAzimuthXml = "<satAzimuth>";
    string endSatAzimuthXml = "</satAzimuth>";
    string satSignalXml = "<satSignal>";
    string endSatSignalXml = "</satSignal>";
    string shamRangeXml = "<shamRange>";
    string endShamRangeXml = "</shamRange>";
    string shamRangePrecXml = "<shamRangePrec>";
    string endShamRangePrecXml = "</shamRangePrec>";
    string createTimeXml = "<createTime>";
    string endCreateTimeXml = "</createTime>";

    satDetailIdXml.append(ltos(satDetailId)).append(endSatDetailIdXml);
    corIdXml.append(ltos(corId)).append(endCorIdXml);
    regIdXml.append(ltos(regId)).append(endRegIdXml);
    satCdXml.append(satCd).append(endSatCdXml);
    satStatusXml.append(satStatus).append(endSatStatusXml);
    satTypeXml.append(ltos(satType)).append(endSatTypeXml);
    satHeightXml.append(dtos(satHeight)).append(endSatHeightXml);
    satAzimuthXml.append(dtos(satAzimuth)).append(endSatAzimuthXml);
    satSignalXml.append(dtos(satSignal)).append(endSatSignalXml);
    satChannelXml.append(ltos(satChannel)).append(endSatChannelXml);
    shamRangeXml.append(dtos(shamRange)).append(endShamRangeXml);
    shamRangePrecXml.append(dtos(shamRangePrec)).append(endShamRangePrecXml);
    createTimeXml.append(createTime).append(endCreateTimeXml);

    corSatDetXml.append(satDetailIdXml)
            .append(corIdXml)
            .append(regIdXml)
            .append(satCdXml)
            .append(satStatusXml)
            .append(satTypeXml)
            .append(satHeightXml)
            .append(satAzimuthXml)
            .append(satSignalXml)
            .append(satChannelXml)
            .append(shamRangeXml)
            .append(shamRangePrecXml)
            .append(createTimeXml)
            .append(endCorSatDetXml);

    /*if(satChannel != null){
        corSatDetXml.append(satChannelXml).append(satChannel).append(endSatChannelXml);
    }
    if(shamRange != 0.0){
        corSatDetXml.append(shamRangeXml).append(dtos(shamRange)).append(endShamRangeXml);
    }
    if(shamRange != 0.0){
        corSatDetXml.append(shamRangePrecXml).append(dtos(shamRangePrec)).append(endShamRangePrecXml);
    }*/

    return corSatDetXml;

}
