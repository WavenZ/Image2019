#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "shader/shader.h"
using namespace std;
struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
};
struct Texture {
	unsigned int id;
	string type;
	string path;
};
class Mesh {
public:
	/* 网格数据 */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;
	/* 函数 */
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> texture);
	void Draw(Shader shader);
private:
	/* 渲染数据 */
	unsigned int VBO, EBO;
	/* 函数 */
	void setupMesh();
};