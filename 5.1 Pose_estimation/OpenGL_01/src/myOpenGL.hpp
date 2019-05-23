/************************************************
	@brief: Render

	@author: WavenZ

	@time: 2019/5/8
*************************************************/
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "std_img.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

class myOpenGL {
private:
	GLFWwindow* window;
	unsigned int SCR_HEIGHT, SCR_WIDTH;
	unsigned int VBO, VBO1, VAO, VAO1;
	// static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	Shader* squareShader;
	Shader* imgShader;
	unsigned int texture1, texture2;
	glm::mat4 projection, view, model;
	glm::vec3 lightPos;
public:
	myOpenGL(unsigned int width, unsigned int height, float fx, float fy, float cx, float cy);
	~myOpenGL() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glfwTerminate();
	};
	int init();
	bool checkEscape();
	void display(unsigned char* imgData, unsigned int width, unsigned int height, float rtArray[], int num);
};

