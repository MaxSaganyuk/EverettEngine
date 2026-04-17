#pragma once

#include "LGLStructs.h"
#include "EverettStructs.h"
#include "CommonStructs.h"

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <typeindex>

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
	public:
		struct SimFuncNameInfo
		{
			std::string_view rawFuncName;
			std::string objectName;
			std::string interfaceTypeName;
		};

		struct KeybindFuncNameInfo
		{
			std::string_view rawPressedFuncName;
			std::string_view rawReleasedFuncName;
			bool holdable{};
		};

		using SimScriptFuncNameMultimap = std::multimap<size_t, SimFuncNameInfo>;
		using KeybindScriptFuncNameMap = std::unordered_map<std::string, KeybindFuncNameInfo>;
	private:
		std::recursive_mutex scriptWrapperLock;

		struct ScriptDLLInfo
		{
			HMODULE dllHandle;
			ScriptFuncMainMap scriptFuncMap;
			std::vector<std::string_view> rawScriptFuncNames;
			std::function<void()> mainFunc;
			std::function<void()> cleanUpFunc;
		};

		using ScriptDLLPath = std::string;
		using ScriptMap = std::map<ScriptDLLPath, ScriptDLLInfo>;

		bool LoadDLL(const std::string& dllPath);

		std::vector<std::string_view> GetRawScriptFuncNamesFromHandle(HMODULE handle);

		template<typename MultimapType>
		MultimapType GetScriptFuncNamesFromDLL(const std::string& dllPath, char indicatorChar);
		
		constexpr static char SimIndicator = 'S';
		constexpr static char KeybindIndicator = 'K';
		constexpr static char SeparatorChar = '_';
		constexpr static short SeparatorAmountInScriptFuncs = 4;
		using SeparatorArray = std::array<size_t, SeparatorAmountInScriptFuncs>;

		bool GetSeparatorPoints(std::string_view rawName, SeparatorArray& separatorPoints);
		void ParseRawNameFor(std::string_view rawName, SimScriptFuncNameMultimap& scriptFuncNameMap);
		void ParseRawNameFor(std::string_view rawName, KeybindScriptFuncNameMap& scriptFuncNameMap);

		bool GetScriptFuncFromDLLImpl(
			const std::string& funcName,
			ScriptDLLInfo& dllInfo,
			ScriptFuncWeakPtr& scriptFuncPtr
		);
		void SetNewDLLHandle(const std::string& dllPath, HMODULE dllHandle, ScriptDLLInfo& dllInfo);
		void UnloadScriptDLL(ScriptDLLInfo& dllInfo);

		constexpr static char mainScriptFuncName[] = "Main";
		constexpr static char cleanUpFuncName[] = "CleanUp";

		ScriptMap dllHandleMap;
	public:
		SimScriptFuncNameMultimap GetSimScriptFuncNamesFromDll(const std::string& dllPath);
		KeybindScriptFuncNameMap GetKeybindScriptFuncNamesFromDll(const std::string& dllPath);
		void FreeDllData();
		bool IsDLLLoaded(const std::string& dllPath);
		bool AnyDLLLoaded();
		bool GetScriptFuncFromDLL(
			const std::string& dllPath,
			const std::string& funcName,
			ScriptFuncWeakPtr& scriptFuncPtr
		);
		void UnloadScriptDLL(const std::string& dllPath);
		std::vector<EverettStructs::BasicFileInfo> GetLoadedScriptDlls();
		void ExecuteAllMainScriptFuncs();
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
	static std::string GetFileFromPath(const std::string& dllPath);
	static std::string GetFileHash(const std::string& path);

	void DeleteAllAbsentAssets(const EverettStructs::AssetPaths& assetPaths = {});

	DLLLoader dllLoader;
	ModelLoader modelLoader;
	FontLoader fontLoader;
	AudioLoader audioLoader;
};

#undef ForwardDeclarePtrTo