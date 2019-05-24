#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "std_img.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include "shader/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include "MarkerDetector.hpp"
using namespace cv;
using namespace std;
void readThread();
void imgRead();
void camera_init();
void calcModel(glm::mat4& model, Mat& Rvec, Mat_<float>& Tvec);
Point2f calcCenter(vector<Point>& contour);
void findMarkers(Mat& imgThre, vector<Point2f>& centers);
void drawMarkers(Mat& img, vector<Point2f>& centers);
void sortMarkers(vector<Point2f>& markers);
void estimatePosition(vector<Point2f>& markers, Mat& cameraMatrix, Mat& distCoeffs, Mat& Rvec, Mat_<float>& Tvec);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void matToArray(cv::Mat& img, unsigned char* data);
static void onMouse(int event, int x, int y, int flags, void* param);
// settings
#define PI 3.14159265358979323846   // pi
#define CV_EVENT_LBUTTONDOWN 1           // left click
unsigned int SCR_WIDTH = 1366;
unsigned int SCR_HEIGHT = 768;

Mat cameraMatrix = (Mat_<double>(3, 3) <<
	781.283471919768, 0., 498.488969668705,
	0., 780.836322198165, 271.472298720301,
	0., 0., 1.);

Mat distCoeffs = (Mat_<double>(5, 1) << 3.3186929343409627e-01, -1.5328976453608059e+00,
	-1.0555395111807473e-04, 5.3863533330160257e-05, 0.);
