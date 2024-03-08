#pragma once

#include "pch.h"
#include <ppl.h>
#include <vector>
#include <stack>

#include "SIMD_Processor.h"
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

void SIMD_Processor::Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh)
{
    __m256i threshold = _mm256_set1_epi8(static_cast<char>(nThresh));
    __m256i thresholdVal = _mm256_set1_epi8(255);
    __m256i SignedBit = _mm256_set1_epi8(-128); // Signed bit: 10000000
    __m256i ZeroData = _mm256_setzero_si256();

    LPBYTE pResult = pDst;
    LPBYTE pSrcImg = pSrc;

    __m256i* pRstH;
    __m256i* pInH;

    __m256i In_High, In_Low;
    __m256i CheckNegative_High, CheckNegative_Low;
    __m256i InDiff_High, InDiff_Low;
    __m256i InputReg;

    int blockEndWidth = nDstW / 32;
    int blockEndHeight = nDstH;
    int Width2 = nDstW % 32;

    for (int r = 0; r < blockEndHeight; r++)
    {
        pInH = (__m256i*)(pSrcImg);
        pRstH = (__m256i*)(pResult);

        for (int c = 0; c < blockEndWidth; c++, pRstH++, pInH++)
        {
            InputReg = _mm256_load_si256(pInH);

            In_High = _mm256_unpackhi_epi8(InputReg, ZeroData);
            In_Low = _mm256_unpacklo_epi8(InputReg, ZeroData);

            if (bDark) {
                InDiff_High = _mm256_sub_epi8(In_High, threshold);
                InDiff_Low = _mm256_sub_epi8(In_Low, threshold);
            }
            else {
                InDiff_High = _mm256_sub_epi8(threshold, In_High);
                InDiff_Low = _mm256_sub_epi8(threshold, In_Low);
            }

            // 음수면 16bit가 모두 1111 1111 1111 1111, 양수면 0000 0000 0000 0000
            CheckNegative_High = _mm256_cmpeq_epi8(SignedBit, _mm256_and_si256(SignedBit, InDiff_High));
            CheckNegative_Low = _mm256_cmpeq_epi8(SignedBit, _mm256_and_si256(SignedBit, InDiff_Low));

            In_High = _mm256_and_si256(CheckNegative_High, thresholdVal);
            In_Low = _mm256_and_si256(CheckNegative_Low, thresholdVal);

            _mm256_storeu_si256(pRstH, _mm256_packus_epi16(In_Low, In_High));
        }

        if (Width2 != 0) {
            InputReg = _mm256_load_si256(pInH);

            In_High = _mm256_unpackhi_epi8(InputReg, ZeroData);
            In_Low = _mm256_unpacklo_epi8(InputReg, ZeroData);

            if (bDark) {
                InDiff_High = _mm256_sub_epi8(In_High, threshold);
                InDiff_Low = _mm256_sub_epi8(In_Low, threshold);
            }
            else {
                InDiff_High = _mm256_sub_epi8(threshold, In_High);
                InDiff_Low = _mm256_sub_epi8(threshold, In_Low);
            }

            CheckNegative_High = _mm256_cmpeq_epi8(SignedBit, _mm256_and_si256(SignedBit, InDiff_High));
            CheckNegative_Low = _mm256_cmpeq_epi8(SignedBit, _mm256_and_si256(SignedBit, InDiff_Low));

            In_High = _mm256_and_si256(CheckNegative_High, thresholdVal);
            In_Low = _mm256_and_si256(CheckNegative_Low, thresholdVal);

            In_High = _mm256_packus_epi16(In_Low, In_High);

            for (int c = 0; c < Width2; c++)
                pResult[c + blockEndWidth * 32] = In_High.m256i_i8[c];
        }

        pSrcImg += nDstW;
        pResult += nDstW;
    }
}

