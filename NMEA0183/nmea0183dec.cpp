#include "nmea0183dec.h"
#include <QStringList>
#include <util.h>

Nmea0183Dec::Nmea0183Dec()
{
    m_strNMEA.clear();
}

bool Nmea0183Dec::Decode(char iData)
{
    static COR_SAT_ALL nmea_cor_dat;
    if (iData=='$')
    {
        QStringList arList = m_strNMEA.mid(0, m_strNMEA.length() - 5).split(",");
        m_strNMEA.clear();
        if(arList[0].startsWith("$GNGGA"))
        {
            nmea_cor_dat.dUTCTime = arList[1].toDouble();
            nmea_cor_dat.dUTCTime = round(nmea_cor_dat.dUTCTime);
            nmea_cor_dat.dLat = arList[2].toDouble()/100;
            nmea_cor_dat.dLat = (arList[3].toLatin1() == "N") ? nmea_cor_dat.dLat : -nmea_cor_dat.dLat;
            nmea_cor_dat.dLng = arList[4].toDouble()/100;
            nmea_cor_dat.dLng = (arList[5].toLatin1() == "E") ? nmea_cor_dat.dLng : -nmea_cor_dat.dLng;
            nmea_cor_dat.nCalState = arList[6].toInt();
            nmea_cor_dat.nSatCount = arList[7].toInt();
            nmea_cor_dat.dHeight = arList[9].toDouble();
        }
        else if(arList[0].startsWith("$BDGSV") || arList[0].startsWith("$GPGSV") || arList[0].startsWith("$GLGSV"))
        {
            COR_SAT_DET cor_sat_det;
            int Offset = 4;
            int DataNum = 4;
            for(int k = 0; k < (arList.count() - Offset) / DataNum; k++)
            {
                if(arList[0].startsWith("$GPGSV"))
                {
                    cor_sat_det.nSatType = 0;
                    cor_sat_det.nSatNum = arList[Offset + k * DataNum].toInt() + 100;
                }
                else if(arList[0].startsWith("$BDGSV"))
                {
                    cor_sat_det.nSatType = 1;
                    cor_sat_det.nSatNum = arList[Offset + k * DataNum].toInt() - 160 + 200;
                }
                else if(arList[0].startsWith("$GLGSV"))
                {
                    cor_sat_det.nSatType = 2;
                    cor_sat_det.nSatNum = arList[Offset + k * DataNum].toInt() - 64 + 300;
                }
                cor_sat_det.dYangjiao = arList[Offset + k * DataNum + 1].toDouble();
                cor_sat_det.dFangweijiao = arList[Offset + k * DataNum + 2].toDouble();
                cor_sat_det.dSNR = arList[Offset + k * DataNum + 3].toDouble();
                nmea_cor_dat.vSatDets. push_back(cor_sat_det);
            }
        }
        else if(arList[0].startsWith("$GNGST"))
        {
            nmea_cor_dat.dDiffX = arList[6].toDouble();
            nmea_cor_dat.dDiffY = arList[7].toDouble();
            nmea_cor_dat.dDiffZ = arList[8].toDouble();
            g_nmea_cache_cor_dat.push(nmea_cor_dat);
            nmea_cor_dat.clear();
            return true;
        }
    }
    m_strNMEA.append(iData);
    return false;
}
