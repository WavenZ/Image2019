#define _CRT_SECURE_NO_WARNINGS
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace cv;
using namespace std;

#define CV_EVENT_LBUTTONDOWN 1           // left click

/************************************************
	@brief: Monocular camera measurement

	@author: WavenZ

	@time: 2019/5/1
*************************************************/
struct CameraParam{
	Mat cameraMatrix;
	Mat distCoeffs;
	Mat Rt;
	Mat Rt1;
};

Mat imageOrigin;		// original image
Mat imageDisplay;		// display image
Mat imageGray;			// gray image
CameraParam param;		// camera param
Point3d corners[2];		// start&end corner
int whichCorner = 0;
double Distance = 0;	// distance

void calcDistance() {
	// 反向投影
	Mat M_;
	invert(param.cameraMatrix, M_, DECOMP_SVD);
	Mat x1 = Mat(corners[0]);
	Mat x2 = Mat(corners[1]);

	// 去畸变（存在导致很小的误差）
	Mat X1 = M_ * x1;
	double r2 = X1.at<double>(0) * X1.at<double>(0) + X1.at<double>(1) * X1.at<double>(1);
	double r4 = r2 * r2;
	double dist = 1 + r2 * param.distCoeffs.at<double>(0) + r4 * param.distCoeffs.at<double>(1);
	X1.at<double>(0) /= dist;
	X1.at<double>(1) /= dist;

	Mat X2 = M_ * x2;
	r2 = X2.at<double>(0) * X2.at<double>(0) + X2.at<double>(1) * X2.at<double>(1);
	r4 = r2 * r2;
	dist = 1 + r2 * param.distCoeffs.at<double>(0) + r4 * param.distCoeffs.at<double>(1);
	X2.at<double>(0) /= dist;
	X2.at<double>(1) /= dist;

	// 转换到世界坐标系（约束：Z = 0）
	Mat Rt_;
	invert(param.Rt1, Rt_, DECOMP_SVD);
	Mat X1w = Rt_ * X1;
	Mat X2w = Rt_ * X2;

	// 归一化
	X1w = X1w / X1w.at<double>(2);
	X2w = X2w / X2w.at<double>(2);

	// 计算距离
	Mat D = X2w - X1w;
	Distance = sqrt(D.at<double>(0) * D.at<double>(0) + D.at<double>(1) * D.at<double>(1));
	cout << "Distance: " << Distance << endl;
}

static void onMouse(int event, int x, int y, int flags, void* param)
{	// 鼠标点击事件
	Mat* im = reinterpret_cast<Mat*>(param);
	if (event == CV_EVENT_LBUTTONDOWN) {
		Rect r(x - 20 > 0 ? x - 20 : 0, y - 20 > 0 ? y - 20 : 0, 40, 40);
		vector<Point2f> pointbuf;
		// 点击感兴趣区域
		Mat mask;
		mask = Mat::zeros(imageGray.size(), CV_8UC1);
		mask(r).setTo(255);
		// 角点检测
		goodFeaturesToTrack(imageGray, pointbuf, 1, 0.01, 10, mask);
		// 亚像素级角点检测提升精度
		cornerSubPix(imageGray, pointbuf, Size(5, 5),
			Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
		// 画点
		line(imageDisplay, Point2f{ pointbuf[0].x - 10, pointbuf[0].y - 10 },
				Point2d{ pointbuf[0].x + 10, pointbuf[0].y + 10 }, Scalar(0, 0, 200), 2);
		line(imageDisplay, Point2f{ pointbuf[0].x - 10, pointbuf[0].y + 10 },
				Point2d{ pointbuf[0].x + 10, pointbuf[0].y - 10 }, Scalar(0, 0, 200), 2);
		// 保存角点
		corners[whichCorner].x = pointbuf[0].x;
		corners[whichCorner].y = pointbuf[0].y;
		corners[whichCorner].z = 1;
		// 已检测到两个点时计算距离
		if (whichCorner == 1) {
			line(imageDisplay, Point2d{ corners[0].x, corners[0].y },
				Point2d{ corners[1].x, corners[1].y }, Scalar(0, 0, 200), 2);
			cout << corners[0] << ", " << corners[1] << endl;
			calcDistance();
			char str[20];
			sprintf(str, "%.2lfmm", Distance);
			putText(imageDisplay, str, Point2d{ (corners[0].x + corners[1].x) / 2.0 , (corners[0].y + corners[1].y) / 2.0 - 10 },
				2, 1.5, Scalar(0, 0, 200), 2, LINE_AA);
		}
		whichCorner = !whichCorner;
		imshow("Image", imageDisplay);
	}

}

void allInit(){
	// param init
	param.cameraMatrix = (Mat_<double>(3, 3) <<
		1.5621284315553469e+03, 0., 9.9531745685229419e+02,
		0., 1.5629858065134160e+03, 5.4181661016529097e+02,
		0., 0., 1.);

	param.distCoeffs = (Mat_<double>(5, 1) << 3.3186929343409627e-01, -1.5328976453608059e+00,
		-1.0555395111807473e-04, 5.3863533330160257e-05, 0.);

	param.Rt = (Mat_<double>(3, 4) <<
		0.9328536065677697, 0.04360998139854794, -0.3576063733155814, -9.2208456379855079e+01,
		-0.02461864634625879, 0.9980424581793637, 0.05749064205043169, -4.9301180933888901e+01,
		0.3594135097149, -0.04482656794481737, 0.9321011253295997, 5.6603131731116139e+02);

	param.Rt1 = (Mat_<double>(3, 3) <<
		0.9328536065677697, 0.04360998139854794, -9.2208456379855079e+01,
		-0.02461864634625879, 0.9980424581793637, -4.9301180933888901e+01,
		0.3594135097149, -0.04482656794481737, 5.6603131731116139e+02);

	// image init
	imageOrigin = imread("chess4.jpg", 1);
	imageDisplay = imageOrigin;
	cvtColor(imageOrigin, imageGray, COLOR_BGR2GRAY);

	// window init
	namedWindow("Image", 2);
	resizeWindow("Image", 1000, 560);
	setMouseCallback("Image", onMouse, reinterpret_cast<void*> (&imageDisplay));
	imshow("Image", imageDisplay);
}


int main( int argc, char** argv )
{
	allInit();
	while (1) { 
		if (waitKey(100) == 27)
			break;
	}
	destroyAllWindows();
	return 0;
}
