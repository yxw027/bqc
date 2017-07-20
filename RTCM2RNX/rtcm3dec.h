///////////////////////////////////////////////////////////////////////////////////////////////////
// Rtcm3解码器包装类

#ifndef RTCM3DEC_H
#define RTCM3DEC_H

#include "def_convrnx.h"
#include "def_DataQC.h"
#include "OracleWrapper/corsdbmanager.h"

class Rtcm3Dec
{
public:
    Rtcm3Dec();
    // 逐字节解码
    int Decode(char iData);
private:
    T_RTCM m_tRtcm;
};

#endif // RTCM3DEC_H
