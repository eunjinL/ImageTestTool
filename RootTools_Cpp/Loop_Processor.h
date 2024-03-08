#pragma once

#include "TypeDefines.h"
#include <math.h>
#include <ipp.h>
#include <omp.h>
#include <opencv2/opencv.hpp>

using namespace cv;

class Loop_Processor
{
public:
	//Vision
	static void Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh);

	static float TemplateMatching(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nSrcW, int nSrcH, int nTempW, int nTempH, Point ptLT, Point ptRB, Point ptStart, int nStride, int nSamplingRatio, int nHistScale);
	static float CalcMutualInforPos(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nMemW, int nMemH, int nTempW, int nTempH, Point ptLT, Point ptRB, int nStride, int nSamplingRatio, int nHistScale);
	static void CalcSubMutualInforPos(BYTE* pSrc, BYTE* pTemp, float& fMin, int& nMinX, int& nMinY, int nMemW, int nMemH, int nTempW, int nTempH, int64 roiW, int64 roiH, int nIndex, int nStride, int nSamplingRatio, int nHistShift, int nHistScale, int nThreadNum);
	static float ClacMatchingScore_RenyiE(BYTE* pSrc, BYTE* pTemp, int nMemW, int nMemH, int nTempW, int nTempH, Point ptLT, Point ptRB, int nMinX, int nMinY, int nHistShift);

};