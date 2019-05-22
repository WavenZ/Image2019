/************************************************
	@brief: Unlabeled AR

	@author: WavenZ

	@time: 2019/5/12
*************************************************/
#include "src/ARPipeline.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <vector>
#include <ctime>
#include <thread>
#include <string>

#include "src/ARPipeline.hpp"
#include "src/myOpenGL.hpp"
void OpenCV_init();
void matToArray(cv::Mat& img, unsigned char* data);
void readThread(); // A thread to read image
void imgRead(); // Read image
void calcTimeCost(); // Calculate the time-cost of each frame

unsigned int SCR_WIDTH = 1000, SCR_HEIGHT = 562;
float fx =  781.283471919768;
float fy =  780.836322198165;
float cx =  498.488969668705;
float cy =  271.472298720301;
cv::Mat imgOrigin, imgBuffer;
cv::VideoCapture capture;
int width = 1000, height = 562, nrChannels = 3;
const std::string addr = "http://admin:admin@192.168.1.101:8081";

int useLoacalImg = 1;
int main(int argc, const char* argv[])
{
	OpenCV_init();
	unsigned char* imgData = new unsigned char[(unsigned int)(width * height * nrChannels)]; // Image data for OpenGL
	float* rtArray = new float[16]; // Rotate and translatoin matrix for OpenGL
	CameraCalibration calibration(fx, fy, cx, cy); // Camera calibration matrix for ARPipeline
	cv::Mat patternImage = cv::imread("Pattern.jpg"); // Pattern image
	ARPipeline pipeline(patternImage, calibration);
	myOpenGL myGL(SCR_WIDTH, SCR_HEIGHT, fx, fy, cx, cy);

	while (!myGL.checkEscape())
	{
		imgRead(); // Read image from capture or local
		matToArray(imgOrigin, imgData); // Convert opencv-mat to opengl-array
		pipeline.processFrame(imgOrigin); // Process a single frame
		Transformation Rt = pipeline.getPatternLocation(); // Get rotation-translation matrix
		myGL.display(imgData, width, height, (float*)Rt.getMat44().data); // Render the result with OpenGL
		calcTimeCost();
	}
	return 0;
}
// Calculate the time-comsuming
void calcTimeCost() {
	static clock_t time, time_last;
	time = clock();
	std::cout << "Time cost :" << (double)(time - time_last) / CLOCKS_PER_SEC << "s" << std::endl;
	time_last = time;
}
// OpenCV-mat to opengl-array
void matToArray(cv::Mat& img, unsigned char* data) {
	int height = img.rows, width = img.cols;
	unsigned char* p = data;
	for (int row = height - 1; row >= 0; row--) {
		const uchar* ptr = img.ptr(row);
		for (int col = 0; col < width; col++) {
			const uchar* uc_pixel = ptr;
			p[2] = uc_pixel[0];
			p[1] = uc_pixel[1];
			p[0] = uc_pixel[2];
			p += 3;
			ptr += 3;
		}
	}
}

int readFlag = 0;
// Image read thread.
void readThread() {
	while (1) {
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
		imgOrigin = cv::imread("test.bmp");
	}
	else {
		readFlag = 1;
		while (readFlag);
		resize(imgOrigin, imgOrigin, cv::Size(width, height));
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