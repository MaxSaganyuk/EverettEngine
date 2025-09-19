#pragma once

#include "LGLStructs.h"
#include "EverettStructs.h"
#include "CommonStructs.h"

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
	struct ModelOwner
	{
		std::shared_ptr<LGLStructs::ModelInfo> model;
		std::shared_ptr<AnimSystem::ModelAnim> modelAnim;
		std::unordered_map<std::string, LGLStructs::Texture::TextureData> textureMap;

		ModelOwner();
		ModelOwner(ModelOwner&&) noexcept = default;
		~ModelOwner();

		ModelOwner(const ModelOwner&) = delete;
		ModelOwner& operator=(const ModelOwner&) = delete;
	};
	
	template<typename AssetType>
	class AssetManager
	{
	protected:
		std::unordered_map<std::string, AssetType> ownerContainer;
	public:
		void DeleteAbsentAssets(const std::unordered_set<std::string>& assetsToKeep);
	};

	class ModelLoader : public AssetManager<ModelOwner>
	{
		const aiScene* modelHandle;
		std::vector<std::string> extraTextureName;
		std::string fileProcessed;
		std::string nameToSet;

		using BoneMap = std::unordered_map<std::string, AnimSystem::BoneInfo>;
		using TempTexMap = std::unordered_map<std::string, LGLStructs::Texture>;

		void ProcessNodeForModelInfo(
			const aiNode* nodeHandle,
			std::weak_ptr<LGLStructs::ModelInfo> model,
			BoneMap& boneMap,
			TempTexMap& tempTexMap
		);
		void ProcessNodeForBoneTree(
			const std::string& rootNodeName,
			const aiNode* nodeHandle,
			BoneMap& boneMap,
			AnimSystem::BoneTree::TreeManagerNode* parentTreeNode,
			glm::mat4& globalTransform
		);
		void SetGlobalInverseTransform(
			const std::string& rootNodeName,
			std::weak_ptr<AnimSystem::ModelAnim> modelAnim
		);
		void LoadAnimations(
			AnimSystem::AnimKeyMap& animKeyMap,
			AnimSystem::AnimInfoVect& animInfo
		);

		template<typename AssimpType, typename GLMCont>
		void ParseAnimInfo(AssimpType* keys, size_t keyAmount, GLMCont& glmCont);

		bool GetTextureFilenames(const std::string& path);
		LGLStructs::Mesh ProcessMesh(const aiMesh* meshHandle, BoneMap& boneMap, TempTexMap& tempTexMap);
		bool LoadTexture(
			const std::string& file,
			LGLStructs::Texture& texture,
			unsigned char* data = nullptr,
			size_t dataSize = 0
		);
	public:
		bool LoadModel(
			const std::string& file,
			const std::string& name,
			std::weak_ptr<LGLStructs::ModelInfo>& model,
			std::weak_ptr<AnimSystem::ModelAnim>& modelAnim
		);
	};


	class DLLLoader
	{
		std::recursive_mutex scriptWrapperLock;

		bool GetScriptFuncFromDLLImpl(
			const std::string& dllPath,
			const std::string& funcName,
			std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncPtr
		);
		void SetNewDLLHandle(const std::string& dllPath, HMODULE dllHandle);

		constexpr static char cleanUpFuncName[] = "CleanUp";

		struct ScriptDLLInfo
		{
			HMODULE dllHandle;
			ScriptFuncStorage::ScriptFuncMainMap scriptFuncMap;
			std::function<void()> cleanUpFunc;
		};

		using ScriptDLLPath = std::string;
		using ScriptMap = std::map<ScriptDLLPath, ScriptDLLInfo>;

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

	class AudioLoader : public AssetManager<WavDataOwner>
	{
	public:
		WavData GetWavDataFromFile(const std::string& filename);
	};

public:
	FileLoader();
	~FileLoader();

	static std::string GetCurrentDir();
	static bool GetFilesInDir(std::vector<std::string>& files, const std::string& dir);

	void DeleteAllAbsentAssets(const EverettStructs::AssetPaths& assetPaths = {});

	DLLLoader dllLoader;
	ModelLoader modelLoader;
	FontLoader fontLoader;
	AudioLoader audioLoader;
};

#undef ForwardDeclarePtrTo