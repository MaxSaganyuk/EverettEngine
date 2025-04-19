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

	using BoneMap = std::unordered_map<std::string, LGLStructs::ModelInfo::BoneInfo>;

	void ProcessNodeForModelInfo(
		const aiNode* nodeHandle, 
		LGLStructs::ModelInfo& model, 
		BoneMap& boneMap
	);
	void ProcessNodeForBoneTree(
		const std::string& rootNodeName,
		const aiNode* nodeHandle,
		BoneMap& boneMap,
		LGLStructs::ModelInfo::BoneTree::TreeManagerNode* parentTreeNode,
		glm::mat4& globalTransform
	);
	void LoadAnimations(
		LGLStructs::ModelInfo::AnimKeyMap& animKeyMap,
		LGLStructs::ModelInfo::AnimInfoVect& animInfo
	);
	
	bool GetTextureFilenames(const std::string& path);
	LGLStructs::Mesh ProcessMesh(const aiMesh* meshHandle, BoneMap& boneMap);

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

	template<typename AssimpType, typename GLMCont>
	void ParseAnimInfo(AssimpType* keys, size_t keyAmount, GLMCont& glmCont);

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
