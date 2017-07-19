/***************************************************************************************
Note: 
1. Yellow LP contrast is too small, sometime sobel can't get good edge enhance.
2. Hypothesis: real lp width range [400,80], height range [100, 11].

****************************************************************************************/
#include "LpDetectRec.h"

#define HorProWnd_H 60
#define HorProWnd_W 120
#define MIN_LP_HEIGHT 5

#define ResizeRatio 2


CLpDetectRec::CLpDetectRec()
{
	m_pIntgralImg = NULL;
	m_iW = 0;
	m_iH = 0;
}

CLpDetectRec::~CLpDetectRec()
{

}

bool CLpDetectRec::Detect(cv::Mat gray, std::vector<cv::Rect>& vecLpPos)
{
	cv::Mat textureImg = calcTexture(gray);

	createIntgralImg(textureImg);

	std::vector<cv::Rect> vecRt = coarseHorLoc1(textureImg);
	
	vecRt = coarseLoc1Merge(vecRt);

	// Debug show coarseHorLoc1 result
	//for (int h = 0; h < vecRt.size(); h++)
	//{
	//	cv::rectangle(textureImg, vecRt[h], cv::Scalar(255, 255, 255), 1);
	//}

	vecLpPos = coarseVerticalLoc2(textureImg, vecRt);
	// Debug show coarseVerticalLoc2 result
	//for (int h = 0; h < vecRt.size(); h++)
	//{
	//	cv::rectangle(textureImg, vecRt[h], cv::Scalar(255, 255, 255), 1);
	//}

	//cv::imshow("x", textureImg);
	//cv::waitKey(0);

	// resize to src image.
	for (size_t i = 0; i < vecLpPos.size(); i++)
	{
		vecLpPos[i].x *= ResizeRatio;
		vecLpPos[i].y *= ResizeRatio;
		vecLpPos[i].width *= ResizeRatio;
		vecLpPos[i].height *= ResizeRatio;
	}

	return true;
}

cv::Mat CLpDetectRec::calcTexture(cv::Mat gray)
{
	cv::Mat textureImg = cv::Mat::zeros(gray.rows / ResizeRatio, gray.cols / ResizeRatio, CV_8UC1);
	//cv::resize(gray, textureImg, cv::Size(gray.cols / 4, gray.rows / 4));
	
	//cv::Mat sobelimg;
	//float kernelfilter[] = { 1, 0, -1, 2, 0, -2, 1, 0, -1 };
	//cv::Mat kernel(3, 3, CV_32FC1, kernelfilter);
	////cv::filter2D(gray, sobelimg, -1, kernel);
	//cv::Sobel(gray, sobelimg, -1, 1, 0);
	//gray = sobelimg.clone();

	//cv::resize(gray, textureImg, cv::Size(gray.cols / 4, gray.rows / 4));
	//cv::imshow("f", sobelimg);
	//cv::waitKey(1);
	//return textureImg;

	uchar* psrc = gray.data;
	uchar* pdst = textureImg.data;
	int step4W = gray.cols * ResizeRatio;

	for (int h = 0; h < gray.rows - 1; h += ResizeRatio)
	{
		int dw = 0;
		for (int w = 0; w < gray.cols - 4; w += ResizeRatio, dw++)
		{
			pdst[dw] = std::min(255, 2*std::max(abs(psrc[w + 4] - psrc[w]), abs(psrc[w + 4] - psrc[w + 2])));
		}

		psrc += step4W;
		pdst += textureImg.cols;
	}

	return textureImg;
}

