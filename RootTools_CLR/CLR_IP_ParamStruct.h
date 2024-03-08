#pragma once

// standary library
#include <windows.h>
#include <vector>

// User Type Defines
public ref class Cpp_LabelParam
{
public:
	float centerX;
	float centerY;

	int boundTop;
	int boundBottom;
	int boundLeft;
	int boundRight;

	float width;
	float height;

	float area;
	float value;
	float valueMin;
	float valueMax;

	//Defect이 어떤 검사에서 나온 결과인지 확인
	void* extraData;
};

public ref class Cpp_Point
{
public:
	int x;
	int y;

	Cpp_Point() { }
	Cpp_Point(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
};

public ref class Cpp_Rect
{
public:
	int x; // left
	int y; // top
	int h;
	int w;
};

public ref class Cpp_ImageProcessParam
{
public:
	array<byte>^ pSrc;
	array<byte>^ pDst;
	int nSrcW;
	int nSrcH;
	System::String^ processMode;
};

public ref class Cpp_TemplateParam : public Cpp_ImageProcessParam
{
public:
	int outPosX;
	int outPosY;
	int nTempW;
	int nTempH;
	int nROIL; int nROIT;
	int nROIR; int nROIB;

	int method;
	int nByteCnt;
	int nChIdx;
	int nFilterMethod;

	int nStartX; int nStartY;
	int nStride;
	int nSamplingRatio;
	int nHistScale;
};

public ref class Cpp_ThresholdParam : public Cpp_ImageProcessParam
{
public:
	bool bDark;
	int nThresh;
};

public ref class Cpp_LabelingParam : public Cpp_ImageProcessParam
{
public:
	bool bDark;
};

public ref class Cpp_GaussianParam : public Cpp_ImageProcessParam
{
public:
	int nSig;
};
public ref class Cpp_MedianParam : public Cpp_ImageProcessParam
{
public:
	int nFiltSz;
};
public ref class Cpp_MorphologyParam : public Cpp_ImageProcessParam
{
public:
	int nFiltSz;
	System::String^ strMethod;
	int nIter;
};