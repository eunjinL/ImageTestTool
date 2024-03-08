#pragma once

#include "pch.h"
#include <ppl.h>
#include <vector>
#include <stack>

#include "OpenCV_Processor.h"
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
void OpenCV_Processor::Threshold(BYTE* pSrc, BYTE* pDst, int nDstW, int nDstH, bool bDark, int nThresh)
{
    Mat imgSrc = Mat(nDstH, nDstW, CV_8UC1, pSrc);
    Mat imgDst = Mat(nDstH, nDstW, CV_8UC1, pDst);

    if (bDark)
        cv::threshold(imgSrc, imgDst, nThresh, 255, CV_THRESH_BINARY_INV);
    else
        cv::threshold(imgSrc, imgDst, nThresh, 255, CV_THRESH_BINARY);
}
//Point ptStart = Point(0, 0), int nStride = 1, int nSamplingRatio = 1, int nHistScale = 1
float OpenCV_Processor::TemplateMatching(BYTE* pSrc, BYTE* pTemp, Point& outMatchPoint, int nSrcW, int nSrcH, int nTempW, int nTempH,
    Point ptLT, Point ptRB, int method, int nByteCnt, int nChIdx, int nFilterMethod)
{
    int64 roiW = (ptRB.x - (int64)ptLT.x);
    int64 roiH = (ptRB.y - (int64)ptLT.y);

    PBYTE imgROI = new BYTE[roiW * roiH];
    for (int64 r = ptLT.y; r < ptRB.y; r++)
    {
        BYTE* pImg = &pSrc[r * nSrcW + ptLT.x];
        memcpy(&imgROI[roiW * (r - (int64)ptLT.y)], pImg, roiW);
    }

    Mat imgSrc = Mat(roiH, roiW, CV_8UC1, imgROI);
    Mat imgTemp;
    double chMax = 0;

    if (nByteCnt == 1)
    {
        imgTemp = Mat(nTempH, nTempW, CV_8UC1, pTemp);

        if (nFilterMethod == 1)
        {
            Mat f = cv::Mat(imgTemp);
            Mat s = cv::Mat(imgSrc);
            Mat fl, fl_x, fl_y, abs_fl_x, abs_fl_y, sl, sl_x, sl_y, abs_sl_x, abs_sl_y, r;
            Sobel(f, fl_x, CV_16S, 1, 0);
            Sobel(f, fl_y, CV_16S, 0, 1);
            Sobel(s, sl_x, CV_16S, 1, 0);
            Sobel(s, sl_y, CV_16S, 0, 1);
            convertScaleAbs(fl_x, abs_fl_x);
            convertScaleAbs(fl_y, abs_fl_y);
            convertScaleAbs(sl_x, abs_sl_x);
            convertScaleAbs(sl_y, abs_sl_y);
            addWeighted(abs_fl_x, 0.5, abs_fl_y, 0.5, 0, fl);
            addWeighted(abs_sl_x, 0.5, abs_sl_y, 0.5, 0, sl);

            matchTemplate(sl, fl, r, method);
            normalize(r, r, 0, 1, NORM_MINMAX, -1, Mat());
            minMaxLoc(r, NULL, &chMax, NULL, &outMatchPoint); // 완벽하게 매칭될 경우 1

            f.release();
            s.release();
            fl.release();
            sl.release();
            r.release();
        }
        else
        {
            Mat result;
            matchTemplate(imgSrc, imgTemp, result, method);
            minMaxLoc(result, NULL, &chMax, NULL, &outMatchPoint); // 완벽하게 매칭될 경우 1
        }

    }
    else if (nByteCnt == 3)
    {
        imgTemp = Mat(nTempH, nTempW, CV_8UC3, pTemp);

        Mat bgr[3];
        split(imgTemp, bgr);

        if (nFilterMethod == 1)
        {
            Mat f = cv::Mat(bgr[nChIdx]);
            Mat s = cv::Mat(imgSrc);
            Mat fl, fl_x, fl_y, abs_fl_x, abs_fl_y, sl, sl_x, sl_y, abs_sl_x, abs_sl_y, r;
            Sobel(f, fl_x, CV_16S, 1, 0);
            Sobel(f, fl_y, CV_16S, 0, 1);
            Sobel(s, sl_x, CV_16S, 1, 0);
            Sobel(s, sl_y, CV_16S, 0, 1);
            convertScaleAbs(fl_x, abs_fl_x);
            convertScaleAbs(fl_y, abs_fl_y);
            convertScaleAbs(sl_x, abs_sl_x);
            convertScaleAbs(sl_y, abs_sl_y);
            addWeighted(abs_fl_x, 0.5, abs_fl_y, 0.5, 0, fl);
            addWeighted(abs_sl_x, 0.5, abs_sl_y, 0.5, 0, sl);

            matchTemplate(sl, fl, r, method);
            normalize(r, r, 0, 1, NORM_MINMAX, -1, Mat());
            minMaxLoc(r, NULL, &chMax, NULL, &outMatchPoint); // 완벽하게 매칭될 경우 1

            f.release();
            s.release();
            fl.release();
            sl.release();
            r.release();
        }
        else
        {
            Mat result;
            matchTemplate(imgSrc, bgr[nChIdx], result, method);
            minMaxLoc(result, NULL, &chMax, NULL, &outMatchPoint); // 완벽하게 매칭될 경우 1
        }
    }

    delete[] imgROI;

    return (chMax * 100 > 1) ? chMax * 100 : 1; // Matching Score
}

