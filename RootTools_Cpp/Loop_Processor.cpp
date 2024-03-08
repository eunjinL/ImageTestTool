
#include "pch.h"
#include <ppl.h>
#include <vector>
#include <stack>

#include "Loop_Processor.h"

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

void Loop_Processor::Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh)
{
    long long srcStrideOffset = nDstW;
    long long dstStrideOffset = nDstW;

    if (bDark)
    {
        for (int y = 0; y < nDstH; ++y, pSrc += srcStrideOffset, pDst += dstStrideOffset)
        {
            for (int x = 0; x < nDstW; ++x, ++pSrc, ++pDst)
            {
                *pDst = (*pSrc < nThresh) ? 1 : 0;
            }
        }
    }
    else
    {
        for (int y = 0; y < nDstH; ++y, pSrc += srcStrideOffset, pDst += dstStrideOffset)
        {
            for (int x = 0; x < nDstW; ++x, ++pSrc, ++pDst)
            {
                *pDst = (*pSrc > nThresh) ? 1 : 0;
            }
        }
    }
}

float Loop_Processor::TemplateMatching(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nSrcW, int nSrcH, int nTempW, int nTempH,
    Point ptLT, Point ptRB, Point ptStart, int nStride, int nSamplingRatio, int nHistScale)
{
    if (nStride < 1)
    {
        nStride = 1;
    }
    int nDivisorRatio = 1;
    for (int n = 2; n <= nSamplingRatio; n++)
    {
        if (nStride % n == 0)
        {
            nDivisorRatio = n;
        }
    }
    if (nHistScale < 1)
    {
        nHistScale = 1;
    }

    ptLT.x = ptStart.x - ((ptStart.x - ptLT.x) / nDivisorRatio * nDivisorRatio);
    ptLT.y = ptStart.y - ((ptStart.y - ptLT.y) / nDivisorRatio * nDivisorRatio);
    ptRB.x = (ptStart.x + nTempW) + ((ptRB.x - (ptStart.x + nTempW)) / nDivisorRatio * nDivisorRatio);
    ptRB.y = (ptStart.y + nTempH) + ((ptRB.y - (ptStart.y + nTempH)) / nDivisorRatio * nDivisorRatio);

    float fScore;
    Point ptRoughMin, ptMin;

    if (nStride > 1)
    {
        fScore = CalcMutualInforPos(pSrc, pTemp, ptRoughMin, nSrcW, nSrcH, nTempW, nTempH, ptLT, ptRB, nStride, nSamplingRatio, nHistScale);
        Point ptRoughLT = Point(ptLT.x + (ptRoughMin.x - nStride + 1), ptLT.y + (ptRoughMin.y - nStride + 1));
        Point ptRoughRB = Point(ptRoughLT.x + nTempW + nStride, ptRoughLT.y + nTempH + nStride);
        fScore = CalcMutualInforPos(pSrc, pTemp, ptMin, nSrcW, nSrcH, nTempW, nTempH, ptRoughLT, ptRoughRB, 1, 1, nHistScale);

        ptMin.x += ptRoughMin.x + (1 - nStride);
        ptMin.y += ptRoughMin.y + (1 - nStride);
    }
    else
    {
        fScore = CalcMutualInforPos(pSrc, pTemp, ptMin, nSrcW, nSrcH, nTempW, nTempH, ptLT, ptRB, nStride, 1, nHistScale);
    }

    outMatchPoint.x = ptMin.x;
    outMatchPoint.y = ptMin.y;
    return  (int)(fScore * 100);
}
float Loop_Processor::CalcMutualInforPos(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nMemW, int nMemH, int nTempW, int nTempH, Point ptLT, Point ptRB, int nStride, int nSamplingRatio, int nHistScale)
{
    int nThreadNum = 8;
    IppiSize SizeSource = { ptRB.x - ptLT.x, ptRB.y - ptLT.y };

    int nHistShift = 8 - (int)(log(nHistScale) / log(2));

    int64 roiW = (ptRB.x - (int64)ptLT.x);
    int64 roiH = (ptRB.y - (int64)ptLT.y);

    PBYTE pSourceBuffer = new BYTE[roiW * roiH];
    BYTE* pFeatureBuffer = new BYTE[nTempW * nTempH];

    BYTE* pTarget = pFeatureBuffer;
    BYTE* pFeatureCurrent = pTemp;
    for (int i = 0; i < nTempW * nTempH; i++)
    {
        *pTarget++ = *pFeatureCurrent++ / nHistScale;
    }

    BYTE* pSourceCurrent;
    pTarget = pSourceBuffer;
    for (int i = 0; i < roiH; i++)
    {
        pSourceCurrent = &pSrc[(int64)(i + ptLT.y) * (int64)nMemW + ptLT.x];
        for (int j = 0; j < roiW; j++)
        {
            *pTarget++ = *pSourceCurrent++ / nHistScale;
        }
    }

    Mat imgSrcAB = Mat(roiH, roiW, CV_8UC1, pSourceBuffer);
    Mat imgSrcB = Mat(nTempH, nTempW, CV_8UC1, pFeatureBuffer);

    float* threadMin = new float[nThreadNum] {0, };
    int* threadX = new int[nThreadNum] {0, };
    int* threadY = new int[nThreadNum] {0, };

    concurrency::parallel_for(0, nThreadNum, [&](int i)
        {
            int tmpX = 0;
            int tmpY = 0;
            float tmpMin = 9999999;

            CalcSubMutualInforPos(pSourceBuffer, pFeatureBuffer, tmpMin, tmpX, tmpY, nMemW, nMemH, nTempW, nTempH, roiW, roiH, i, nStride, nSamplingRatio, nHistShift, nHistScale, nThreadNum);

            threadMin[i] = tmpMin;
            threadX[i] = tmpX;
            threadY[i] = tmpY;
        });

    int nMinX = 0;
    int nMinY = 0;
    float fMin = 9999999;

    for (int i = 0; i < nThreadNum; i++)
    {
        if (threadMin[i] < fMin)
        {
            fMin = threadMin[i];
            nMinX = threadX[i];
            nMinY = threadY[i];
        }
    }

    // Renyi Entropy (Q = 2)를 사용하여 Matching Score 계산
    float pScore = ClacMatchingScore_RenyiE(pSrc, pTemp, nMemW, nMemH, nTempW, nTempH, ptLT, ptRB, nMinX, nMinY, nHistShift);
    outMatchPoint = Point(nMinX, nMinY);

    delete[] pFeatureBuffer;
    delete[] pSourceBuffer;
    delete[] threadMin;
    delete[] threadX;
    delete[] threadY;
    return pScore;
}

