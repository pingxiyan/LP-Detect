#include "LpDetectRec.h"
#include "CommonTest.h"
#include <string>
using std::string;

std::string parseOneLpTxt(string ptxt)
{
	string tmp = ptxt;
	string curchar = tmp.substr(ptxt.find("：") + 2, ptxt.length());
	//std::cout << curchar << std::endl;

	return curchar;
}

cv::Rect parseOneLpPos(string ptxt)
{
	int x = 0, y = 0, x2 = 0, y2 = 0;
	string tmp = ptxt;
	int p1 = tmp.find("上") + 2; 
	int p2 = tmp.find("下");
	string curchar = tmp.substr(p1, p2 - p1 - 1);
	y = atoi(curchar.c_str());

	p1 = tmp.find("下") + 2;
	p2 = tmp.find("左");
	curchar = tmp.substr(p1, p2 - p1 - 1);
	y2 = atoi(curchar.c_str());

	p1 = tmp.find("左") + 2;
	p2 = tmp.find("右");
	curchar = tmp.substr(p1, p2 - p1 - 1);
	x = atoi(curchar.c_str());

	p1 = tmp.find("右") + 2;
	curchar = tmp.substr(p1, tmp.length() - p1);
	x2 = atoi(curchar.c_str());

	cv::Rect rt(x, y, x2 - x + 1, y2 - y + 1);
	return rt;
}

CRealLP parseFirstLp(FILE* pf)
{
	CRealLP rLP;
	char atxt[1024] = { 0 };

	fgets(atxt, 1024, pf);
	rLP.strLPTxt = parseOneLpTxt(atxt);

	for (int i = 1; i < 6; i++)
	{
		fgets(atxt, 1024, pf);
	}
	rLP.lpPos = parseOneLpPos(atxt);

	return rLP;
}

CRealLP parseSecondLp(FILE* pf)
{
	char atxt[1024] = { 0 };
	for (int i = 0; i < 4; i++)
	{
		fgets(atxt, 1024, pf);
	}

	return parseFirstLp(pf);
}

std::vector<CRealLP> parseOneImgTxt(std::string ansfn)
{
	std::vector<CRealLP> vecRes;
	char atxt[1024] = { 0 };

	FILE* pf = fopen(ansfn.c_str(), "rb");
	
	for (int i = 0; i < 11; i++)
	{
		fgets(atxt, 1024, pf);
	}

	int lpnum = atoi(atxt + 10);

	if (lpnum >= 1)
	{
		vecRes.push_back(parseFirstLp(pf));
	}
	if (lpnum >= 2)
	{
		vecRes.push_back(parseSecondLp(pf));
	}
	if (lpnum >= 3)
	{
		vecRes.push_back(parseSecondLp(pf));
	}
	if (lpnum >= 4)
	{
		vecRes.push_back(parseSecondLp(pf));
	}

	fclose(pf);

	return vecRes;
}

void parseLpAns()
{
	std::string dirfn = "C:/XipingYan_Code/ImgLib/Intel_lp_db/dir.set";
	std::vector<std::string> vecFn = readsample(dirfn);

	int maxW = 0, minW = 1000;
	int maxH = 0, minH = 1000;
	float maxWHRatio = 0, minWHRatio = 500;

	for (int i = 0; i < vecFn.size(); i++)
	{
		std::string srcfn = vecFn[i];
		std::string ansfn = srcfn.substr(0, srcfn.rfind(".")) + ".txt";
		
		std::vector<CRealLP> vecRt = parseOneImgTxt(ansfn);

		cv::Mat src = cv::imread(srcfn);
		// find max min value
		for (size_t j = 0; j < vecRt.size(); j++)
		{
			maxW = std::max(maxW, vecRt[j].lpPos.width);
			minW = std::min(minW, vecRt[j].lpPos.width);
			maxH = std::max(maxH, vecRt[j].lpPos.height);
			minH = std::min(minH, vecRt[j].lpPos.height);
			
			cv::rectangle(src, vecRt[j].lpPos, cv::Scalar(0, 0, 255));

			float curwhr = (float)vecRt[j].lpPos.width / vecRt[j].lpPos.height;
			maxWHRatio = std::max(curwhr, maxWHRatio);
			minWHRatio = std::min(curwhr, minWHRatio);
		}
		//if (minWHRatio < 2)
		//{
		//	cv::imshow("xx", src);
		//	cv::waitKey(0);
		//}
		//if (i >= 508)
		//{
		//	printf("xxx");
		//}
		//printf("%d, max w = %d, min w = %d, max h = %d, min h = %d\n", i, maxW, minW, maxH, minH);
		//printf("max whr = %f, min whr = %f\n", (float)maxWHRatio, (float)minWHRatio);
	}
	printf("max w = %d, min w = %d, max h = %d, min h = %d\n", maxW, minW, maxH, minH);
	printf("max whr = %f, min whr = %f\n", (float)maxWHRatio, (float)minWHRatio);
}