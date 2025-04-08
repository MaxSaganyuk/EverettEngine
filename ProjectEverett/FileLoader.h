#pragma once

#include "LGLStructs.h"

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "interfaces/ISolidSim.h"
#include "ScriptFuncStorage.h"

class aiScene;
class aiMesh;
class aiNode;
class SolidSim;
class HINSTANCE__;
using HMODULE = HINSTANCE__*;

class FileLoader
{
	const aiScene* modelHandle;
	std::vector<std::string> extraTextureName;
	std::map<std::string, LGLStructs::Texture> texturesLoaded;
	std::string nameToSet;

	std::recursive_mutex scriptWrapperLock;

	void ProcessNode(const aiNode* nodeHandle, LGLStructs::ModelInfo& model);
	bool GetTextureFilenames(const std::string& path);
	LGLStructs::Mesh ProcessMesh(const aiMesh* meshHandle);

	bool GetScriptFuncFromDLLImpl(
		const std::string& dllPath,
		const std::string& funcName,
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncPtr
	);

	using ScriptMap = std::map<std::string, std::pair<HMODULE, ScriptFuncStorage::ScriptFuncMainMap>>;

	ScriptMap dllHandleMap;

public:
	FileLoader();
	~FileLoader();

	bool LoadModel(const std::string& file, const std::string& name, LGLStructs::ModelInfo& model);
	bool LoadTexture(
		const std::string& file,
		LGLStructs::Texture& texture, 
		unsigned char* data = nullptr, 
		size_t dataSize = 0
	);

	void FreeTextureData();
	void FreeDllData();
	std::string GetCurrentDir();
	bool GetFilesInDir(std::vector<std::string>& files, const std::string& dir);

	bool GetScriptFuncFromDLL(
		const std::string& dllPath,
		const std::string& funcName,
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncPtr
	);
	void UnloadScriptDLL(const std::string& dllPath);
	std::vector<std::string> GetLoadedScriptDlls();
};