void Loop_Processor::CalcSubMutualInforPos(BYTE* pSourceBuffer, BYTE* pFeatureBuffer, float& fMin, int& nMinX, int& nMinY, int nMemW, int nMemH, int nTempW, int nTempH, int64 roiW, int64 roiH, int nIndex, int nStride, int nSamplingRatio, int nHistShift, int nHistScale, int nThreadNum)
{
    byte* pFeatureCurrent;
    byte* pSourceCurrent;

    int nHistSize = pow(2, nHistShift);
    int nJointHistSize = nHistSize * nHistSize;
    int* Histogram = new int[nHistSize];
    int* JointHistogram = new int[nJointHistSize];
    int* pHistCurrent, * pHistFinal, * pJointHistCurrent, * pJointHistFinal;

    int i = 0;
    float fVal = 0;
    float fEntropy = 0;
    float fJointEntropy = 0;
    float fMI = 0;

    int nSubRowSize = (roiH - nTempH + 1) / nThreadNum;
    int nStartRow = nIndex * nSubRowSize;
    int nEndRow = (nIndex + 1) * nSubRowSize;
    if (nIndex == nThreadNum - 1)
        nEndRow = roiH - nTempH + 1;

    int nStartCol = 0;
    int nEndCol = roiW - nTempW + 1;
    int nLastColIndex = (nTempW - 1) / nSamplingRatio * nSamplingRatio;
    for (int Y = nStartRow; Y < nEndRow; Y += nStride)
    {
        for (int X = nStartCol; X < nEndCol; X += nStride)
        {
            // Construct 1D Histogram
            if (X == 0)
            {
                memset(Histogram, 0, nHistSize * sizeof(int));
                for (int i = 0; i < nTempH; i += nSamplingRatio)
                {
                    pSourceCurrent = pSourceBuffer + (i + Y) * roiW + X;
                    for (int j = 0; j < nTempW; j += nSamplingRatio, pSourceCurrent += nSamplingRatio)
                    {
                        Histogram[*pSourceCurrent]++;
                    }
                }
            }
            else
            {
                for (int i = 0; i < nTempH; i += nSamplingRatio)
                {
                    for (int j = 0; j < nStride; j += nSamplingRatio)
                    {
                        Histogram[pSourceBuffer[(i + Y) * roiW + X - nStride + j]]--;
                        Histogram[pSourceBuffer[(i + Y) * roiW + X + nLastColIndex - j]]++;
                    }
                }
            }

            // Construct 2D Histogram
            pFeatureCurrent = pFeatureBuffer;
            memset(JointHistogram, 0, nJointHistSize * sizeof(int));
            for (int i = 0; i < nTempH; i += nSamplingRatio)
            {
                pSourceCurrent = pSourceBuffer + (i + Y) * roiW + X;
                for (int j = 0; j < nTempW; j += nSamplingRatio, pSourceCurrent += nSamplingRatio, pFeatureCurrent += nSamplingRatio)
                {
                    JointHistogram[(*pSourceCurrent << nHistShift) + *pFeatureCurrent]++;
                }
            }

            // Calculate Mutual Information
            fMI = 0;
            i = 0;
            pHistCurrent = &Histogram[0];
            pHistFinal = pHistCurrent + nHistSize;
            while (pHistCurrent < pHistFinal)
            {
                if (*pHistCurrent > 0)
                {
                    fEntropy = 0;
                    pJointHistCurrent = &JointHistogram[i * nHistSize];
                    pJointHistFinal = pJointHistCurrent + nHistSize;
                    while (pJointHistCurrent < pJointHistFinal)
                    {
                        if (*pJointHistCurrent > 0)
                        {
                            fVal = *pJointHistCurrent / (float)*pHistCurrent;
                            fEntropy += fVal * fVal;
                        }
                        pJointHistCurrent++;
                    }
                    fMI += *pHistCurrent * (1 - fEntropy);
                }
                i++;
                pHistCurrent++;
            }
            fMI /= (float)(nTempW * nTempH);

            if (fMI < fMin)
            {
                fMin = fMI;
                nMinX = X;// +(ptLT.x - ptStart.x);
                nMinY = Y;// +(ptLT.y - ptStart.y);
            }
        }
    }

    delete[] Histogram;
    delete[] JointHistogram;
}

