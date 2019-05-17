#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "std_img.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "shader/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include "MarkerDetector.hpp"
#include <ctime>
#include <thread>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
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
cv::Mat imgOrigin;
cv::Mat imgBuffer;
cv::VideoCapture capture;
int width = 1000, height = 562, nrChannels = 3;
int readFlag = 0;
int main()
{
	capture.open("http://admin:admin@192.168.1.109:8081");
	std::thread t(readThread);
	imgRead();
	float cameraMatrix[16] = { fx,  0, 0, 0,
							   0, fy, 0, 0,
							   cx, cy, 1, 0,
							   0, 0, 0, 1};
	// std::cout << imgOrigin << std::endl << std::endl << std::endl;
	unsigned char* imgData = new unsigned char[width * height * nrChannels];
	matToArray(imgOrigin, imgData);

	// GaussianBlur(imgGray, imgGray, Size(5, 5), 0);
	CameraCalibration calib(fx, fy, cx, cy, distCoefficients);
	MarkerDetector markerDetector(calib);
	markerDetector.processFrame(imgOrigin, 1);
	std::vector<Transformation> trFormation = markerDetector.getTransformations();
	float tr96[96] = {};
	for (int i = 0; i < trFormation.size(); i++) {
		float* temp = trFormation[i].getMat44().data;
		for (int j = 0; j < 16; j++) {
			tr96[16 * i + j] = temp[j];
		}
	}
	//cv::destroyAllWindows();
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
	Shader imgShader("shader/vertex1.vs", "shader/fragment.fs");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
	-36.0f, -36.0f, 0.0f,  0.0f, 0.0f,
	 36.0f, -36.0f, 0.0f,  1.0f, 0.0f,
	 36.0f,  36.0f, 0.0f,  1.0f, 1.0f,
	 36.0f,  36.0f, 0.0f,  1.0f, 1.0f,
	-36.0f,  36.0f, 0.0f,  0.0f, 1.0f,
	-36.0f, -36.0f, 0.0f,  0.0f, 0.0f,

	-36.0f, -36.0f,  70.0f,  0.0f, 0.0f,
	 36.0f, -36.0f,  70.0f,  1.0f, 0.0f,
	 36.0f,  36.0f,  70.0f,  1.0f, 1.0f,
	 36.0f,  36.0f,  70.0f,  1.0f, 1.0f,
	-36.0f,  36.0f,  70.0f,  0.0f, 1.0f,
	-36.0f, -36.0f,  70.0f,  0.0f, 0.0f,

	-36.0f,  36.0f,  70.0f,  1.0f, 0.0f,
	-36.0f,  36.0f, -0.0f,  1.0f, 1.0f,
	-36.0f, -36.0f, -0.0f,  0.0f, 1.0f,
	-36.0f, -36.0f, -0.0f,  0.0f, 1.0f,
	-36.0f, -36.0f,  70.0f,  0.0f, 0.0f,
	-36.0f,  36.0f,  70.0f,  1.0f, 0.0f,

	 36.0f,  36.0f,  70.0f,  1.0f, 0.0f,
	 36.0f,  36.0f, -0.0f,  1.0f, 1.0f,
	 36.0f, -36.0f, -0.0f,  0.0f, 1.0f,
	 36.0f, -36.0f, -0.0f,  0.0f, 1.0f,
	 36.0f, -36.0f,  70.0f,  0.0f, 0.0f,
	 36.0f,  36.0f,  70.0f,  1.0f, 0.0f,

	-36.0f, -36.0f, -0.0f,  0.0f, 1.0f,
	 36.0f, -36.0f, -0.0f,  1.0f, 1.0f,
	 36.0f, -36.0f,  70.0f,  1.0f, 0.0f,
	 36.0f, -36.0f,  70.0f,  1.0f, 0.0f,
	-36.0f, -36.0f,  70.0f,  0.0f, 0.0f,
	-36.0f, -36.0f, -0.0f,  0.0f, 1.0f,

	-36.0f,  36.0f, -0.0f,  0.0f, 1.0f,
	 36.0f,  36.0f, -0.0f,  1.0f, 1.0f,
	 36.0f,  36.0f,  70.0f,  1.0f, 0.0f,
	 36.0f,  36.0f,  70.0f,  1.0f, 0.0f,
	-36.0f,  36.0f,  70.0f,  0.0f, 0.0f,
	-36.0f,  36.0f, -0.0f,  0.0f, 1.0f,
	};
	float vertices1[] = {
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	};

	unsigned int VBO, VBO1, VAO, VAO1;
	glGenVertexArrays(1, &VAO);
	glGenVertexArrays(1, &VAO1);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


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



	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	data = imgData;
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	
	
	clock_t time = 0, time_last = 0;
	while (!glfwWindowShouldClose(window))
	{
		time = clock();
		std::cout << (double)(time - time_last) / CLOCKS_PER_SEC << "s" << std::endl;
		time_last = time;
		// input
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		imgRead();
		width = imgOrigin.cols, height = imgOrigin.rows, nrChannels = 3;
		matToArray(imgOrigin, imgData);
		markerDetector.processFrame(imgOrigin, 1);
		std::vector<Transformation> trFormation = markerDetector.getTransformations();
		float tr96[96] = {};
		for (int i = 0; i < trFormation.size(); i++) {
			float* temp = trFormation[i].getMat44().data;
			for (int j = 0; j < 16; j++) {
				tr96[16 * i + j] = temp[j];
			}
		}
		glBindTexture(GL_TEXTURE_2D, texture1); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		data = stbi_load("texture1.jpg", &width1, &height1, &nrChannels1, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glBindTexture(GL_TEXTURE_2D, texture2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		data = imgData;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// render container
		imgShader.use();
		glBindVertexArray(VAO1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glClear(GL_DEPTH_BUFFER_BIT);

		squareShader.use();
		glm::mat4 projection = glm::make_mat4(cameraMatrix);
		glm::mat4 view = glm::mat4(1.0f);
		for (int i = 0; i < trFormation.size(); i++) {
			glm::mat4 model = glm::make_mat4(tr96 + 16 * i);
			squareShader.setMat4("model", model);
			squareShader.setMat4("projection", projection);
			squareShader.setMat4("view", view);
			// squareShader.setInt("ourTexture", 0);
			glBindVertexArray(VAO);
			glBindTexture(GL_TEXTURE_2D, texture1);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glDrawArrays(GL_TRIANGLES, 0, 36);	
		}

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
	readFlag = 1;
	while (readFlag);
	resize(imgOrigin, imgOrigin, cv::Size(1000, 562));
}