std::vector<cv::Rect> CLpDetectRec::coarseVerticalLoc2(cv::Mat textureImg, std::vector<cv::Rect> vecHorRt)
{
	std::vector<cv::Rect> vecRt;

	int *pProjVArr = new int[textureImg.cols];
	int *pCurIntgralImg = m_pIntgralImg;

	for (int v = 0; v < vecHorRt.size(); v++)
	{
		cv::Rect& curRt = vecHorRt[v];
		int addWidth = curRt.height * 2;
		int x1 = std::max(0, curRt.x - addWidth);
		int x2 = std::min(textureImg.cols - 1, curRt.x + curRt.width - 1 + addWidth);
		int y1 = curRt.y;
		int y2 = curRt.y + curRt.height - 1;

		int lpPos = 0;
		int lpMaxRange = 0;
		int lpMinRange = 0;
		wndPrjVerBox(x1, x2, y1, y2, pProjVArr, lpPos, lpMaxRange, lpMinRange);

		int realx1 = 0, realx2 = 0;
		if (checkIsLp(x1, x2, y1, y2, pProjVArr, lpPos, lpMaxRange, lpMinRange, realx1, realx2))
		{
			if(realx2 - realx1 + 1 > lpMaxRange * 2)
			{
				continue;
			}
			if(realx2 - realx1 + 1 > 200)
			{
				continue;
			}
			// Not exceeding image.
			if (x1 + realx2 >= textureImg.cols || realx1 < 0)
			{
				continue;
			}
			vecRt.push_back(cv::Rect(x1 + realx1, y1, realx2 - realx1 + 1, y2 - y1 + 1));
#if 0
			cv::Mat showt = textureImg.clone();
			cv::rectangle(showt, curRt, cv::Scalar(110, 110, 110), 1);
			for (int wp = x1; wp < x2; wp++)
			{
				cv::line(showt,
					cv::Point(wp, pProjVArr[wp - x1]),
					cv::Point(wp + 1, pProjVArr[wp - x1 + 1]),
					cv::Scalar(255, 255, 255), 1);
			}
			cv::line(showt, cv::Point(x1, y1), cv::Point(x2, y1),
				cv::Scalar(255, 255, 255), 1);

			cv::rectangle(showt, cv::Rect(x1 + realx1, y1, realx2 - realx1 + 1, y2 - y1 + 1), cv::Scalar(255, 255, 255), 2);
			
			cv::imshow("showt_v_checkIsLp", showt);
			cv::waitKey(0);
#endif
		}

		// Show Vertical project result
#if 0
		cv::Mat showt = textureImg.clone();
		cv::rectangle(showt, curRt, cv::Scalar(110, 110, 110), 1);
		for (int wp = x1; wp < x2; wp++)
		{
			cv::line(showt,
				cv::Point(wp, pProjVArr[wp - x1]),
				cv::Point(wp + 1, pProjVArr[wp - x1 + 1]),
				cv::Scalar(255, 255, 255), 1);
		}
		cv::line(showt, cv::Point(x1, y1), cv::Point(x2, y1),
			cv::Scalar(255, 255, 255), 1);

		cv::rectangle(showt, cv::Rect(x1 + lpPos - lpMaxRange, y1, lpMaxRange * 2, y2 - y1 + 1), cv::Scalar(110, 110, 110), 2);
		cv::rectangle(showt, cv::Rect(x1 + lpPos - lpMinRange, y1, lpMinRange * 2, y2 - y1 + 1), cv::Scalar(110, 110, 110), 2);

		cv::imshow("showt_v", showt);
		cv::waitKey(0);
#endif

	}
	
	// Show all lp
#if 0
	cv::Mat showt = textureImg.clone();
	for (int v = 0; v < vecRt.size(); v++)
	{
		cv::rectangle(showt, vecRt[v], cv::Scalar(255, 255, 255), 2);
	}
	cv::imshow("showt_all_lp", showt);
	cv::waitKey(0);
#endif

	delete [] pProjVArr;
	return vecRt;
}

void CLpDetectRec::createIntgralImg(cv::Mat texture)
{
	if (m_pIntgralImg != NULL)
	{
		delete[] m_pIntgralImg;
	}
	m_iW = texture.cols + 1;
	m_iH = texture.rows + 1;

	m_pIntgralImg = calcIntgralImg(texture);
}

