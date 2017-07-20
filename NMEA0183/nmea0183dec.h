///////////////////////////////////////////////////////////////////////////////////////////////////
// Nmea0183解码器包装类

#ifndef NMEA0183DEC_H
#define NMEA0183DEC_H

#include <QString>

class Nmea0183Dec
{
private:
    QString m_strNMEA;

public:
    Nmea0183Dec();
    // 逐字节解码
    bool Decode(char iData);
};

#endif // NMEA0183DEC_H
