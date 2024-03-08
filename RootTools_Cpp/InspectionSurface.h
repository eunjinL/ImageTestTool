#pragma once
#include "InspectionBase.h"

class CInspectionSurface : public InspectionBase
{
public:
	virtual void SetParams(byte* buffer, int bufferwidth, int bufferheight, RECT roi,  int defectCode, int grayLevel, int defectSize, bool bDarkInspection, int threadindex);
	std::vector<DefectDataStruct> SurfaceInspection(bool bAbsolute, int nClassifyCode);
	std::vector<DefectDataStruct> SurfaceInspection(bool bAbsolute);
	CInspectionSurface(int nWidth, int nHeight);
};