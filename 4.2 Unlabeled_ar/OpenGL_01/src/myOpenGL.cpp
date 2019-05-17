#include "myOpenGL.hpp"
myOpenGL::myOpenGL(unsigned int width, unsigned int height, float fx, float fy, float cx, float cy) {
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	float cameraMatrix[16] = { fx,  0, 0, 0,
							   0, fy, 0, 0,
							   cx, cy, 1, 0,
							   0, 0, 0, 1 };
	projection = glm::make_mat4(cameraMatrix);
	view = glm::mat4(1.0f);
	init();
}
void myOpenGL::display(unsigned char *imgData, unsigned int width, unsigned int height,  float rtArray[]) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, texture2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	unsigned char* data = imgData;
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// render container
	imgShader->use();
	glBindVertexArray(VAO1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	squareShader->use();
	glm::mat4 model = glm::make_mat4(rtArray);
	squareShader->setMat4("model", model);
	squareShader->setMat4("projection", projection);
	squareShader->setMat4("view", view);
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glDrawArrays(GL_TRIANGLES, 0, 64);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool myOpenGL::checkEscape() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		return true;
	else return false;
	return false;
}

//void myOpenGL::framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	SCR_WIDTH = width;
//	SCR_HEIGHT = height;
//	glViewport(0, 0, width, height);
//}
int myOpenGL::init() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	squareShader = new Shader("shader/vertex.vs", "shader/fragment.fs");
	imgShader = new Shader("shader/vertex1.vs", "shader/fragment.fs");
	
	float vertices[] = {
	-30, -30, 0.0f,  0.0f, 0.0f,
	 30, -30, 0.0f,  1.0f, 0.0f,
	 30,  30, 0.0f,  1.0f, 1.0f,
	 30,  30, 0.0f,  1.0f, 1.0f,
	-30,  30, 0.0f,  0.0f, 1.0f,
	-30, -30, 0.0f,  0.0f, 0.0f,

	-30, -30,  60.0f,  0.0f, 0.0f,
	 30, -30,  60.0f,  1.0f, 0.0f,
	 30,  30,  60.0f,  1.0f, 1.0f,
	 30,  30,  60.0f,  1.0f, 1.0f,
	-30,  30,  60.0f,  0.0f, 1.0f,
	-30, -30,  60.0f,  0.0f, 0.0f,

	-30,  30,  60.0f,  1.0f, 0.0f,
	-30,  30, -0.0f,   1.0f, 1.0f,
	-30, -30, -0.0f,   0.0f, 1.0f,
	-30, -30, -0.0f,   0.0f, 1.0f,
	-30, -30,  60.0f,  0.0f, 0.0f,
	-30,  30,  60.0f,  1.0f, 0.0f,

	 30,  30,  60.0f,  1.0f, 0.0f,
	 30,  30, -0.0f,   1.0f, 1.0f,
	 30, -30, -0.0f,   0.0f, 1.0f,
	 30, -30, -0.0f,   0.0f, 1.0f,
	 30, -30,  60.0f,  0.0f, 0.0f,
	 30,  30,  60.0f,  1.0f, 0.0f,

	-30, -30, -0.0f,   0.0f, 1.0f,
	 30, -30, -0.0f,   1.0f, 1.0f,
	 30, -30,  60.0f,  1.0f, 0.0f,
	 30, -30,  60.0f,  1.0f, 0.0f,
	-30, -30,  60.0f,  0.0f, 0.0f,
	-30, -30, -0.0f,   0.0f, 1.0f,

	-30,  30, -0.0f,   0.0f, 1.0f,
	 30,  30, -0.0f,   1.0f, 1.0f,
	 30,  30,  60.0f,  1.0f, 0.0f,
	 30,  30,  60.0f,  1.0f, 0.0f,
	-30,  30,  60.0f,  0.0f, 0.0f,
	-30,  30, -0.0f,   0.0f, 1.0f,

	};
	float vertices1[] = {
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	};

	
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

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	int width1, height1, nrChannels1;
	unsigned char* data = stbi_load("texture1.png", &width1, &height1, &nrChannels1, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return 1;
}