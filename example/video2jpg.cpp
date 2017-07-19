/****************************************************
vido position: \\iot-server\DL Share\Check_Point_Traffic_Video
Xiping Yan 2017-07-10
****************************************************/

#include <CommonTest.h>
#include "LpDetectRec.h"
#include <opencv2\opencv.hpp>

using namespace cv;
using std::cout;
using std::endl;


void video2Jpg()
{
	std::string strVideoFn = "C:/XipingYan_Code/OpenSourceCode/MyGithub/LP-Detection-Recognition/windows/";
	strVideoFn += "192.168.1.46_ch1_20150525_092731_20150525092945_20150525100558.avi";

	VideoCapture capture(strVideoFn);

	if (!capture.isOpened())
	{
		std::cout << "fail to open!" << std::endl;
	}

	//获取整个帧数
	long totalFrameNumber = capture.get(CAP_PROP_FRAME_COUNT);
	std::cout << "Total frame number = " << totalFrameNumber << " frames" << std::endl;

	//获取帧率
	double rate = capture.get(CAP_PROP_FPS);
	cout << "frame rate = " << rate << endl;

	bool stop = false;
	int idx = 0;
	Mat frame;
	namedWindow("Extracted frame");

	std::string strSVPath = "result_jpg";
	MKDirTest(strSVPath.c_str());
	static int g_idx = 0;

	while (!stop)
	{
		//读取下一帧
		if (!capture.read(frame))
		{
			cout << "read video fail" << endl;
			return;
		}
		char aIdx[32] = { 0 };
		sprintf(aIdx, "%04d.jpg", g_idx++);
		cv::imwrite(strSVPath + "/" + aIdx, frame);

		if (idx > 2000)
		{
			break;
		}
		idx++;
	}

	//关闭视频文件
	capture.release();
}