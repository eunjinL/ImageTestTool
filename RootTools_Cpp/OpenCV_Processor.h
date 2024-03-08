#pragma once

#include "TypeDefines.h"
#include <math.h>
#include <ipp.h>
#include <omp.h>
#include <opencv2/opencv.hpp>

using namespace cv;

class OpenCV_Processor
{
public:
	// Vision
	static void Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh);
	static float TemplateMatching(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nSrcW, int nSrcH, int nTempW, int nTempH, Point ptLT, Point ptRB,
		int method, int nByteCnt, int nChIdx, int nFilterMethod);
	static std::vector<LabeledData> Labeling(BYTE* pSrc, BYTE* pBin, int nSrcW, int nSrcH, bool bDark);

	// Filtering
	static void GaussianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nSigma);
	static void AverageBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH);
	static void MedianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFiltSz);
	static void Morphology(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFiltSz, std::string strMethod, int nIter);
};