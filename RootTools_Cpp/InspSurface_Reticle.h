#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include "PitSizer.h"
//#include "Cpp_DB.h"


class InspSurface_Reticle
{
public:
	InspSurface_Reticle(int processornum);
	PitSizer* m_pPitSizer;
	//Cpp_DB* m_DBmgr;


	int TestFunc(POINT A);
	int Width(RECT rt);
	int Height(RECT rt);

	//RECT abc;

	//VisionWokrs2 pattern surface 검사 선언. vs2에서는 pop mask 한 후 mask를 아래 함수로 넘겨줌
		
	bool InspRect_Dark(int nDCode, RECT rtROI, bool bABSGV, int nDarkLevel, int nDarkSize,
		POINT ptDPos, BOOL bLengthInsp, BOOL bInterpolation);
	void CopyImageToBuffer(byte* mem, int nW, RECT rtR, int nBackGround, BOOL bInterpolation);
	int CalHPPatternGV(int nBandwidth, int nIntensity, int nW, int nH);
	//bool AddDefect(int nDCode, RECT rt, POINT ptDPos, float fSize);

	POINT m_ptCurrent;

	/*void OpenDB();
	void CloseDB();
	bool WriteLastFlag();
	bool WriteDB(int DCode, RECT rt, float size);*/

protected:
	int m_Histogram[256];
	int m_nInspOffset = 10;
	LPBYTE m_pImageBuffer;
	LPBYTE m_pImageBuffer2;
	LPBYTE* m_ppImageBuffer;
	LPBYTE* m_ppImageBuffer2;
		
	//buffer크기 parameter화 동적할당 작업 필요
	byte testbyte[2000][2000];

	// FOR TEST 아래 변수들 모두 vega recipe 내장화 해야함
	double m_nPatternInterpolationOffset = 2.0;
	bool m_bPatternUseInterpolation = false;//second interpolation

	std::string DBFolderPath = "C:/vsdb";
	int dbcount;
	int ProcessorNum;
};