std::vector<LabeledData> SIMD_Processor::Labeling(BYTE* pSrc, BYTE* pBin, int nSrcW, int nSrcH, bool bDark)
{
    std::vector<LabeledData> vtLabeled;
    int nMergeDist = 1;

    cv::Point* pitsizerDir = new cv::Point[(nMergeDist * 2 + 1) * (nMergeDist * 2 + 1)];
    int count = 0;

    for (int y = -nMergeDist; y <= nMergeDist; y++)
    {
        for (int x = -nMergeDist; x <= nMergeDist; x++, count++)
        {
            pitsizerDir[count].x = x;
            pitsizerDir[count].y = y;
        }
    }

    __m256i* pInH = (__m256i*)pBin;
    __m256i InputReg, vcmp;

    int nBinW = nSrcW; // 임시로 Source width를 Binary width로 사용
    int nBinH = nSrcH; // 임시로 Source height를 Binary height로 사용

    uint64 nArrLen = (uint64)((uint64)nBinW * nBinH) / 32;

    for (uint64 i = 0; i < nArrLen; i++, pInH++)
    {
        InputReg = _mm256_load_si256(pInH);
        vcmp = _mm256_cmpeq_epi8(InputReg, _mm256_setzero_si256());

        if (_mm256_testz_si256(vcmp, _mm256_set1_epi32(0xFFFF)) == 0)
            continue;

        for (int j = 0; j < 32; j++)
        {
            int r = (uint64)(((uint64)i * 32) + j) / nBinW;
            int c = (uint64)(((uint64)i * 32) + j) % nBinW;

            if (pBin[((int64)r * nBinW) + c] == 0)
                continue;

            int nROIL = 0; // 임시로 0으로 설정
            int nROIT = 0; // 임시로 0으로 설정

            LabeledData tmpDefect = SIMD_Processor::Labeling_PitSizer(pSrc, pBin, pitsizerDir, cv::Point(nROIL, nROIT), cv::Point(c, r), nSrcW, nSrcH, nBinW, nBinH, bDark);
            int defectLen = (tmpDefect.width < tmpDefect.height) ? tmpDefect.width : tmpDefect.height;
            if (defectLen < 99999)
                vtLabeled.push_back(tmpDefect);
        }
    }

    return vtLabeled;
}
LabeledData SIMD_Processor::Labeling_PitSizer(BYTE* pSrc, BYTE* pBin, cv::Point* pitsizerDir, cv::Point ROILT, cv::Point ptSearch, int nSrcW, int nSrcH, int nBinW, int nBinH, bool bDark)
{
    Mat imgSrc = Mat(nBinH, nBinW, CV_8UC1, pBin);

    int nMergeDist = 1;

    cv::Rect rtPit = cv::Rect(ptSearch.x, ptSearch.y, 1, 1);

    bool isEscape = false, isFound = false;
    int nSearchX, nSearchY;
    int nChainLen = (nMergeDist * 2 + 1) * (nMergeDist * 2 + 1);
    int nPitGV_Min;
    int nPitGV_Max;
    int dirIdx = 0;
    int nPitSize = 0;
    // Search Start!
    nPitGV_Min = pSrc[(uint64)(ptSearch.y + ROILT.y) * nSrcW + ptSearch.x + ROILT.x];
    nPitGV_Max = pSrc[(uint64)(ptSearch.y + ROILT.y) * nSrcW + ptSearch.x + ROILT.x];

    nSearchX = ptSearch.x;
    nSearchY = ptSearch.y;

    // Defect탐색 경로의 History
    std::vector<cv::Point> vtPath;
    vtPath.push_back(cv::Point(nSearchX, nSearchY));

    pBin[((uint64)nSearchY * nBinW) + nSearchX] = 0;

    do
    {
        dirIdx = 0;
        isFound = false;

        for (int iter = dirIdx; iter < nChainLen; iter++, dirIdx++)
        {
            if (nSearchX + pitsizerDir[dirIdx].x < 0 || nSearchY + pitsizerDir[dirIdx].y < 0)
                continue;

            if (nSearchX + pitsizerDir[dirIdx].x >= nBinW || nSearchY + pitsizerDir[dirIdx].y >= nBinH)
                continue;

            if (pBin[(uint64)(nSearchY + pitsizerDir[dirIdx].y) * nBinW + nSearchX + pitsizerDir[dirIdx].x] == 0)
                continue;

            // GV 계산시 Dark는 min, Bright는 max
            nPitGV_Min = (nPitGV_Min > pSrc[(uint64)(nSearchY + pitsizerDir[dirIdx].y + ROILT.y) * nSrcW + nSearchX + pitsizerDir[dirIdx].x + ROILT.x]) ? pSrc[(uint64)(nSearchY + pitsizerDir[dirIdx].y + ROILT.y) * nSrcW + nSearchX + pitsizerDir[dirIdx].x + ROILT.x] : nPitGV_Min;
            nPitGV_Max = (nPitGV_Max < pSrc[(uint64)(nSearchY + pitsizerDir[dirIdx].y + ROILT.y) * nSrcW + nSearchX + pitsizerDir[dirIdx].x + ROILT.x]) ? pSrc[(uint64)(nSearchY + pitsizerDir[dirIdx].y + ROILT.y) * nSrcW + nSearchX + pitsizerDir[dirIdx].x + ROILT.x] : nPitGV_Max;

            pBin[((uint64)nSearchY * nBinW) + nSearchX] = 0; // 하위 8비트에 어디까지 탐색했는지 저장

            nSearchX = nSearchX + pitsizerDir[dirIdx].x;
            nSearchY = nSearchY + pitsizerDir[dirIdx].y;

            vtPath.push_back(cv::Point2i(nSearchX, nSearchY));
            isFound = true;

            nPitSize++;
            // 탐색된 주변 픽셀위치에서 다시 탐색하기 위해 반복문 탈출!
            break;
        }

        if (!isFound)
        {
            if (vtPath.size() == 1)
                isEscape = true;
            else
            {
                // History에서 현재 위치 삭제하고 탐색 위치를 이전 픽셀 위치로 이동
                vtPath.pop_back();
                nSearchX = vtPath.back().x;
                nSearchY = vtPath.back().y;
            }
        }

        if (vtPath.size() > 100000)
        {
            vtPath.clear();
            LabeledData data;
            data.width = 99999;
            data.height = 99999;
            return data;
        }

        if (nSearchX > rtPit.x + rtPit.width)
        {
            rtPit.width = nSearchX - rtPit.x;
        }
        else if (nSearchX < rtPit.x)
        {
            rtPit.x = nSearchX;
        }
        if (nSearchY > rtPit.y + rtPit.height)
        {
            rtPit.height = nSearchY - rtPit.y;
        }
        else if (nSearchY < rtPit.y)
        {
            rtPit.y = nSearchY;
        }
    } while (!isEscape);

    LabeledData data;
    data.bound = { rtPit.x, rtPit.y, rtPit.x + rtPit.width, rtPit.y + rtPit.height };
    data.width = rtPit.width;
    data.height = rtPit.height;
    data.centerX = rtPit.x + rtPit.width / 2;
    data.centerY = rtPit.y + rtPit.height / 2;
    data.area = (data.width > data.height) ? data.width : data.height;
    data.value = (bDark) ? nPitGV_Min : nPitGV_Max;

    return data;
}