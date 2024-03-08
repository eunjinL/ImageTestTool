#pragma once
#include "D2DPtrStruc.h"
#include "D2DChipStruc.h"

class MyClrTest
{
public:
	int testInt(int a, int b);
	void testDB();
	void test_struc_func(D2DPtrStruc reference, D2DPtrStruc Target);
	void testTrigger(D2DPtrStruc reference, D2DPtrStruc Target);
	ULONGLONG SSE_GetDiffSum4DoubleSize_256(D2DPtrStruc* Target, D2DPtrStruc* reference);
	ULONGLONG SSE_GetDiffSum4DoubleSize_256_threshold(D2DPtrStruc* Target, D2DPtrStruc* reference, byte bMean);
	void SSE_MakeABS_Proto(D2DPtrStruc * target, D2DPtrStruc* reference, D2DPtrStruc* ResultABS);
};