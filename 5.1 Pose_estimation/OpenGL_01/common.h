#pragma once
#include <GLFW/glfw3.h>
typedef unsigned char uchar;
const int SUCESS = 1;
const int FAIL = 0;
int glfwInitialization(uchar major, uchar minor, int profile) {
	if (!glfwInit()) {
		return FAIL;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
	return SUCESS;
}