int * CLpDetectRec::calcIntgralImg(cv::Mat texture)
{
	int * pIntgralImg = new int[m_iW * m_iH];
	if (NULL == pIntgralImg)
	{
		printf("can't new \n");
	}
	memset(pIntgralImg, 0, sizeof(int)* m_iW * m_iH);
	
	uchar* pSrc = texture.data;
	int * pBuf1 = pIntgralImg + m_iW;
	for (size_t i = 1; i < m_iH; i++)
	{
		int * pBuf2 = pBuf1;
		for (size_t j = 1; j < m_iW; j++)
		{
			pBuf2[j] = pBuf2[j - 1] + pBuf2[j - m_iW] - pBuf2[j - m_iW - 1] + pSrc[0];
			pSrc++;
		}
		pBuf1 += m_iW;
	}
	return pIntgralImg;
}

// Coarse Horizontal Loc.
std::vector<cv::Rect> CLpDetectRec::coarseHorLoc1(cv::Mat texture)
{
	std::vector<cv::Rect> vecRt;
	vecRt.push_back(cv::Rect(0, 0, 0, 0));

	// MAX LP [120*30] on the texture image.
	int *pProjHArr = new int[HorProWnd_H];

	int startH = texture.rows > 100 ? 20 : 0;
	m_realProjH = std::min(HorProWnd_H, (int)texture.rows - startH);
	m_realProjW = std::max(std::min(HorProWnd_W, (int)texture.cols - HorProWnd_W), 60 / ResizeRatio);

	int *pCurIntgralImg = m_pIntgralImg;
	for (int h = startH; h <= (int)texture.rows - m_realProjH; h += 1)
	{
		pCurIntgralImg = m_pIntgralImg + h * m_iW;
		uchar* pbuf = texture.data + h*texture.cols;
		for (int w = 0; w < (int)texture.cols - m_realProjW; w += 4)
		{
			wndPrjHor120_30(texture, pCurIntgralImg + w, m_iW, m_iH, pProjHArr);

			// find lp loc based on Horizontal Project Data
			int y1 = 0, y2 = 0;
			if (judgeIsLpLocByHorProData_120_30(pProjHArr, y1, y2))
			{
				if (y1 + h == vecRt[vecRt.size() - 1].y && y2 + h == vecRt[vecRt.size() - 1].y + vecRt[vecRt.size() - 1].height - 1)
				{
					vecRt[vecRt.size() - 1].width = (y2 - y1 + 1) * 4 + w - vecRt[vecRt.size() - 1].x;
					continue;
				}
				// add suspected LP
				cv::Rect curRT = cv::Rect(w, y1 + h,0,y2 - y1 + 1);
				curRT.width = curRT.height * 4;
				vecRt.push_back(curRT);

				//// Debug show horizontal data
				//cv::Mat showt = texture.clone();
				//cv::rectangle(showt, curRT, cv::Scalar(110, 110, 110), 1);
				//for (int hp = 0; hp < m_realProjH - 1; hp++)
				//{
				//	cv::line(showt,
				//		cv::Point(w + pProjHArr[hp], hp + h),
				//		cv::Point(w + pProjHArr[hp + 1], hp + 1 + h),
				//		cv::Scalar(255, 255, 255), 1);
				//}
				//cv::line(showt, cv::Point(w, h), cv::Point(w, h + m_realProjH - 1),
				//	cv::Scalar(255, 255, 255), 1);
				//cv::imshow("showt", showt);
				//cv::waitKey(0);
			
				// If sure, skip current postion.
				//w += curRT.width/2 - 4;
				continue;
			}
		}
	}

	delete pProjHArr;
	return vecRt;
}

