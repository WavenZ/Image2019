#include "MarkerDetector.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <vector>

#include <ctime>
#include <thread>

#include "myOpenGL.hpp"
void OpenCV_init();
void calcTimeCost();
void matToArray(cv::Mat& img, unsigned char* data);
void readThread();
void imgRead();
// settings
unsigned int SCR_WIDTH = 1000;
unsigned int SCR_HEIGHT = 562;

float fx = 781.283471919768;
float fy = 780.836322198165;
float cx = 498.488969668705;
float cy = 271.472298720301;

float distCoefficients[4] = {
		3.3186929343409627e-01, -1.5328976453608059e+00,
	   -1.0555395111807473e-04, 5.3863533330160257e-05 };
cv::Mat imgOrigin, imgBuffer;
cv::VideoCapture capture;
int width = 1000, height = 562, nrChannels = 3;
const std::string addr = "http://admin:admin@192.168.1.109:8081";

int useLoacalImg = 0; // 0: Use local image    1£ºRead image from ipCamera
int main()
{
	OpenCV_init();
	unsigned char* imgData = new unsigned char[(size_t)width * height * nrChannels];
	CameraCalibration calib(fx, fy, cx, cy, distCoefficients);
	MarkerDetector markerDetector(calib);
	myOpenGL openGL(1000, 526, fx, fy, cx, cy);

	while (!openGL.checkEscape())
	{
		imgRead();
		matToArray(imgOrigin, imgData);
		markerDetector.processFrame(imgOrigin, 1);
		std::vector<Transformation> trFormation = markerDetector.getTransformations();

		float* trArray = new float[trFormation.size() * 16];
		for (int i = trFormation.size() - 1; i >= 0; i--) {
			float* temp = trFormation[i].getMat44().data;
			for (int j = 0; j < 16; j++) {
				trArray[16 * i + j] = temp[j];
			}
		}
		openGL.display(imgData, 1000, 562, trArray, trFormation.size());
		calcTimeCost();
	}
	cv::destroyAllWindows();
	return 0;
}
// Calculate the time-comsuming
void calcTimeCost() {
	static clock_t time, time_last;
	time = clock();
	std::cout << "Time cost :" << (double)(time - time_last) / CLOCKS_PER_SEC << "s" << std::endl;
	time_last = time;
}
void matToArray(cv::Mat& img, unsigned char* data) {
	int height = img.rows, width = img.cols;
	unsigned char* p = data;
	for (int row = height - 1; row >=0 ; row--) {
		const uchar* ptr = img.ptr(row);
		for (int col = 0; col < width; col++) {
			const uchar* uc_pixel = ptr;
			p[2] = uc_pixel[0];
			p[1] = uc_pixel[1];
			p[0] = uc_pixel[2];
			p += 3;
			ptr +=3;
		}
	}
}
int readFlag = 0;
// Image read thread.
void readThread() {
	while(1){
		capture >> imgBuffer;
		if (readFlag) {
			imgOrigin = imgBuffer;
			readFlag = 0;
		}
	}
	
}
// Image read
void imgRead() {
	if (useLoacalImg) {
		imgOrigin = cv::imread("VR21.jpg");
	}
	else {
		readFlag = 1;
		while (readFlag);
		resize(imgOrigin, imgOrigin, cv::Size(1000, 562));
	}
}
// Opencv initialization
void OpenCV_init() {
	if (!useLoacalImg) {
		capture.open(addr);
		std::thread t(readThread);
		t.detach();
	}
}