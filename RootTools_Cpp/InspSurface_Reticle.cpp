#include "pch.h"
#include "InspSurface_Reticle.h"



InspSurface_Reticle::InspSurface_Reticle(int processornum)
{
	ProcessorNum = processornum;
	m_pPitSizer = new PitSizer(2048 * 2048, 1);
	//m_DBmgr = new Cpp_DB();

	dbcount = 0;

	//임시
	m_ptCurrent.x = -1;
	m_ptCurrent.y = 0;
}
int InspSurface_Reticle::TestFunc(POINT TEST)
{
	int a, b;
	
	a = TEST.x;
	
	b = TEST.y;
	
	return a + b;
}
int InspSurface_Reticle::Width(RECT rt)
{
	int width = rt.right - rt.left;

	width = abs(width);
		
	return width;
}
int InspSurface_Reticle::Height(RECT rt)
{
	int Height = rt.bottom - rt.top;

	Height = abs(Height);

	return Height;
}
void InspSurface_Reticle::CopyImageToBuffer(byte* mem, int nW, RECT rt, int nBackGround, BOOL bInterpolation)
{
	double InterOffset_ori = m_nPatternInterpolationOffset;
	double InterOffset = m_nPatternInterpolationOffset;

	//// Large Defect
	//bool bIP_2nd = m_bPatternUseInterpolation;
	//if (bIP_2nd && (m_nCurrent_Mask - m_nMask_2nd < 0))
	//	InterOffset = m_pParam->m_nPatternInterpolationOffset2;

	int nWidth = (int)(Width(rt) + m_nInspOffset * InterOffset);
	int nHeight = (int)(Height(rt) + m_nInspOffset * InterOffset);

	int nOffset = m_nInspOffset;
	LPBYTE* ppBuffer = m_ppImageBuffer2;

	if (bInterpolation)
	{
		nOffset = (int)(nOffset / InterOffset);
		ppBuffer = m_ppImageBuffer;
		nWidth = (int)(Width(rt) * InterOffset + m_nInspOffset * InterOffset);
		nHeight = (int)(Height(rt) * InterOffset + m_nInspOffset * InterOffset);
	}

	// Copy Area
	int nStart = 0, nEndX = Width(rt), nEndY = Height(rt);
	if (bInterpolation) {
		nStart = -2;
		nEndX = nEndX + 2;
		nEndY = nEndY + 2;
	}


	//std::string mmfName = "pool";
	//wstring t;
	//t.assign(mmfName.begin(), mmfName.end());
	//HANDLE hMapping;
	//hMapping = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, t.c_str());
	//LPVOID p = ::MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 10000 * 10000);

	//byte* pByte = (byte*)p;

	//gv = mem[y * nW + x];

	for (int i = nStart; i < nEndY; i++)
	{
		for (int j = nStart; j < nEndX; j++)
		{
			int ytarget = i + rt.top + (nOffset - 5) * 2;
			int xtarget = j + rt.left + (nOffset - 5) * 2;
				
			//ppBuffer[i + nOffset][nOffset + j] = mem[(y + ytarget) * nW + (x + xtarget)];  // Interpolation값이 낮아서 배열에 음수들어가면 오류
			//testbyte[i + nOffset][nOffset + j] = mem[(y + ytarget) * nW + (x + xtarget)];  // Interpolation값이 낮아서 배열에 음수들어가면 오류
			testbyte[i + nOffset][nOffset + j] = mem[(ytarget) * nW + (xtarget)];

		}
	}

	//if (bInterpolation) 
	//	CImageLib::ippiInterpolation(m_pImageBuffer, m_szBuffer.cx, m_pImageBuffer2, m_szBuffer.cx, CRect(0, 0, nWidth, nHeight), InterOffset, InterOffset);
	//}

	for (int i = 0; i < m_nInspOffset; i++) {//Jerry
		memset(testbyte[i], nBackGround, nWidth);
		memset(testbyte[nHeight - 1 - i], nBackGround, nWidth);
		for (int j = 0; j < nHeight; j++) {
			testbyte[j][i] = nBackGround;
			testbyte[j][nWidth - 1 - i] = nBackGround;
		}
	}
}

