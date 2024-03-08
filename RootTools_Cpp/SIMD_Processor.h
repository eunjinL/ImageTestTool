#pragma once
#include "TypeDefines.h"
#include <math.h>
#include <ipp.h>
#include <omp.h>
#include <opencv2/opencv.hpp>

using namespace cv;

class SIMD_Processor
{
public:
	//Vision
	static void Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh);

	static std::vector<LabeledData> Labeling(BYTE* pSrc, BYTE* pBin, int nSrcW, int nSrcH, bool bDark);
	static LabeledData Labeling_PitSizer(BYTE* pSrc, BYTE* pBin, cv::Point* pitsizerDir, Point ROILT, Point ptSearch, int nSrcW, int nSrcH, int nBinW, int nBinH, bool bDark);

};