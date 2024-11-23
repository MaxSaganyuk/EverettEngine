#pragma once

#include "LGLStructs.h"

#include <map>
#include <vector>
#include <string>

class aiScene;
class aiMesh;
class aiNode;

class FileLoader
{
	const aiScene* modelHandle;
	std::vector<std::string> extraTextureName;
	std::map<std::string, LGLStructs::Texture> texturesLoaded;

	void ProcessNode(const aiNode* nodeHandle, LGLStructs::ModelInfo& model);
	bool GetTextureFilenames(const std::string& path);
	LGLStructs::Mesh ProcessMesh(const aiMesh* meshHandle);
public:
	FileLoader();
	~FileLoader();

	bool LoadModel(const std::string& file, LGLStructs::ModelInfo& model);
	bool LoadTexture(
		const std::string& file,
		LGLStructs::Texture& texture, 
		unsigned char* data = nullptr, 
		size_t dataSize = -1
	);

	void FreeTextureData();
	bool GetFilesInDir(std::vector<std::string>& files, const std::string& dir);
};