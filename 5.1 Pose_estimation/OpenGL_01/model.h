#pragma once
#include "myAssimp.hpp"
#include <vector>
#include <string>
#include "shader/shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
	Model(char* path) {
		loadModel(path);
	}
	void Draw(Shader shader);
private:
	vector<Mesh> meshes;
	string directory;
	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTexture(aiMaterial* mat, aiTextureType type,
		string typeName);
};
void Model::Draw(Shader shader) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}