// Horizontal merge boundingbox.
std::vector<cv::Rect> CLpDetectRec::coarseLoc1Merge(std::vector<cv::Rect> vecRt)
{
	std::vector<cv::Rect> vecNewRt;

	for (int i = 0; i < (int)vecRt.size() - 1; i++)
	{
		for (int j = i + 1; j < vecRt.size(); j++)
		{
			// merge ?
			int maxLeft = std::max(vecRt[i].x, vecRt[j].x);
			int minRight = std::min(vecRt[i].x + vecRt[i].width, vecRt[j].x + vecRt[j].width);

			if (abs(vecRt[i].y - vecRt[j].y) < 3 && abs(vecRt[i].y + vecRt[i].height- vecRt[j].y - vecRt[j].height) < 3
				&& maxLeft < minRight)
			{
				vecRt[i].x = std::min(vecRt[i].x, vecRt[j].x);
				vecRt[i].y = std::min(vecRt[i].y, vecRt[j].y);
				vecRt[i].width = std::max(vecRt[i].x + vecRt[i].width, vecRt[j].x + vecRt[j].width) - vecRt[i].x;
				vecRt[i].height = std::max(vecRt[i].y + vecRt[i].height, vecRt[j].y + vecRt[j].height) - vecRt[i].y;
				// delete j element
				vecRt[j].y = 0;
			}
		}
	}

	for (int i = 0; i < (int)vecRt.size() - 1; i++)
	{
		if (vecRt[i].y == 0)
		{
			continue;
		}
		vecNewRt.push_back(vecRt[i]);
	}
	return vecNewRt;
}

// Based on judgeIsLpLocByHorProData_120_30, search y2;
bool CLpDetectRec::searchY2(int *pPorjHArr, int y1, int& y2)
{
	for (size_t i = y1; i < m_realProjH - 3; i++)
	{
		if (pPorjHArr[i + 2] < 45 /*&& pPorjHArr[i + 2] < 45*/)
		{
			y2 = i + 1;
			return true;
		}
	}
	return false;
}

// Find lp loc based on Horizontal Project Data, MAX value 100
bool CLpDetectRec::judgeIsLpLocByHorProData_120_30(int *pPorjHArr, int& y1, int& y2)
{
	// Search peak of LP waveform. value range[0,100];
	// Min LP height = 3;

	for (size_t i = 0; i < m_realProjH - MIN_LP_HEIGHT - 2; i++)
	{
		if (pPorjHArr[i] < 45)
		{
			// At least 3 pixel > 60;
			if (pPorjHArr[i + 2] > 60 && pPorjHArr[i + 3] > 60 && pPorjHArr[i + 4] > 60)
			{
				y1 = i;
				// Search y2
				if (searchY2(pPorjHArr, y1 + 3, y2))
				{
					// find y2
					return true;
				}
			}
		}
	}

	return false;
}

// Check if it is real LP
bool CLpDetectRec::checkIsLp(int x1, int x2, int y1, int y2, int *pProjVArr, int &lpPos, int& lpMaxRange, int& lpMinRange, int& realx1, int &realx2)
{
	// Check min range
	int minx1 = std::max(0, lpPos - lpMinRange);
	int minx2 = std::min(x2 - x1 + 1, lpPos + lpMinRange);
	//for (size_t i = minx1; i < minx2 - 1; i++)
	//{
	//	// neighbor two, both < 10, is not lp.
	//	if (pProjVArr[i] < 10 && pProjVArr[i + 1] < 10)
	//	{
	//		return false;
	//	}
	//}

	// find left position.
	realx1 = 0;
	int maxLeftx = std::max(0, lpPos - lpMaxRange);
	for (size_t i = lpPos; i > maxLeftx; i--)
	{
		// neighbor three pixels < 10 is LP left edge.
		if (pProjVArr[i] < 10 && pProjVArr[i + 1] < 10 && pProjVArr[i + 2] < 10)
		{
			realx1 = i + 2;
			break;
		}
	}

	// find right position.
	realx2 = 0;
	int maxRightX = std::max(x2 - x1 + 1, lpPos + lpMaxRange);
	for (size_t i = lpPos; i < maxRightX - 2; i++)
	{
		// neighbor three pixels < 10 is LP left edge.
		if (pProjVArr[i] < 10 && pProjVArr[i + 1] < 10 && pProjVArr[i + 2] < 10)
		{
			realx2 = i;
			break;
		}
	}

	int maxW = (y2 - y1 + 1) * 7;
	int minW = (y2 - y1 + 1) * 1.8 + 0.5f;
	if (realx2 - realx1 > maxW || realx2 - realx1 < minW)
	{
		return false;
	}

	return true;
}

