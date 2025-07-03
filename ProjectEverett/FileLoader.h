#pragma once

#include "LGLStructs.h"

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "interfaces/ISolidSim.h"
#include "ScriptFuncStorage.h"
#include "AnimSystem.h"

// Assimp forward declarations
struct aiScene;
struct aiMesh;
struct aiNode;

// FreeType forward declarations
#define ForwardDeclarePtrTo(Type) \
struct Type##Rec_; \
using Type = Type##Rec_*;

ForwardDeclarePtrTo(FT_Library)
ForwardDeclarePtrTo(FT_Face)

// Other forward declarations
struct HINSTANCE__;
using HMODULE = HINSTANCE__*;

/*
* FileLoader will include all direct usages of APIs that just load and process files
* otherwise, will separate to other class and files (ShaderGenerator for example)
*/
class FileLoader
{
	class ModelLoader
	{
		const aiScene* modelHandle;
		std::vector<std::string> extraTextureName;
		std::map<std::string, LGLStructs::Texture> texturesLoaded;
		std::string nameToSet;

		using BoneMap = std::unordered_map<std::string, AnimSystem::BoneInfo>;

		void ProcessNodeForModelInfo(
			const aiNode* nodeHandle,
			LGLStructs::ModelInfo& model,
			BoneMap& boneMap
		);
		void ProcessNodeForBoneTree(
			const std::string& rootNodeName,
			const aiNode* nodeHandle,
			BoneMap& boneMap,
			AnimSystem::BoneTree::TreeManagerNode* parentTreeNode,
			glm::mat4& globalTransform
		);
		void LoadAnimations(
			AnimSystem::AnimKeyMap& animKeyMap,
			AnimSystem::AnimInfoVect& animInfo
		);

		bool GetTextureFilenames(const std::string& path);
		LGLStructs::Mesh ProcessMesh(const aiMesh* meshHandle, BoneMap& boneMap);
	public:
		bool LoadModel(
			const std::string& file,
			const std::string& name,
			LGLStructs::ModelInfo& model,
			AnimSystem::ModelAnim& modelAnim
		);
		bool LoadTexture(
			const std::string& file,
			LGLStructs::Texture& texture,
			unsigned char* data = nullptr,
			size_t dataSize = 0
		);

		void FreeTextureData();

		template<typename AssimpType, typename GLMCont>
		void ParseAnimInfo(AssimpType* keys, size_t keyAmount, GLMCont& glmCont);
	};


	class DLLLoader
	{
		std::recursive_mutex scriptWrapperLock;

		bool GetScriptFuncFromDLLImpl(
			const std::string& dllPath,
			const std::string& funcName,
			std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncPtr
		);

		using ScriptMap = std::map<std::string, std::pair<HMODULE, ScriptFuncStorage::ScriptFuncMainMap>>;

		ScriptMap dllHandleMap;
	public:
		void FreeDllData();
		bool GetScriptFuncFromDLL(
			const std::string& dllPath,
			const std::string& funcName,
			std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncPtr
		);
		void UnloadScriptDLL(const std::string& dllPath);
		std::vector<std::pair<std::string, std::string>> GetLoadedScriptDlls();
	};

	class FontLoader
	{
		struct FaceInfo
		{
			FT_Face face;
			std::map<char, std::vector<unsigned char>> glyphData;
			LGLStructs::GlyphInfo glyphInfo;
		};

		LGLStructs::GlyphTexture GetGlyphTextureOfImpl(FaceInfo& face, char c);

		FT_Library ft;
		std::map<std::string, FaceInfo> fontToFaceMap;

	public:
		FontLoader();
		~FontLoader();

		std::string LoadFontFromPath(const std::string& fontPath, int fontSize);

		LGLStructs::GlyphTexture GetGlyphTextureOf(const std::string& fontName, char c);
		LGLStructs::GlyphInfo& GetAllGlyphTextures(const std::string& fontName);

		void FreeFaceInfoByFont(const std::string& fontName, bool keepGlyphData = false);
	};

public:
	FileLoader();
	~FileLoader();

	static std::string GetCurrentDir();
	static bool GetFilesInDir(std::vector<std::string>& files, const std::string& dir);

	DLLLoader dllLoader;
	ModelLoader modelLoader;
	FontLoader fontLoader;
};

#undef ForwardDeclarePtrTo