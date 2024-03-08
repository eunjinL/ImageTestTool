
#include "pch.h"
#include <ppl.h>
#include <vector>
#include <stack>

#include "IPP_Processor.h"

#include <string>
#include <memory>
#include <cstdio>
#include <iostream>
#include <sstream>

#pragma warning(disable: 4101)
#pragma warning(disable: 4244)
#pragma warning(disable: 4838)
#pragma warning(disable: 6297)
#pragma warning(disable: 26451)

// Vision
void IPP_Processor::Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh)
{
    // Calculate strides
    int stride_src = nDstW;
    int stride_dst = nDstW;

    // Define ROI size
    IppiSize roiSize = { nDstW, nDstH };

    // Convert threshold value to Ipp8u
    Ipp8u thresholdValue = static_cast<Ipp8u>(nThresh);

    IppStatus status = ippStsNoErr;

    if (bDark)
    {
        status = ippiThreshold_LTVal_8u_C1R(pSrc, stride_src, pDst, stride_dst, roiSize, thresholdValue, 1);
    }
    else
    {
        status = ippiThreshold_GTVal_8u_C1R(pSrc, stride_src, pDst, stride_dst, roiSize, thresholdValue, 1);
    }

    if (status != ippStsNoErr)
    {
        // Handle error
    }
}
float IPP_Processor::TemplateMatching(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nSrcW, int nSrcH, int nTempW, int nTempH,
    Point ptLT, Point ptRB, int method, int nByteCnt, int nChIdx)
{
    int64 roiW = (ptRB.x - (int64)ptLT.x);
    int64 roiH = (ptRB.y - (int64)ptLT.y);

    IppiSize featureSize = { nTempW, nTempH };
    IppiSize sourceSize = { roiW, roiH };

    int nFeatureStep = featureSize.width * sizeof(Ipp8u);
    int nFeatureSize = featureSize.width * featureSize.height;
    int nSrcStep = sourceSize.width * sizeof(Ipp8u);
    int nSrcSize = sourceSize.width * sourceSize.height;

    int dstStep = (sourceSize.width) * sizeof(Ipp32f);
    Ipp8u* pFeature = new Ipp8u[nFeatureSize];
    Ipp8u* pippSrc = new Ipp8u[nSrcSize];
    Ipp32f* pDst = NULL;

    // 버퍼의 크기 계산
    int bufferSize;
    ippiCrossCorrNormGetBufferSize(sourceSize, featureSize, ippAlgHintAccurate, &bufferSize);
    // 버퍼 할당
    Ipp8u* pBuffer = new Ipp8u[bufferSize];

    for (int64 r = ptLT.y; r < ptRB.y; r++)
    {
        BYTE* pImg = &pSrc[r * nSrcW + ptLT.x];
        memcpy(&pippSrc[roiW * (r - (int64)ptLT.y)], pImg, roiW);
    }

    for (int64 r = 0; r < featureSize.height; r++)
    {
        BYTE* pImg = &pTemp[r * nTempW];
        memcpy(&pFeature[r * nTempW], pImg, featureSize.width);
    }

    Ipp32f fMax = 0;
    int nMaxX = 0;
    int nMaxY = 0;

    if (nByteCnt == 1)
    {
        pDst = new Ipp32f[nSrcSize];
        ippiCrossCorrNorm_8u32f_C1R(pippSrc, nSrcStep, sourceSize, pFeature, nFeatureStep, featureSize, pDst, dstStep, ippAlgHintAccurate, pBuffer);

        ippiMaxIndx_32f_C1R(pDst, dstStep, sourceSize, &fMax, &nMaxX, &nMaxY);
        outMatchPoint = Point(nMaxX - featureSize.width / 2, nMaxY - featureSize.height / 2);
    }

    delete[] pippSrc;
    delete[] pFeature;
    delete[] pDst;
    delete[] pBuffer;

    int score = (int)(fMax * 100);
    return score == 0 ? 1 : score;
}

