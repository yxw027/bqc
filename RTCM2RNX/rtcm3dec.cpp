#include "rtcm3dec.h"
#include <util.h>

Rtcm3Dec::Rtcm3Dec()
{
    memset(&m_tRtcm,0,sizeof(T_RTCM));
    Init_RTCM(&m_tRtcm);
    InitialQC();
    m_tRtcm.tTime=YMD2WeekSec({2017,7,20,0,0,0});
}

int Rtcm3Dec::Decode(char iData)
{
    INT32S iMark;
    static COR_SAT_ALL rtcm_cor_dat;
    // 从RTCM中读取的原始数据，主要为MSM4中的卫星数据
    if ((iMark=Input_RTCM3(&m_tRtcm,(unsigned char)iData)) > 0)
    {
        if(iMark == 1 && m_tRtcm.iObsflag)  // 1s msm complete
        {
            InitialQC();//初始化周跳探测
            rtcm_cor_dat.nCorId=m_tRtcm.iStaid;
            rtcm_cor_dat.nCorState=m_tRtcm.iStah;
            for(int i = 0; i < m_tRtcm.tObs.iN; i++)
            {
                COR_SAT_DET cor_sat_det;
                const T_OBSSAT &obsSat = m_tRtcm.tObs.ptData[i];
                for(int ch = 0; ch < 3; ch++)
                {
                    if(obsSat.dCNR[ch] != 0)    // 该频点是否有值
                    {
                        cor_sat_det.SatChannel=ch+1;
                        cor_sat_det.shamRange = obsSat.dP[ch]; /*伪距观测值 (m)*/
                        cor_sat_det.shamRangePrec = obsSat.fD[ch];// 多普勒观测值能算出伪距率
                        cor_sat_det.tch[ch].dCNR= obsSat.dCNR[ch];  /*载噪比*/
                        cor_sat_det.tch[ch].dL= obsSat.dL[ch]; /*载波观测值 (cycle)*/
                        cor_sat_det.tch[ch].dP= obsSat.dP[ch]; /*伪距观测值 (m)*/
                        cor_sat_det.tch[ch].fD= obsSat.fD[ch];// 多普勒观测值能算出伪距率
                        cor_sat_det.tch[ch].SatChannel= obsSat.dFrq[ch];//
                        // GPS time to UTC time, sub 18 seconds ,2017
                        const T_YMD &ymd = WeekSec2YMD(TimeAdd(obsSat.tObsTime, -18));
                        cor_sat_det.dUTCTime =round(ymd.iHor * 10000 + ymd.iMin * 100 + ymd.dSec);
                    }
                }
                cor_sat_det.nSatNum = Get_Num_From_RTCM_Num(obsSat.iSat);
                cor_sat_det.nSatType = Get_Type_From_RTCM_Num(obsSat.iSat);
                T_SatState* tmp=DataQC(m_tRtcm.tObs.ptData[i]);
                memcpy (&cor_sat_det.tssCal,tmp,sizeof(T_SatState));
                rtcm_cor_dat.vSatDets.push_back(cor_sat_det);
            }
            rtcm_cor_dat.dUTCTime=rtcm_cor_dat.vSatDets[0].dUTCTime;
            m_tRtcm.iObsflag = 0;
            g_rtcm_cache_cor_dat.push(rtcm_cor_dat);
            rtcm_cor_dat.clear();
            return 1;
        }
        else if(iMark == 5) // station imformation
        {// 1008 get nothing - -!
            int idSta = m_tRtcm.iStaid;
            return 0;
        }
    }
    return 0;
}
