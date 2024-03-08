#pragma once

// standary library
#include <windows.h>
#include <vector>

// User Type Defines
typedef struct _LabeledData
{
	double centerX;
	double centerY;
	RECT bound;
	double width;
	double height;
	double area;
	double value;
	double valueMin;
	double valueMax;
	void* extraData;
} LabeledData;

typedef struct _DefectDataStruct
{
	LONG nIdx;
	LONG nClassifyCode;
	float fAreaSize;
	LONG nLength;
	LONG nWidth;
	LONG nHeight;
	LONG nInspMode;
	LONG nFOV;
	LONG GV;
	double fPosX;
	double fPosY;
} DefectDataStruct;

typedef struct _SubMutualInforData
{
	int m_nIndex;
	int m_nStartRow;
	int m_nEndRow;
	BOOL m_bDone;
	float fMin;
	int nMinX;
	int nMinY;
}SubMutualInforData;
// namespace;