// Filtering
void IPP_Processor::GaussianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nSigma = 1)
{
    int nStep = nW * sizeof(Ipp8u);
    IppiSize ROISize = { nW - 4, nH - 4 };
    IppiPoint anchor = { 2, 2 };
    Ipp8u* ippSrc = (Ipp8u*)(pSrc + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));
    Ipp8u* ippDst = (Ipp8u*)(pDst + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));

    // 필터링에 필요한 초기화 작업 수행
    IppFilterGaussianSpec* pSpec;
    Ipp8u* pBuffer;
    int specSize, bufferSize;
    ippiFilterGaussianGetSpecSize(ippMskSize5x5, ipp8u, 1, &specSize, &bufferSize);
    ippiFilterGaussianGetBufferSize(ROISize, ippMskSize5x5, ipp8u, 1, &specSize, &bufferSize);
    pSpec = (IppFilterGaussianSpec*)ippsMalloc_8u(specSize);
    pBuffer = ippsMalloc_8u(bufferSize);
    ippiFilterGaussianInit(ROISize, 5, 1.0f, ippBorderRepl, ipp8u, 1, pSpec, pBuffer);

    // 가우시안 필터링 적용
    ippiFilterGaussian_8u_C1R(ippSrc, nStep, ippDst, nStep, ROISize, ippBorderRepl, nullptr, pSpec, pBuffer);

    // 사용한 자원 해제
    ippsFree(pSpec);
    ippsFree(pBuffer);

    /*ippiFilterGauss_8u_C1R(ippSrc, nStep, ippDst, nStep, ROISize, ippMskSize5x5);
    IPPAPI(IppStatus, ippiFilterGauss_8u_C1R, (const Ipp8u * pSrc, int srcStep,
        Ipp8u * pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask))*/
}
void IPP_Processor::AverageBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH)
{
    int nStep = nW * sizeof(Ipp8u);
    IppiSize ROISize = { nW - 2, nH - 2 };
    IppiSize maskSize = { 3,3 };
    IppiPoint anchor = { 1, 1 };
    Ipp8u* ippSrc = (Ipp8u*)(pSrc + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));
    Ipp8u* ippDst = (Ipp8u*)(pDst + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));

    // 필요한 버퍼의 크기 함수로 계산
    int bufferSize;
    ippiFilterBoxBorderGetBufferSize(ROISize, maskSize, ipp8u, 1, &bufferSize);

    // 필요한 버퍼를 할당
    Ipp8u* pBuffer = ippsMalloc_8u(bufferSize);

    // BoxBorder 적용 
    ippiFilterBoxBorder_8u_C1R(ippSrc, nStep, ippDst, nStep, ROISize, maskSize, ippBorderRepl, nullptr, pBuffer);

    // 사용한 버퍼 해제
    ippsFree(pBuffer);
}
void IPP_Processor::MedianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFilterSz)
{
    int nStep = nW * sizeof(Ipp8u);
    IppiSize ROISize = { nW - (nFilterSz - 1), nH - (nFilterSz - 1) };
    IppiSize maskSize = { nFilterSz, nFilterSz };
    IppiPoint anchor = { (nFilterSz - 1) / 2, (nFilterSz - 1) / 2 };
    Ipp8u* ippSrc = (Ipp8u*)(pSrc + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));
    Ipp8u* ippDst = (Ipp8u*)(pDst + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));

    Ipp8u borderValue = 0; // 변경: 테두리 값 설정 (필요에 따라 변경)
    IppiBorderType borderType = ippBorderRepl; // 변경: 테두리 타입 설정 (필요에 따라 변경)

    int bufferSize = 0;
    ippiFilterMedianBorderGetBufferSize(ROISize, maskSize, ipp8u, 1, &bufferSize); // 변경: 필요한 버퍼 크기 계산

    Ipp8u* pBuffer = ippsMalloc_8u(bufferSize); // 변경: 계산된 크기로 버퍼 할당

    ippiFilterMedianBorder_8u_C1R(ippSrc, nStep, ippDst, nStep, ROISize, maskSize, borderType, borderValue, pBuffer); // 변경: 경계 처리가 포함된 중앙값 필터 적용

    ippsFree(pBuffer);
}
void IPP_Processor::Morphology(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFilterSz, std::string strMethod = "", int nIter = 1)
{
    byte* pTmpImg = new byte[(uint64)nW * nH];
    int nStep = nW * sizeof(Ipp8u);
    IppiSize ROISize = { nW - (nFilterSz - 1), nH - (nFilterSz - 1) };
    IppiSize maskSize = { nFilterSz, nFilterSz };
    IppiPoint anchor = { (nFilterSz - 1) / 2, (nFilterSz - 1) / 2 };
    Ipp8u* ippSrc = (Ipp8u*)(pSrc + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));
    Ipp8u* ipptmp = (Ipp8u*)(pTmpImg + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));
    Ipp8u* ippDst = (Ipp8u*)(pDst + (uint64)anchor.y * nW + (uint64)anchor.x * sizeof(Ipp8u));

    Ipp8u borderValue = 0;
    IppiBorderType borderType = ippBorderRepl;

    int bufferSize = 0;
    // 필요한 버터 계산
    ippiFilterMinBorderGetBufferSize(ROISize, maskSize, ipp8u, 1, &bufferSize);
    //버퍼 크기 할당 
    Ipp8u* pBuffer = ippsMalloc_8u(bufferSize);

    // 경계 처리 포함된 min 필터 
    ippiFilterMinBorder_8u_C1R(ippSrc, nStep, ipptmp, nStep, ROISize, maskSize, borderType, borderValue, pBuffer);
    // 경계 처리 포함된 max 필터 
    ippiFilterMaxBorder_8u_C1R(ipptmp, nStep, ippDst, nStep, ROISize, maskSize, borderType, borderValue, pBuffer);

    // 버퍼 메모리 해제
    ippsFree(pBuffer);
    delete[] pTmpImg;
}