int InspSurface_Reticle::CalHPPatternGV(int nBandwidth, int nIntensity, int nW, int nH)
{
	memset(m_Histogram, 0, 256 * sizeof(int));
	int nCnt = 0;
	int nPL = 0;
	LPBYTE p;

	for (int y = m_nInspOffset; y < nH + m_nInspOffset; y++) {
		p = &testbyte[y][m_nInspOffset];
		for (int x = m_nInspOffset; x < nW + m_nInspOffset; x++, p++) {
			m_Histogram[*p]++;
		}
	}

	for (int j = 0; j < nBandwidth; j++) {
		nCnt += m_Histogram[j];
	}


	for (int i = 0; i < 254 - nBandwidth; i++) {
		if (nCnt > nIntensity) {
			nPL = i;
			i = 256;
		}
		nCnt = nCnt - m_Histogram[i];
		//nCnt -= m_Histogram[i];
		if (i + nBandwidth < 256)//Joseph 2019-01-31 예외 방지
			nCnt += m_Histogram[i + nBandwidth];
	}

	return nPL;
}
bool InspSurface_Reticle::InspRect_Dark(int nDCode, RECT rtROI, bool bABSGV, int nDarkLevel, int nDarkSize,
	POINT ptDPos, BOOL bLengthInsp, BOOL bInterpolation)
{
	LPBYTE pPos;
	float ret;
	RECT rt;
	BOOL bRst = TRUE;
	//bool bIP_2nd = m_pParam->m_bPatternUseInterpolation; // Large Defect

	int nDarkRatio = nDarkLevel;

	LONGLONG llAVG = 0;
	LONGLONG nCnt = 0;
	int nAvg;

	double InterOffset = m_nPatternInterpolationOffset;

	//// Large Defect
	//if (bIP_2nd && (m_nCurrent_Mask - m_nMask_2nd < 0))
	//{
	//	InterOffset = m_pParam->m_nPatternInterpolationOffset2;
	//}

	// 1. Cal - AVG
	llAVG = 0;
	nCnt = 0;

	if (bABSGV)
	{
		llAVG = nDarkLevel;
	}
	else
	{
		for (int y = rtROI.top; y < rtROI.bottom; y++)
		{
			pPos = &testbyte[y][rtROI.left];
			for (int x = rtROI.left; x < rtROI.right; x++, pPos++)
			{
				if (*pPos > 0)
				{
					llAVG += *pPos;
					nCnt++;
				}
			}
		}
		if (llAVG > 0 && nCnt > 0)
		{
			nAvg = (int)(llAVG / nCnt);
			nDarkLevel = (int)(nDarkRatio * 0.01 * nAvg);
		}
	}
	for (int y = rtROI.top; y < rtROI.bottom; y++)
	{
		pPos = &testbyte[y][rtROI.left];
		for (int x = rtROI.left; x < rtROI.right; x++, pPos++)
		{

			if (*pPos < nDarkLevel)
			{
				int col = sizeof(testbyte[0]) / sizeof(byte);
				ret = m_pPitSizer->GetPitSize(*testbyte, x, y, col, nDarkLevel, nDarkLevel, 1, false);
				if (bLengthInsp)
				{
					rt = m_pPitSizer->GetPitRect();
					if (Width(rt) >= nDarkSize || Height(rt) >= nDarkSize)
					{
						if (bInterpolation)
						{
							// Large Defect
							rt.left = (LONG)(rt.left / InterOffset + m_nInspOffset / InterOffset);
							rt.right = (LONG)(rt.right / InterOffset + m_nInspOffset / InterOffset);
							rt.top = (LONG)(rt.top / InterOffset + m_nInspOffset / InterOffset);
							rt.bottom = (LONG)(rt.bottom / InterOffset + m_nInspOffset / InterOffset);
							ret = (float)(ret / (InterOffset * InterOffset));

						}
						//if (!AddDefect(nDCode, rt, ptDPos, ret)) //SASSAS
						//{
						//	return false;
						//}
						bRst = false;
					}
				}
				else if (ret >= nDarkSize)
				{
					rt = m_pPitSizer->GetPitRect();

					if (bInterpolation)
					{
						rt.left = (LONG)(rt.left / InterOffset + m_nInspOffset / InterOffset);
						rt.right = (LONG)(rt.right / InterOffset + m_nInspOffset / InterOffset);
						rt.top = (LONG)(rt.top / InterOffset + m_nInspOffset / InterOffset);
						rt.bottom = (LONG)(rt.bottom / InterOffset + m_nInspOffset / InterOffset);
						ret = (float)(ret / (InterOffset * InterOffset));

					}
					else//Jerry
					{
						/*rt.left = rt.left+ m_nInspOffset;
						rt.right = rt.right + m_nInspOffset;
						rt.top = rt.top + m_nInspOffset;
						rt.bottom = rt.bottom + m_nInspOffset;*/
					}

					//Defect _tempDefect;
					//_tempDefect.v_code = nDCode;
					//_tempDefect.v_rt = rt;
					//_tempDefect.v_pt = ptDPos;
					//_tempDefect.v_ret = ret;

					//defectVector.push_back(_tempDefect);

					//구조체 defect add 나중에 구현
					/*if(!AddDefect(nDCode, rt, ptDPos, ret)) 
						return false;	*/
					bRst = false;
				}
			}
		}
	}

	//// *-SM AddDefect
	//for (int i = 0; i < defectVector.size(); i++)
	//{
	//	//AfxMessageBox("test");
	//	Defect _tempDefect;
	//	_tempDefect = defectVector[i];
	//	AddDefect(_tempDefect.v_code, _tempDefect.v_rt, _tempDefect.v_pt, _tempDefect.v_ret);
	//}
	//defectVector.clear();

	return bRst;
}
//bool InspSurface_Reticle::AddDefect(int nDCode, RECT rt, POINT ptDPos, float fSize)
//{
//	rt.left = rt.left + ptDPos.x - m_ptCurrent.x - m_nInspOffset;
//	rt.right = rt.right + ptDPos.x - m_ptCurrent.x - m_nInspOffset;
//	rt.bottom = rt.bottom + ptDPos.y - m_ptCurrent.y - m_nInspOffset;
//	rt.top = rt.top + ptDPos.y - m_ptCurrent.y - m_nInspOffset;
//
//	int nX = rt.left + (int)(Width(rt) * 0.5);
//	int nY = rt.top + (int)(Height(rt) * 0.5);
//
//	//CheckRedundancy 나중에 구현
//
//	//int nMin = 256, nMax = 0;
//	//int nBoxWidth = m_pParamSurface->m_nNonePattern_DZone;
//	//int nCheckSize = m_pParamSurface->m_nNonePattern_CheckLength;
//	//int nCheckGV = m_pParamSurface->m_nNonePattern_CheckGV;
//	//int nSX, nEX, nSY, nEY;
//	//LPBYTE p;
//
//	//CRect rtDefectZone;
//	//if (m_bNonePatternFiltering) {
//	//	// 현 불량 위치에서 상,하,좌,우의 패턴 여부 인지해서 nonepattern이 아니면 불량을 제거
//	//	// top
//	//	nSX = nX - nCheckSize;
//	//	nEX = nX + nCheckSize;
//	//	nSY = nY - nCheckSize;
//	//	nEY = nY + nCheckSize;
//
//	//	rtDefectZone = CRect(nX - nBoxWidth, nY - nBoxWidth, nX + nBoxWidth, nY + nBoxWidth);
//
//	//	for (int i = nSY; i < nEY; i++) {
//	//		p = &m_ppImage[i][nSX];
//	//		for (int j = nSX; j < nEX; j++, p++) {
//	//			if (rtDefectZone.PtInRect(CPoint(j, i)));
//	//			else if (*p < nMin)	nMin = *p;
//	//			else if (*p > nMax)	nMax = *p;
//	//		}
//	//	}
//
//	//	if (nMax - nMin > nCheckGV)
//	//		return TRUE;
//	//}
//
//	//if (CheckRedundancy(nDCode, nX, nY, rt, fSize, ptDPos)) // Xian
//	//{
//	//	// 결과값이 True 일때는 재검사를 시도함 불량이 추가 된지 안된지는 몰른다. 
//	//	return true;
//
//	//}
//	//else
//	//{
//	//	// 결과값이 false 일때는 실행을 안함
//	//}
//
//	return WriteDB(nDCode, rt, fSize);
//	//return AddDefect(nDCode, nX, nY, fSize, rt);
//}
//void InspSurface_Reticle::OpenDB()
//{
//	string temp;
//	temp = DBFolderPath + "/VSTEMP" + to_string(ProcessorNum) + ".sqlite";
//	dbcount = 0;
//	int result = CreateDirectoryA(DBFolderPath.c_str(), NULL);
//	//m_DBmgr->DBCreateVSTemp(temp);
//	//m_DBmgr->OpenAndBegin(temp);
//
//}
//void InspSurface_Reticle::CloseDB()
//{
//	dbcount = 0;
//	//WriteLastFlag();
//	//m_DBmgr->CommitAndClose();
//
//}
//bool InspSurface_Reticle::WriteLastFlag()
//{
//	char ch[500];
//
//	sprintf_s(ch, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
//		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1);
//
//	string str(ch);
//	//m_DBmgr->InsertData(str);
//
//	return true;
//}
//bool InspSurface_Reticle::WriteDB(int DCode, RECT rt, float size)
//{
//	int PosX;//
//	int PosY;//
//	int Darea;//
//	int UnitX;
//	int UnitY;
//	int Width;//
//	int Height;//
//	int ClusterID;
//	int Dcode;
//
//	Width = rt.right - rt.left;
//	Height = rt.bottom - rt.top;
//	PosX = rt.left + (int)(Width * 0.5);
//	PosY = rt.top + (int)(Height * 0.5);
//	Darea = (int)size;
//	UnitX = 0;
//	UnitY = 0;
//	ClusterID = 0;
//	Dcode = DCode;
//
//
//	char ch[500];
//	//NO POSX POSY DAREA UNITX UNITY WIDTH HEIGHT CLUSTERID RECTL RECTT RECTR RECTB DNAME DCODE
//	//기존 VISION에는 adddefect에서 posx, posy, rect 환산식이 있음
//
//	// 보여주기 용 일부로 rt rect 값 뺌
//	sprintf_s(ch, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
//		dbcount, PosX, PosY, Darea, UnitX, UnitY, Width, Height, ClusterID, rt.left + 10, rt.top + 10, rt.right + 10, rt.bottom + 10, 1234, Dcode, 0);
//
////	sprintf_s(ch, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
////		dbcount, PosX, PosY, Darea, UnitX, UnitY, Width, Height, ClusterID, rt.left, rt.top, rt.right, rt.bottom, 1234, Dcode,0);
//
////	sprintf(ch, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
////		dbcount, PosX, PosY, Darea, UnitX, UnitY, Width, Height, ClusterID, rt.left, rt.top, rt.right, rt.bottom, 1234, Dcode);
//	string str(ch);
//	m_DBmgr->InsertData(str);
//	dbcount++;
//
//	return true;
//}
