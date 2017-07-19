/* test lp detect 
	xiping 2017-6-26	*/

#include "CommonTest.h"
#include <iostream>
#include "LpDetectRec.h"
#include <vector>

std::string svSamplePosPath = ".\\sample\\pos";
std::string svSampleNegPath = ".\\sample\\neg";
static int g_neg_num = 0;

void savePosSample(std::string fn, CRealLP realLP, cv::Mat src)
{
	std::string sfn = fn.substr(fn.rfind("\\")+1, fn.length());
	std::string lpTxt = realLP.strLPTxt.substr(0, realLP.strLPTxt.length() - 2);
	std::string svfn = svSamplePosPath + "\\" + sfn + "_" + lpTxt + "_0.jpg";
	
	//std::cout << svfn << std::endl;

	// src lp
	cv::Rect lpSrcPos = realLP.lpPos;
	if (lpSrcPos.width < 1 || lpSrcPos.height)
	{
		return;
	}

	cv::Mat roiLP = cv::Mat(src, lpSrcPos);
	cv::imwrite(svfn, roiLP);

	int sx1 = lpSrcPos.x;
	int sx2 = lpSrcPos.x + lpSrcPos.width - 1;
	int sy1 = lpSrcPos.y;
	int sy2 = lpSrcPos.y + lpSrcPos.height;

	int offx = lpSrcPos.height / 2;
	int offy = lpSrcPos.height / 5;

	// 4 edges expansion
	svfn = svSamplePosPath + "\\" + sfn + "_" + lpTxt + "_1.jpg";
	int x1 = std::max(0, sx1 - offx);
	int y1 = std::max(0, sy1 - offy);
	int x2 = std::min(src.cols - 1, sx2 + offx);
	int y2 = std::min(src.rows - 1, sy2 + offy);
	cv::Rect lpP1 = cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	roiLP = cv::Mat(src, lpP1);
	cv::imwrite(svfn, roiLP);

	// expand left + top + bottom
	svfn = svSamplePosPath + "\\" + sfn + "_" + lpTxt + "_2.jpg";
	x1 = std::max(0, sx1 - offx);
	y1 = std::max(0, sy1 - offy);
	x2 = sx2;
	y2 = std::min(src.rows - 1, sy2 + offy);
	lpP1 = cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	roiLP = cv::Mat(src, lpP1);
	cv::imwrite(svfn, roiLP);

	// expand right + top + bottom
	svfn = svSamplePosPath + "\\" + sfn + "_" + lpTxt + "_3.jpg";
	x1 = sx1;
	y1 = std::max(0, sy1 - offy);
	x2 = std::min(src.cols - 1, sx2 + offx);
	y2 = std::min(src.rows - 1, sy2 + offy);
	lpP1 = cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	roiLP = cv::Mat(src, lpP1);
	cv::imwrite(svfn, roiLP);
}

void saveNegSample(std::string fn, cv::Rect lpPos, cv::Mat gray, std::vector<CRealLP> vecRealLp)
{
	for (int i = 0; i < vecRealLp.size(); i++)
	{
		int xm = std::max(lpPos.x, vecRealLp[i].lpPos.x);
		int ym = std::max(lpPos.y, vecRealLp[i].lpPos.y);
		int x2m = std::min(lpPos.x + lpPos.width, vecRealLp[i].lpPos.x + vecRealLp[i].lpPos.width);
		int y2m = std::min(lpPos.y + lpPos.height, vecRealLp[i].lpPos.y + vecRealLp[i].lpPos.height);

		if (x2m > xm && y2m > ym)// have merage
		{
			int mArea = (y2m - ym)*(x2m - xm);
			if (mArea > 0.4 * vecRealLp[i].lpPos.width * vecRealLp[i].lpPos.height || 
				mArea > 0.6 * lpPos.width * lpPos.height)
			{
				// real lp, no save.
				return;
			}
		}
	}

	std::string sfn = fn.substr(fn.rfind("\\") + 1, fn.length());

	char g_neg_num_txt[128] = { 0 };
	sprintf(g_neg_num_txt, "%d", g_neg_num);
	std::string svfn = svSampleNegPath + "\\" + sfn + "_" + g_neg_num_txt + ".jpg";
	g_neg_num++;

	cv::Mat roiLP = cv::Mat(gray, lpPos);
	cv::imwrite(svfn, roiLP);
}

void testOneImg(std::string fn)
{
	//fn = "C:\\XipingYan_Code\\ImgLib\\Intel_lp_db\\SingleLayerBlueLp\\07_1360x1024D\\A01_07_014.jpg";
	
	CLpDetectRec lp;

	cv::Mat src = cv::imread(fn.c_str(), 1);
	cv::Mat gray = cv::imread(fn.c_str(), 0);
	std::vector<cv::Rect> vecLpPos;

	float rszCoeff = 1.0f;
	//resize(src, src, cv::Size(src.cols / rszCoeff, src.rows / rszCoeff));
	//resize(gray, gray, cv::Size(gray.cols / rszCoeff, gray.rows / rszCoeff));


	// show reslut
	//cv::namedWindow("result", 1);
	//cv::imshow("result", src);
	//cv::waitKey(0);

	// lp.Detect(gray, vecLpPos);

	// parse real lp
	std::string srcfn = fn;
	std::string ansfn = srcfn.substr(0, srcfn.rfind(".")) + ".txt";
	std::vector<CRealLP> vecRealLp = parseOneImgTxt(ansfn);
	for (int i = 0; i < vecRealLp.size(); i++)
	{
		cv::Rect& rlp = vecRealLp[i].lpPos;
		vecRealLp[i].lpPos.x = rlp.x / rszCoeff;
		vecRealLp[i].lpPos.y = rlp.y / rszCoeff;
		vecRealLp[i].lpPos.width = rlp.width / rszCoeff;
		vecRealLp[i].lpPos.height = rlp.height / rszCoeff;
	}

	// detect result.
	for (int i = 0; i < vecLpPos.size(); i++)
	{
		//cv::rectangle(src, vecLpPos[i], cv::Scalar(0, 255, 0), 2);

		//saveNegSample(fn, vecLpPos[i], gray, vecRealLp);
	}
	// real result
	for (int i = 0; i < vecRealLp.size(); i++)
	{
		//cv::rectangle(src, vecRealLp[i].lpPos, cv::Scalar(0, 0, 255), 1);

		savePosSample(fn, vecRealLp[i], src);
	}

	// show reslut
	//cv::namedWindow("result", 1);
	//cv::imshow("result", src);
	//cv::waitKey(0);
}

int main(int argc, char** argv)
{	
	video2Jpg(); return 0;

	//parseLpAns();
	//return 0;

	// Decoding src video by OPENCVS
	// Save real LP position.
	MKDirTest(svSamplePosPath.c_str());
	MKDirTest(svSampleNegPath.c_str());

	std::string dirfn = "C:/XipingYan_Code/ImgLib/Intel_lp_db/dir.set";
	std::vector<std::string> vecFn = readsample(dirfn);

	for (int i = 0; i < vecFn.size(); i++)
	{
		printf("\r%d    ", i);
		testOneImg(vecFn[i]);
	}

	return 0;
}