float Loop_Processor::ClacMatchingScore_RenyiE(BYTE* pSrc, BYTE* pTemp, int nMemW, int nMemH, int nTempW, int nTempH, Point ptLT, Point ptRB, int nMinX, int nMinY, int nHistShift)
{
    int64 roiW = (ptRB.x - (int64)ptLT.x);
    int64 roiH = (ptRB.y - (int64)ptLT.y);

    PBYTE pSourceBuffer = new BYTE[roiW * roiH];
    BYTE* pFeatureBuffer = new BYTE[nTempW * nTempH];

    BYTE* pFeatureCurrent, * pFeatureFinal, * pSourceCurrent, * pSourceFinal;
    BYTE* pTargetFeature, * pTargetSrc;

    pTargetFeature = &pFeatureBuffer[0];
    pTargetSrc = &pSourceBuffer[0];

    for (int i = 0; i < nTempH; i++)
    {
        pFeatureCurrent = &pTemp[i * nTempW];
        pSourceCurrent = &pSrc[(int64)(i + ptLT.y + nMinY) * (int64)nMemW + ptLT.x + nMinX];
        pFeatureFinal = pFeatureCurrent + nTempW;

        while (pFeatureCurrent < pFeatureFinal)
        {
            *pTargetFeature++ = *pFeatureCurrent >> (8 - nHistShift);
            *pTargetSrc++ = *pSourceCurrent >> (8 - nHistShift);

            pFeatureCurrent++;
            pSourceCurrent++;
        }
    }

    int nHistSize = pow(2, nHistShift);
    int nJointHistSize = nHistSize * nHistSize;
    int* Histogram = new int[nHistSize];
    int* FeatureHistogram = new int[nHistSize];
    int* JointHistogram = new int[nJointHistSize];
    int* pHistCurrent, * pHistFinal, * pJointHistCurrent, * pJointHistFinal;

    // Construct 1D Histogram
    memset(Histogram, 0, nHistSize * sizeof(int));
    memset(FeatureHistogram, 0, nHistSize * sizeof(int));
    for (int i = 0; i < nTempH; i++)
    {
        pFeatureCurrent = &pFeatureBuffer[i * nTempW];
        pSourceCurrent = &pSourceBuffer[i * nTempW];
        pFeatureFinal = pFeatureCurrent + nTempW;

        while (pFeatureCurrent < pFeatureFinal)
        {
            Histogram[*pSourceCurrent]++;
            FeatureHistogram[*pFeatureCurrent]++;
            pSourceCurrent++;
            pFeatureCurrent++;
        }
    }

    // Construct 2D Histogram
    memset(JointHistogram, 0, nJointHistSize * sizeof(int));
    for (int i = 0; i < nTempH; i++)
    {
        pFeatureCurrent = &pFeatureBuffer[i * nTempW];
        pSourceCurrent = &pSourceBuffer[i * nTempW];
        pFeatureFinal = pFeatureCurrent + nTempW;

        while (pFeatureCurrent < pFeatureFinal)
        {
            JointHistogram[(*pSourceCurrent << nHistShift) + *pFeatureCurrent]++;
            pSourceCurrent++;
            pFeatureCurrent++;
        }
    }

    // Calculate NMI using Renyi Entropy
    float HUPi = 0.0, HVPi = 0.0, HUVPi = 0;
    float HU = 0, HV = 0, HUV = 0;

    float q = 3.0;

    for (int c = 0; c < nJointHistSize; c++)
    {
        HUVPi = JointHistogram[c] / (float)(nTempW * nTempH);
        HUV += pow(HUVPi, q);
    }

    for (int c = 0; c < nHistSize; c++)
    {
        HUPi = FeatureHistogram[c] / (float)(nTempW * nTempH);
        HVPi = Histogram[c] / (float)(nTempW * nTempH);

        HU += pow(HUPi, q);
        HV += pow(HVPi, q);
    }

    HU = (1 / (1 - q)) * log(HU);
    HV = (1 / (1 - q)) * log(HV);
    HUV = (1 / (1 - q)) * log(HUV);

    if (isnan(HU))
        HU = 0.01;
    if (isnan(HV))
        HV = 0.01;
    if (isnan(HUV))
        HUV = 0.01;


    HUV = (HU + HV - HUV) / ((HU + HV) / 2);

    delete[] pFeatureBuffer;
    delete[] pSourceBuffer;
    delete[] Histogram;
    delete[] FeatureHistogram;
    delete[] JointHistogram;

    return HUV;
}
