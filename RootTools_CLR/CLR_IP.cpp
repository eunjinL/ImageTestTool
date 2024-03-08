#include "pch.h"

#include "CLR_IP.h"
#include <msclr\marshal_cppstd.h>

#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4793)

namespace RootTools_CLR
{
	// Vision
	void CLR_IP::Cpp_Threshold(Cpp_ThresholdParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0]; // pin : 주소값 고정
		pin_ptr<byte> pDst = &param->pDst[0];
		
		if (param->processMode == "IPP") {
			IPP_Processor::Threshold(pSrc, pDst, param->nSrcW, param->nSrcH, param->bDark, param->nThresh);
		}
		else if (param->processMode == "OpenCV") {
			OpenCV_Processor::Threshold(pSrc, pDst, param->nSrcW, param->nSrcH, param->bDark, param->nThresh);
		}
		else if (param->processMode == "SIMD") {
			SIMD_Processor::Threshold(pSrc, pDst, param->nSrcW, param->nSrcH, param->bDark, param->nThresh);
		}
		else if (param->processMode == "Loop") {
			Loop_Processor::Threshold(pSrc, pDst, param->nSrcW, param->nSrcH, param->bDark, param->nThresh);
		}

		pSrc = nullptr;  // unpin
		pDst = nullptr;
	}
	float CLR_IP::Cpp_TemplateMatching(Cpp_TemplateParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0]; // pin : 주소값 고정
		pin_ptr<byte> pTemp = &param->pDst[0];
		Point Pos;
		float score;

		if (param->processMode == "IPP") {
			score = IPP_Processor::TemplateMatching(pSrc, pTemp, Pos, param->nSrcW, param->nSrcH, param->nTempW, param->nTempH, Point(param->nROIL, param->nROIT), Point(param->nROIR, param->nROIB), param->method, param->nByteCnt, param->nChIdx);
		}
		else if (param->processMode == "OpenCV") {
			score = OpenCV_Processor::TemplateMatching(pSrc, pTemp, Pos, param->nSrcW, param->nSrcH, param->nTempW, param->nTempH, Point(param->nROIL, param->nROIT), Point(param->nROIR, param->nROIB), param->method, param->nByteCnt, param->nChIdx, param->nFilterMethod);
		}
		else if (param->processMode == "Loop") {
			score = Loop_Processor::TemplateMatching(pSrc, pTemp, Pos, param->nSrcW, param->nSrcH, param->nTempW, param->nTempH, Point(param->nROIL, param->nROIT), Point(param->nROIR, param->nROIB), Point(param->nStartX, param->nStartY), param->nStride, param->nSamplingRatio, param->nHistScale);
		}

		param->outPosX = Pos.x;
		param->outPosY = Pos.y;

		pSrc = nullptr;  // unpin
		pTemp = nullptr;
		
		return score;
	}
	array<Cpp_LabelParam^>^ CLR_IP::Cpp_Labeling(Cpp_LabelingParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0]; // pin : 주소값 고정
		pin_ptr<byte> pBin = &param->pDst[0];

		std::vector<LabeledData> vtLabeled;

		if (param->processMode == "OpenCV") {
			vtLabeled = OpenCV_Processor::Labeling(pSrc, pBin, param->nSrcW, param->nSrcH, param->bDark);
		}
		else if (param->processMode == "SIMD") {
			vtLabeled = SIMD_Processor::Labeling(pSrc, pBin, param->nSrcW, param->nSrcH, param->bDark);
		}

		array<Cpp_LabelParam^>^ local = gcnew array<Cpp_LabelParam^>(vtLabeled.size());

		pSrc = nullptr;  // unpin
		pBin = nullptr;

		bool bResultExist = vtLabeled.size() > 0;

		if (bResultExist)
		{
			for (int i = 0; i < vtLabeled.size(); i++)
			{
				local[i] = gcnew Cpp_LabelParam();
				local[i]->centerX = vtLabeled[i].centerX;
				local[i]->centerY = vtLabeled[i].centerY;

				local[i]->width = vtLabeled[i].width;
				local[i]->height = vtLabeled[i].height;

				local[i]->boundTop = vtLabeled[i].bound.top;
				local[i]->boundBottom = vtLabeled[i].bound.bottom;
				local[i]->boundLeft = vtLabeled[i].bound.left;
				local[i]->boundRight = vtLabeled[i].bound.right;

				local[i]->area = vtLabeled[i].area;
				local[i]->value = vtLabeled[i].value;

				local[i]->extraData = vtLabeled[i].extraData;
			}
		}
		return local;
	}

	// Filtering 
	void CLR_IP::Cpp_GaussianBlur(Cpp_GaussianParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0];
		pin_ptr<byte> pDst = &param->pDst[0];

		if (param->processMode == "IPP") {
			IPP_Processor::GaussianBlur(pSrc, pDst, param->nSrcW, param->nSrcH, param->nSig);
		}
		else if (param->processMode == "OpenCV") {
			OpenCV_Processor::GaussianBlur(pSrc, pDst, param->nSrcW, param->nSrcH, param->nSig);
		}

		pSrc = nullptr;
		pDst = nullptr;
	}
	void CLR_IP::Cpp_AverageBlur(Cpp_ImageProcessParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0];
		pin_ptr<byte> pDst = &param->pDst[0];

		if (param->processMode == "IPP") {
			IPP_Processor::AverageBlur(pSrc, pDst, param->nSrcW, param->nSrcH);
		}
		else if (param->processMode == "OpenCV") {
			OpenCV_Processor::AverageBlur(pSrc, pDst, param->nSrcW, param->nSrcH);
		}

		pSrc = nullptr;
		pDst = nullptr;
	}
	void CLR_IP::Cpp_MedianBlur(Cpp_MedianParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0];
		pin_ptr<byte> pDst = &param->pDst[0];

		if (param->processMode == "IPP") {
			IPP_Processor::MedianBlur(pSrc, pDst, param->nSrcW, param->nSrcH, param->nFiltSz);
		}
		else if (param->processMode == "OpenCV") {
			OpenCV_Processor::MedianBlur(pSrc, pDst, param->nSrcW, param->nSrcH, param->nFiltSz);
		}

		pSrc = nullptr;
		pDst = nullptr;
	}
	void CLR_IP::Cpp_Morphology(Cpp_MorphologyParam^ param)
	{
		pin_ptr<byte> pSrc = &param->pSrc[0];
		pin_ptr<byte> pDst = &param->pDst[0];

		std::string MorpOp = msclr::interop::marshal_as<std::string>(param->strMethod->ToString());

		if (param->processMode == "IPP") {
			IPP_Processor::Morphology(pSrc, pDst, param->nSrcW, param->nSrcH, param->nFiltSz, MorpOp, param->nIter);
		}
		else if (param->processMode == "OpenCV") {
			OpenCV_Processor::Morphology(pSrc, pDst, param->nSrcW, param->nSrcH, param->nFiltSz, MorpOp, param->nIter);
		}

		pSrc = nullptr;
		pDst = nullptr;
	}
}