std::vector<LabeledData> OpenCV_Processor::Labeling(BYTE* pSrc, BYTE* pBin, int nSrcW, int nSrcH, bool bDark)
{
    std::vector<LabeledData> vtOutLabeled;

    Mat imgSrc = Mat(nSrcH, nSrcW, CV_8UC1, pSrc);
    Mat imgBin = Mat(nSrcH, nSrcW, CV_8UC1, pBin);
    Mat imgMask;

    cv::bitwise_and(imgSrc, imgBin, imgMask);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    cv::findContours(imgBin, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    Rect bounding_rect;
    for (int i = 0; i < contours.size(); i++) // iterate through each contour. 
    {
        if (hierarchy[i][3] != -1) // Parent Contour가 있을 경우 Pass!
            continue;

        bounding_rect = boundingRect(contours[i]);
        // Defect의 GV구하는 부분 (우선은 Min, Max)
        Mat defectROI = imgMask(bounding_rect);
        Mat defectMask = imgBin(bounding_rect);

        //double area = countNonZero(defectMask);

        double min, max;
        minMaxIdx(defectROI, &min, &max, NULL, NULL, defectMask);

        LabeledData data;
        data.bound = { bounding_rect.x, bounding_rect.y, bounding_rect.x + bounding_rect.width, bounding_rect.y + bounding_rect.height };
        data.width = bounding_rect.width;
        data.height = bounding_rect.height;
        data.centerX = bounding_rect.x + (double)bounding_rect.width / 2;
        data.centerY = bounding_rect.y + (double)bounding_rect.height / 2;
        data.area = (bounding_rect.width > bounding_rect.height) ? bounding_rect.width : bounding_rect.height;
        data.value = (bDark) ? min : max;
        vtOutLabeled.push_back(data);
    }

    hierarchy.clear(); // vector의 size를 0으로 줄임
    std::vector< Vec4i>(hierarchy).swap(hierarchy); // vector의 capacity를 0으로 줄임

    contours.clear(); // vector의 size를 0으로 줄임
    std::vector<std::vector<Point>>(contours).swap(contours); // vector의 capacity를 0으로 줄임

    return vtOutLabeled;
}

// Filtering
void OpenCV_Processor::GaussianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nSigma)
{
    Mat imgSrc = Mat(nH, nW, CV_8UC1, pSrc);
    Mat imgDst = Mat(nH, nW, CV_8UC1, pDst);

    cv::GaussianBlur(imgSrc, imgDst, Size(nSigma * 6 + 1, nSigma * 6 + 1), nSigma);
}
void OpenCV_Processor::AverageBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH)
{
    Mat imgSrc = Mat(nH, nW, CV_8UC1, pSrc);
    Mat imgDst = Mat(nH, nW, CV_8UC1, pDst);

    cv::boxFilter(imgSrc, imgDst, CV_8UC1, cv::Size(3, 3), cv::Point(-1, -1), true, cv::BorderTypes::BORDER_DEFAULT);
}
void OpenCV_Processor::MedianBlur(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFilterSz)
{
    Mat imgSrc = Mat(nH, nW, CV_8UC1, pSrc);
    Mat imgDst = Mat(nH, nW, CV_8UC1, pDst);

    cv::medianBlur(imgSrc, imgDst, nFilterSz);
}
void OpenCV_Processor::Morphology(BYTE* pSrc, BYTE* pDst, int nW, int nH, int nFilterSz, std::string strMethod, int nIter)
{
    Mat dirElement(nFilterSz, nFilterSz, CV_8U, cv::Scalar(1)); // 일단 전방향 Morph, 추후 4방향 구현
    Mat imgSrc = Mat(nH, nW, CV_8UC1, pSrc);
    Mat imgDst = Mat(nH, nW, CV_8UC1, pDst);

    if (strMethod == String("erode") || strMethod == String("Erode") || strMethod == String("ERODE"))
    {
        cv::erode(imgSrc, imgDst, dirElement, cv::Point(-1, -1), nIter);
    }
    else if (strMethod == String("dilate") || strMethod == String("Dilate") || strMethod == String("DILATE"))
    {
        cv::dilate(imgSrc, imgDst, dirElement, cv::Point(-1, -1), nIter);
    }
    else if (strMethod == String("open") || strMethod == String("Open") || strMethod == String("OPEN"))
    {
        cv::morphologyEx(imgSrc, imgDst, cv::MORPH_OPEN, dirElement, cv::Point(-1, -1), nIter);
    }
    else if (strMethod == String("close") || strMethod == String("Close") || strMethod == String("CLOSE"))
    {
        cv::morphologyEx(imgSrc, imgDst, cv::MORPH_CLOSE, dirElement, cv::Point(-1, -1), nIter);
    }
}