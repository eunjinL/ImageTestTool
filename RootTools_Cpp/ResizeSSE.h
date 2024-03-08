#pragma once
#include "windows.h"
#include<intrin.h>
#define MAX_FOV 16000

class ResizeSSEData
{
public:
	ResizeSSEData();
	~ResizeSSEData(void);
	byte m_bimg[MAX_FOV];
	byte m_aimg[MAX_FOV];
	__m128i* pa, * pb, * pRst;
	__m128i a, b, ah, bh, al, bl;
	__m128i* pX, X, * pX2, X2;
	__m128i addh, addl, subhi, sublo, mulhi, mullo, shifth, shiftl, add;

};
#define SSEINTERPOLATION_THREADNUM 18
class ResizeSSE
{
public:
	void CreatInterpolationData(double dXScale, double dXShift, int nWidth);
	void ProcessInterpolation(int thid, LPBYTE pSrc, int nSrcHeight, int nSrcWidth, int nFovWidth, LPBYTE* ppTarget, int nXOffset, int nYOffset, int nDir, int nSy, int nEy);
	void ProcessInterpolation(int thid, LPBYTE pSrc, int nSrcHeight, int nSrcWidth, int nFovWidth, LPBYTE pTarget);

	short m_xa8[MAX_FOV];
	int m_gugan[MAX_FOV]; //일일히 대입 안하고 구간별로 멤카피를 해야 빨라서 그거 미리 계산해놓는곳
	int m_gugansize[MAX_FOV];

	__m128i ZeroData = _mm_setzero_si128();
	ResizeSSEData m_SID[SSEINTERPOLATION_THREADNUM];
};

