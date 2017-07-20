/********************************************************************
* 版权所有（C）2016，广州海格通信集团股份有限公司
*
* 文件名称：def_DataQC.h
* 内容摘要：电离层残差探测周跳
* 其他说明：
* 版本  号：
* 作    者：
* 完成日期：2016-12-23
*
* 修改记录1：
*    修改日期：
*    版本  号：
*    修改  人：
*    修改内容：
* 修改记录2：
********************************************************************/

#ifndef DEF_DATAQC_H
#define DEF_DATAQC_H
#include "def_convrnx.h"

void InitialQC(void);
T_SatState* DataQC(const T_OBSSAT& obs);
void DataQC(T_RTCM *ptRtcm, FILE **pFileCS=nullptr);
void Cycleslip(ObserDataEpoch *pObsEpoch, FILE **pFileCS=nullptr);
double satwavelen(int sat, int frq, int iGFrq);
double satFreq(int sat, int frq, int iGFrq);
void IonoRes_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **pFileCS=nullptr);
void HMW_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **pFileCS=nullptr);
void EWL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **pFileCS=nullptr);
void WL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **pFileCS=nullptr);
void NL_DD(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **pFileCS=nullptr);
void CycleslipRepair_DD(ObserDataEpoch *pObsEpoch, INT8U satid, INT32S deltaN[],FILE **pFileCS=nullptr);
void CycleslipRepair_TD(ObserDataEpoch *pObsEpoch, INT8U satid, INT32S deltaN[],FILE **pFileCS=nullptr);
void MPAnalysOneSat(ObserDataEpoch *pObsEpoch, INT8U satid, FILE **ofp=nullptr);
void MatMul(FP64 A[],FP64 B[],INT32U iM,INT32U iN,INT32U iK,FP64 C[]);
INT32S MatInv(FP64 A[],INT32U m,FP64 B[]);

#endif
