#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include "Constants.h"
#include "PitSizer.h"
#include "Functions.h"
#include "TypeDefines.h"
#include <vector>


class InspectionBase
{
public :
	void CopyImageToBuffer(bool bDark);
	void CheckConditions() const;
	static int defectIdx;
protected:
	int m_nInspOffset = 10;
	byte inspbuffer[4000][4000];
	//byte inspbuffer2[16000000];
	RECT inspbufferROI;
	RECT GetInspbufferROI() const;
	DefectDataStruct GetDefectData(RECT rt, POINT ptDPos, float nArea);
public:
	virtual void SetParams();
	inline void SetDefectCode(int nValue)
	{
		assert(0 <= nValue && nValue < Constants::MaxDefectCode);
		nDefectCode = nValue;
	};
	inline void SetGrayLevel(int nValue)
	{
		assert(0 <= nValue && nValue < Constants::MaxGrayLevel);
		nGrayLevel = nValue;
	};
	inline void SetDefectSize(int nValue)
	{
		assert(0 < nValue && nValue < Constants::MaxDefectSize);
		nDefectSize = nValue;
	};
	inline void SetROI(RECT roi)
	{
		assert(roi.left <= 0 || roi.top <= 0 
			|| Functions::GetWidth(roi) < Constants::MaxROI
			|| Functions::GetHeight(roi) < Constants::MaxROI);

		rtROI = roi;
	}
	inline void SetBuffer(byte* pValue)
	{
		assert(pValue != nullptr);
		pBuffer = pValue;
	}
	inline void SetBufferWidthHeight(int w,int h)
	{
		//assert(0 < w && w < Constants::MaxBufferSize);
		nBufferW = w;
		
		//assert(0 < h && h < Constants::MaxBufferSize);
		nBufferH = h;
	}

	void SetIsDarkInspection(bool bValue);
	void SetResult(bool bValue);

	RECT GetROI() const;

	bool GetAbsoulteGrayLevel() const;
	bool GetLengthInspection() const;
	bool GetDarkInspection() const;

	int GetDefectCode() const;
	int GetGrayLevel() const;
	int GetDefectSize() const;
	int GetDefectSizeMax() const;
	int GetDefectSizeMin() const;

	bool GetResult() const;
	byte* GetBuffer() const;
	byte* GetBuffer(const int x, const int y, const int width) const;
	PitSizer* GetPitsizer() const;
	byte** ppBuffer;
private:
	PitSizer* pPitSizer;
	byte* pBuffer;
	int nBufferW;
	int nBufferH;
	

	bool bResult;
	int dbcount;

private:
	RECT rtROI;

	int nDefectCode;
	int nGrayLevel;
	int nDefectSize;
	int nDefectSizeMin;
	int nDefectSizeMax;

	bool bAbsoulteGrayLevel;
	bool bLengthInspection;
	bool bDarkInspection;

public:
	InspectionBase(INT64 nWidth, INT64 nHeight);
	virtual ~InspectionBase();
};