cv::Mat imgOrigin, imgBuffer, imgHsv;
cv::VideoCapture capture;
const std::string addr = "http://admin:admin@192.168.1.100:8081";
int useLoacalImg = 0;
int main()
{
	camera_init();

	// 以下全是OpenGl的初始化
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader squareShader("shader/vertex.vs", "shader/fragment.fs");
	Shader axisShader("shader/vertex1.vs", "shader/fragment1.fs");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
	-20.0f, -40.0f, -0.0f,  0.0f, 0.0f,
	 80.0f, -40.0f, -0.0f,  1.0f, 0.0f,
	 80.0f,  20.0f, -0.0f,  1.0f, 1.0f,
	 80.0f,  20.0f, -0.0f,  1.0f, 1.0f,
	-20.0f,  20.0f, -0.0f,  0.0f, 1.0f,
	-20.0f, -40.0f, -0.0f,  0.0f, 0.0f,

	-20.0f, -40.0f,  10.0f,  0.0f, 0.0f,
	 80.0f, -40.0f,  10.0f,  1.0f, 0.0f,
	 80.0f,  20.0f,  10.0f,  1.0f, 1.0f,
	 80.0f,  20.0f,  10.0f,  1.0f, 1.0f,
	-20.0f,  20.0f,  10.0f,  0.0f, 1.0f,
	-20.0f, -40.0f,  10.0f,  0.0f, 0.0f,

	-20.0f,  20.0f,  10.0f,  1.0f, 0.0f,
	-20.0f,  20.0f, -0.0f,  1.0f, 1.0f,
	-20.0f, -40.0f, -0.0f,  0.0f, 1.0f,
	-20.0f, -40.0f, -0.0f,  0.0f, 1.0f,
	-20.0f, -40.0f,  10.0f,  0.0f, 0.0f,
	-20.0f,  20.0f,  10.0f,  1.0f, 0.0f,

	 80.0f,  20.0f,  10.0f,  1.0f, 0.0f,
	 80.0f,  20.0f, -0.0f,  1.0f, 1.0f,
	 80.0f, -40.0f, -0.0f,  0.0f, 1.0f,
	 80.0f, -40.0f, -0.0f,  0.0f, 1.0f,
	 80.0f, -40.0f,  10.0f,  0.0f, 0.0f,
	 80.0f,  20.0f,  10.0f,  1.0f, 0.0f,

	-20.0f, -40.0f, -0.0f,  0.0f, 1.0f,
	 80.0f, -40.0f, -0.0f,  1.0f, 1.0f,
	 80.0f, -40.0f,  10.0f,  1.0f, 0.0f,
	 80.0f, -40.0f,  10.0f,  1.0f, 0.0f,
	-20.0f, -40.0f,  10.0f,  0.0f, 0.0f,
	-20.0f, -40.0f, -0.0f,  0.0f, 1.0f,

	-20.0f,  20.0f, -0.0f,  0.0f, 1.0f,
	 80.0f,  20.0f, -0.0f,  1.0f, 1.0f,
	 80.0f,  20.0f,  10.0f,  1.0f, 0.0f,
	 80.0f,  20.0f,  10.0f,  1.0f, 0.0f,
	-20.0f,  20.0f,  10.0f,  0.0f, 0.0f,
	-20.0f,  20.0f, -0.0f,  0.0f, 1.0f,
	};
	int k = 0;
	//float vertices1[41*2*6*3] = {
	float vertices1[4 * 3 * 41] = {};
	for (int i = 0; i < 41; i++) {
		// z
		vertices1[k + 12 * i + 0] = -400;
		vertices1[k + 12 * i + 1] = i * 20 - 400;
		vertices1[k + 12 * i + 3] = 400;
		vertices1[k + 12 * i + 4] = i * 20 - 400;
		//
		vertices1[k + 12 * i + 6 + 0] = i * 20 - 400;
		vertices1[k + 12 * i + 6 + 1] = -400;
		vertices1[k + 12 * i + 6 + 3] = i * 20 - 400;
		vertices1[k + 12 * i + 6 + 4] = 400;
	}
	vertices1[k + 12 * 20 + 0] -= 100;
	vertices1[k + 12 * 20 + 3] += 100;
	vertices1[k + 12 * 20 + 6 + 1] -= 100;
	vertices1[k + 12 * 20 + 6 + 4] += 100;

	float vertices2[3 * 3 * 2] = {
		0, 500, 0,
		-9, 488, 0,
		6, 488, 0,
		500, 0, 0,
		488, 9, 0,
		488, -9, 0
	};

	unsigned int VBO, VBO1, VBO2, VAO, VAO1, VAO2;
	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAO1);
	glGenVertexArrays(1, &VAO2);

	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO1);
	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindVertexArray(VAO2);
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// load and create a texture 
	// -------------------------
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	stbi_set_flip_vertically_on_load(true);
	int width1, height1, nrChannels1;
	unsigned char* data = stbi_load("texture1.jpg", &width1, &height1, &nrChannels1, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}


	squareShader.use();
	glm::mat4 view(1.0f);
	view = glm::lookAt(
		glm::vec3(0, -1100, 800), // 摄像机在世界坐标系的(4, 3, 3)处
		glm::vec3(0, 0, 0), // 摄像机对着原点
		glm::vec3(0, 0, 1)  // 头朝上
	);
	glm::mat4 projection(1.0f);
	projection = glm::perspective(glm::radians(30.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 model(1.0f);
	// model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	squareShader.setMat4("projection", projection);
	squareShader.setMat4("view", view);
	squareShader.setMat4("model", model);
	axisShader.use();
	axisShader.setMat4("projection", projection);
	axisShader.setMat4("view", view);
	axisShader.setMat4("model", model);
	// 以上全是OpenGl的初始化

	namedWindow("img", 2);
	resizeWindow("img", 1000, 562);
	setMouseCallback("img", onMouse, reinterpret_cast<void*> (&imgHsv)); // 鼠标
	Mat img, imgThre;
	Mat Rvec; // 世界坐标系到摄像机坐标系的旋转欧拉角
	Mat_<float> Tvec; // 世界坐标系到摄像机坐标系的平移矢量
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window); // 窗口输入事件
		imgRead(); // 读图
		imshow("img", imgOrigin);
		waitKey(1);
		GaussianBlur(imgOrigin, imgOrigin, cv::Size(3, 3), 1, 1);
		// 1. 转换到HSV空间，用以提取颜色特征
		// 2. 提取出四个特征点并按照空间关系排序
		// 3. 计算出旋转平移矢量并估计摄像机姿态(solvePnp)
		// 4. OpenGL渲染
		cvtColor(imgOrigin, imgHsv, 40);
		vector<Point2f> markers;
		inRange(imgHsv, Scalar(115, 180, 150), Scalar(135, 255, 255), imgThre);
		findMarkers(imgThre, markers); // 保证蓝色点是第一个点
		Mat imgThre1;
		inRange(imgHsv, Scalar(0, 140, 150), Scalar(10, 255, 255), imgThre1);
		Mat imgThre2;
		inRange(imgHsv, Scalar(170, 140, 150), Scalar(180, 255, 255), imgThre2);
		Mat imgThre3 = imgThre1 + imgThre2;
		findMarkers(imgThre3, markers);
		if (markers.size() != 4) continue;
		imgThre = imgThre3 + imgThre;
		drawMarkers(imgOrigin, markers);
		sortMarkers(markers);
		estimatePosition(markers, cameraMatrix, distCoeffs, Rvec, Tvec);
		calcModel(model, Rvec, Tvec);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		axisShader.use();
		glBindVertexArray(VAO1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO1);
		glDrawArrays(GL_LINES, 0, 4 * 3 * 41);
		glBindVertexArray(VAO2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO2);
		glDrawArrays(GL_TRIANGLES, 0, 3*3*2 );
		squareShader.use();
		squareShader.setMat4("model", model);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindTexture(GL_TEXTURE_2D, texture1);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
void calcModel(glm::mat4& model, Mat& Rvec, Mat_<float>& Tvec) {
	cv::Mat_<float> rotMat(3, 3);
	cv::Rodrigues(Rvec, rotMat);
	cv::Mat_<float> RTMat(4, 4);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			RTMat(i, j) = rotMat(i, j);
		}
		RTMat(i, 3) = Tvec(i);
	}
	RTMat(3, 0) = 0;
	RTMat(3, 1) = 0;
	RTMat(3, 2) = 0;
	RTMat(3, 3) = 1;
	cv::Mat_<float> RTMat_(4, 4);
	cv::invert(RTMat, RTMat_, cv::DECOMP_SVD);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			model[j][i] = RTMat_(i, j);
		}
	}
}

