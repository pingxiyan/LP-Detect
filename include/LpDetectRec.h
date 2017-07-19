#ifndef _LPDETECTREC_H_
#define _LPDETECTREC_H_

#include <opencv2\opencv.hpp>
#include <algorithm>

class CLpDetectRec
{
public:
	CLpDetectRec();
	~CLpDetectRec();
	bool Detect(cv::Mat gray, std::vector<cv::Rect>& vecLpPos);
	
private:
	std::vector<cv::Rect> coarseVerticalLoc2(cv::Mat textureImg, std::vector<cv::Rect> vecHorRt);

	// Get texture image, resize 4*4 times;
	cv::Mat calcTexture(cv::Mat gray);

	int * CLpDetectRec::calcIntgralImg(cv::Mat texture);

	// Coarse Horizontal Loc.
	std::vector<cv::Rect> coarseHorLoc1(cv::Mat texture);

	std::vector<cv::Rect> coarseLoc1Merge(std::vector<cv::Rect> vecRt);
	
	// Check if it is real LP
	bool checkIsLp(int x1, int x2, int y1, int y2, int *pProjVArr, int &lpPos, int& lpMaxRange, int& lpMinRange, int& realx1, int &realx2);

	// Vertical Project
	void wndPrjVerBox(int x1, int x2, int y1, int y2, int *pProjVArr, int &lpPos, int& lpMaxRange, int& lpMinRange);
	
	// Horizontal Project
	void wndPrjHor120_30(cv::Mat texture, int* pIntgralImg, int iW, int iH, int *pPorjHArr);

	// Find lp loc based on Horizontal Project Data, MAX value 100
	bool judgeIsLpLocByHorProData_120_30(int *pPorjHArr, int& y1, int& y2);

	// Based on judgeIsLpLocByHorProData_120_30, search y2;
	bool searchY2(int *pPorjHArr, int y1, int& y2);

	void createIntgralImg(cv::Mat texture);
	cv::Mat calcIntegralImg(cv::Mat texture);

private:
	int *m_pIntgralImg;
	int m_iW;
	int m_iH;
	int m_realProjH;
	int m_realProjW;
};

// test parse ans.
void parseLpAns();

class CRealLP
{
public:
	CRealLP(){
		lpPos = cv::Rect(0, 0, 0, 0);
	};
	~CRealLP(){};
	cv::Rect lpPos;
	std::string strLPTxt;
};

// Parse one real LP infor.
std::vector<CRealLP> parseOneImgTxt(std::string ansfn);

// video transfer to jpg images.
void video2Jpg();

#endif /* _LPDETECTREC_H_ */