// Vertical Project, search lp pos.
void CLpDetectRec::wndPrjVerBox(int x1, int x2, int y1, int y2, int *pProjVArr, int &lpPos, int& lpMaxRange, int& lpMinRange)
{
	int maxProjVer = 0;
	int *pCurI = m_pIntgralImg + y1 * m_iW + x1;
	int lpHeight = y2 - y1 + 1;
	int stepH = m_iW * lpHeight;
	int xRange = x2 - x1;
	for (int w = 0; w < xRange; w++)
	{
		pProjVArr[w] = pCurI[stepH + 1] -
			pCurI[stepH] -
			pCurI[1] +
			pCurI[0];

		maxProjVer = std::max(maxProjVer, pProjVArr[w]);
		pCurI ++;
	}

	if (maxProjVer <= 0) return;

	// normalize, range [0, 100], and calc intragral
	int *pProArrI = new int[xRange + 1];
	pProArrI[0] = 0;
	for (int w = 0; w < xRange; w++)
	{
		pProjVArr[w] = pProjVArr[w] * 100 / maxProjVer;
		pProArrI[w + 1] = pProArrI[w] + pProjVArr[w];
	}

	// filter, 3 mean
	for (int w = 0; w < xRange-3; w++)
	{
		int cursum = pProArrI[w + 3] - pProArrI[w];
		pProjVArr[w] = cursum / 3;
	}
	for (int w = 0; w < xRange - 3; w++)
	{
		int cursum = pProArrI[w + 3] - pProArrI[w];
		pProjVArr[w] = cursum / 3;
	}

	// Search LP position.
	// Guess LP widht = 4*lpheiht
	int lpWidth = lpHeight * 4;
	int maxV = 0;
	for (int w = 0; w < xRange - lpWidth; w++)
	{
		int cursum = pProArrI[w + lpWidth] - pProArrI[w];
		if (cursum > maxV)
		{
			lpPos = w;
			maxV = cursum;
		}
	}
	lpPos += lpHeight * 2;// half lpWidth
	// LP width / height range [7, 1.8];
	lpMaxRange = 7 * lpHeight / 2;
	lpMinRange = (int)(1.8f * lpHeight / 2 + 0.5f);

	delete[] pProArrI;
}

// Horizontal Project
void CLpDetectRec::wndPrjHor120_30(cv::Mat texture, int* pIntgralImg, int iW, int iH, int *pPorjHArr)
{
	int maxProjHor = 0;
	int *pCurI = pIntgralImg;
	for (int h = 0; h < m_realProjH; h++)
	{
		pPorjHArr[h] = pCurI[iW + m_realProjW] -
			pCurI[iW] -
			pCurI[m_realProjW] +
			pCurI[0];

		maxProjHor = std::max(maxProjHor, pPorjHArr[h]);
		pCurI += iW;
	}

	//for (int h = 1; h < m_realProjH-1; h++)
	//{
	//	if (pPorjHArr[h - 1] - pPorjHArr[h] > 10 &&
	//		pPorjHArr[h + 1] - pPorjHArr[h] > 10)
	//	{
	//		pPorjHArr[h] = (pPorjHArr[h-1] + pPorjHArr[h + 1]) / 2;
	//	}
	//}

	// normalize
	if (maxProjHor <= 0) return;
	for (int h = 0; h < m_realProjH; h++)
	{
		pPorjHArr[h] = pPorjHArr[h] * 100 / maxProjHor;
	}
}

cv::Mat CLpDetectRec::calcIntegralImg(cv::Mat texture)
{
	return cv::Mat();
}