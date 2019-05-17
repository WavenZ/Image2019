#define _CRT_SECURE_NO_WARNINGS
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace cv;
using namespace std;

/************************************************
	@brief: Binocular camera measurement

	@author: WavenZ

	@time: 2019/5/2
*************************************************/
// global param
Mat leftCameraMatrix, rightCameraMatrix, leftDistCoeffs, rightDistCoeffs; // intrinsics
Mat M1, M2; // extrinsics

bool operator<(const Point2f& A, const Point2f& B) {
	return (2 * A.x + A.y) < (2 * B.x + B.y);
}
void mySort(vector<Point2f>& Vec) { // insert sort
	int n = Vec.size();
	for (int j = 1; j < n; j++) {
		Point2f key = Vec[j];
		int i = j - 1;
		while (i > -1 && key < Vec[i]) {
			Vec[i + 1] = Vec[i];
			i = i - 1;
		}
		Vec[i + 1] = key;
	}
}

vector<Point2f> cornerDetect(Mat imageOrigin) { // detect ipad's corner
	vector<Point2f> pointbuf;
	Mat imageGray;
	cvtColor(imageOrigin, imageGray, COLOR_BGR2GRAY);
	goodFeaturesToTrack(imageGray, pointbuf, 4, 0.1, 10, Mat(), 8);
	cornerSubPix(imageGray, pointbuf, Size(5, 5),
		Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
	mySort(pointbuf);
	return pointbuf;
}

void drawCorner(Mat& image, vector<Point2f> Vec) { // draw corner on image
	char str[5];
	for (int i = 0; i < Vec.size(); i++) {
		circle(image, Vec[i], 25, Scalar(0, 0, 255), 3);
		sprintf(str, "%c", i + 'A');
		putText(image, str, Point2f{ Vec[i].x - 60, Vec[i].y - 40 },
			2, 2.5, Scalar(0, 0, 200), 2, LINE_AA);
	}
}
void init(void) {
	// param init
	leftCameraMatrix = (Mat_<double>(3, 3) <<
		3.1363244206459267e+03, 0., 1.9975643305371914e+03,
		0., 3.1379972004343631e+03, 1.0947197231569949e+03,
		0., 0., 1.);
	rightCameraMatrix = (Mat_<double>(3, 3) <<
		3.1388008028250479e+03, 0., 1.9765231053495324e+03,
		0., 3.1387511101925943e+03, 1.0973791717011209e+03,
		0., 0., 1.);

	leftDistCoeffs = (Mat_<double>(5, 1) <<
		3.3856740636293203e-01, -1.7983650243001470e+00,
		2.5883400697955020e-05, 3.9689362292248311e-03, 0.);

	rightDistCoeffs = (Mat_<double>(5, 1) <<
		3.5767716577613357e-01, -2.3450054536051677e+00,
		1.2391061254811923e-03, -1.3487975194031272e-03, 0.);

	M1 = (Mat_<double>(3, 4) <<
		3.1363244206459267e+03, 0., 1.9975643305371914e+03, 0,
		0., 3.1379972004343631e+03, 1.0947197231569949e+03, 0,
		0., 0., 1., 0);

	M2 = (Mat_<double>(3, 4) <<
		1935.0751794733, -70.0912556875, 3163.7454593343, -518461.5436734332,
		-438.4661566839, 3133.1864538068, 1023.1762572221, 97448.9888825567,
		-0.4438240497, -0.0044981749, 0.8961026611, 90.9353830428);
}


Point3f calcCoordinate(Point2f left, Point2f right) {
	// overdetermined equation AX = b --> X = pinv(A) * b
	static Mat A = Mat::zeros(4, 3, CV_64FC1);
	A.at<double>(0, 0) = M1.at<double>(0, 0);
	A.at<double>(1, 1) = M1.at<double>(1, 1);
	A.at<double>(0, 2) = M1.at<double>(0, 2) - left.x;
	A.at<double>(1, 2) = M1.at<double>(1, 2) - left.y;
	A.at<double>(2, 0) = M2.at<double>(0, 0) - M2.at<double>(2, 0) * right.x;
	A.at<double>(2, 1) = M2.at<double>(0, 1) - M2.at<double>(2, 1) * right.x;
	A.at<double>(2, 2) = M2.at<double>(0, 2) - M2.at<double>(2, 2) * right.x;
	A.at<double>(3, 0) = M2.at<double>(1, 0) - M2.at<double>(2, 0) * right.y;
	A.at<double>(3, 1) = M2.at<double>(1, 1) - M2.at<double>(2, 1) * right.y;
	A.at<double>(3, 2) = M2.at<double>(1, 2) - M2.at<double>(2, 2) * right.y;

	static Mat b = Mat::zeros(4, 1, CV_64FC1);
	b.at<double>(2, 0) = M2.at<double>(2, 3) * right.x - M2.at<double>(0, 3);
	b.at<double>(3, 0) = M2.at<double>(2, 3) * right.y - M2.at<double>(1, 3);

	Mat A_;
	invert(A, A_, DECOMP_SVD); // pinv(A)
	
	Mat X = A_ * b; // X = pinv(A) * b

	return Point3f(X);
}
float calcDistance(Point3f p1, Point3f p2) {
	Point3f temp = p1 - p2;
	float distance =  sqrt(temp.x * temp.x + temp.y * temp.y + temp.z * temp.z);
	return distance;
}

int main(int argc, char** argv)
{
	// initialization
	init();

	// image
	Mat leftImageOrigin = imread("left.jpg", 1);
	Mat rightImageOrigin = imread("right.jpg", 1);

	// undistort
	Mat leftImage;
	Mat rightImage;
	undistort(leftImageOrigin, leftImage, leftCameraMatrix, leftDistCoeffs);
	undistort(rightImageOrigin, rightImage, rightCameraMatrix, rightDistCoeffs);

	// cornerdetect
	vector<Point2f> leftPoint = cornerDetect(leftImage);
	vector<Point2f> rightPoint = cornerDetect(rightImage);
	drawCorner(leftImage, leftPoint);
	drawCorner(rightImage, rightPoint);

	// calc coordinate
	Point3f point[4];
	for (int i = 0; i < leftPoint.size(); i++) {
		point[i] =  calcCoordinate(leftPoint[i], rightPoint[i]);
	}

	// Length
	cout << calcDistance(point[0], point[1]) << "mm" << endl;
	// Width
	cout << calcDistance(point[0], point[2]) << "mm" << endl;
	// diagonal line
	cout << calcDistance(point[0], point[3]) << "mm" << endl;

	// show image
	namedWindow("leftImage", 2);
	namedWindow("rightImage", 2);
	resizeWindow("leftImage", 1000, 562);
	resizeWindow("rightImage", 1000, 562);
	imshow("leftImage", leftImage);
	imshow("rightImage", rightImage);
	while (1) {
		if(waitKey(10) == 27)
			break;
	}
	destroyAllWindows();
    return 0;
}
