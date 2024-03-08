#pragma once

#include "windows.h"



class PitSizer 
{
public:
	PitSizer(int, int nMaskSize);
	virtual ~PitSizer();

private:
	int m_nChainLength;
	int m_nMaskSize;
	int m_nMaxPixelNum;
	POINT *dir;
	POINT *Path, *Pixels, *m_pptDefectMap;
	int PathNum;
	float PitSize, DiscolorSize;
	RECT m_rcPitRect;
	int m_nGV;

public:
	RECT GetPitRect();
	int GetGV();
	int GetDefectMapLength();
	POINT* GetDefectMapPath();
	int m_nMaxDefectMapLength;
	int m_nDefectMapLength;
	PitSizer();
	POINT* GetPixelPointer();
	int GetPixelNum();
	int m_nPixelNum;
	POINT* GetPath();
	int GetPathNum();
	float GetPitSize(byte* mem, int x, int y, int nW, int RefGV, int PitValue, int nIndex, bool bGatherDefectMap);
	void MakeDir(void);
};