static void onMouse(int event, int x, int y, int flags, void* param)
{	// 鼠标点击事件
	Mat* im = reinterpret_cast<Mat*>(param);
	if (event == CV_EVENT_LBUTTONDOWN) {
		std::cout << (int)imgHsv.at<Vec3b>(y, x)[0] << " ";
		std::cout << (int)imgHsv.at<Vec3b>(y, x)[1] << " ";
		std::cout << (int)imgHsv.at<Vec3b>(y, x)[2] << std::endl;
	}

}

void estimatePosition(vector<Point2f>& markers, Mat& cameraMatrix, Mat& distCoeffs, Mat& Rvec, Mat_<float>& Tvec) {
	Mat raux, taux;
	vector<Point3f> position = {
		Point3f{-25.0f,  -25.0f, 0.0f},
		Point3f{ 25.0f,  -25.0f, 0.0f},
		Point3f{ 25.0f,  25.0f, 0.0f},
		Point3f{-25.0f,  25.0f, 0.0f}
	};

	solvePnP(position, markers, cameraMatrix, distCoeffs, raux, taux);
	//raux.at<double>(1) = -raux.at<double>(1);
	//taux.at<double>(1) = -taux.at<double>(1);
	raux.convertTo(Rvec, CV_32F);
	taux.convertTo(Tvec, CV_32F);
}



void sortMarkers(vector<Point2f>& markers) {
	Point center;
	float n = markers.size();
	if (n != 4) return;
	float x = 0, y = 0;
	for (int i = 0; i < n; i++) {
		x += markers[i].x / n;
		y += markers[i].y / n;
	}
	center.x = (int)x;
	center.y = (int)y;
	vector<double> angles;
	for (Point marker : markers) {
		Point temp = marker - center;
		temp.y = -temp.y; // 图像坐标系y轴朝下
		angles.push_back(atan2(temp.y, temp.x) > 0 ?
			atan2(temp.y, temp.x) : 2 * PI + atan2(temp.y, temp.x)); // 以中点为坐标原点计算各点的方向
	}
	double offset = angles[0]; // 坐标系转换，蓝色点转换到x轴上
	for (double& angle : angles) {
		angle = angle - offset;
		if (angle < 0) angle += 2 * PI;
	}
	for (int j = 2; j < markers.size(); j++) { // 利用角度进行插入排序
		float key = angles[j];
		Point value = markers[j];
		int i = j - 1;
		while (i > 0 && angles[i] > key) {
			angles[i + 1] = angles[i];
			markers[i + 1] = markers[i];
			i--;
		}
		angles[i + 1] = key;
		markers[i + 1] = value;
	}
	return;
}
void findMarkers(Mat & imgThre, vector<Point2f> & centers) {
	vector<vector<Point>> contoursTemp, contours;
	cv::findContours(imgThre, contoursTemp, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	for (int i = 0; i < contoursTemp.size(); i++)
	{
		int contourSize = contoursTemp[i].size();
		if (contourSize < 200 && contourSize > 20)
		{
			contours.push_back(contoursTemp[i]);
		}
	}
	for (auto contour : contours) {
		centers.push_back(calcCenter(contour));
	}
}
void drawMarkers(Mat & img, vector<Point2f> & centers) {
	for (auto point : centers) {
		line(img, Point{ (int)point.x - 5, (int)point.y - 5 },
			Point{ (int)point.x + 5, (int)point.y + 5 }, Scalar(0, 0, 200), 1.5);
		line(img, Point{ (int)point.x - 5, (int)point.y + 5 },
			Point{ (int)point.x + 5, (int)point.y - 5 }, Scalar(0, 0, 200), 1.5);
	}
}

Point2f calcCenter(vector<Point> & contour) {
	float n = contour.size();
	float x = 0, y = 0;
	for (int i = 0; i < n; i++) {
		x += contour[i].x / n;
		y += contour[i].y / n;
	}
	return Point2f{ x, y };
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{

	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
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
		imgOrigin = cv::imread("test.jpg");
	}
	else {
		readFlag = 1;
		while (readFlag);
		resize(imgOrigin, imgOrigin, cv::Size(1000, 562));
	}
}
void camera_init() {
	if (!useLoacalImg) {
		capture.open(addr);
		std::thread t(readThread);
		t.detach();
	}
}