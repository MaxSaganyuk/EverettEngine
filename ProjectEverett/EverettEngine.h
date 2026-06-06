#pragma once

#ifdef EVERETT_EXPORT
#define EVERETT_API __declspec(dllexport)
#else
#define EVERETT_API __declspec(dllimport)
#endif

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <typeindex>
#include <optional>
#include <generator>
#include <expected>

#include "UnorderedPtrMap.h"

#include "external/IEverettEngine.h"

#include "EverettStructs.h"

class FileLoader;
class ObjectSim;
class CameraSim;
class SolidSim;
class LightSim;
class SoundSim;
class ColliderSim;
class CommandHandler;
class LGL;
class WindowHandleHolder;
class AnimSystem;
class RenderLogger;
class CustomOutput;
class ModelInfo;
class KeyScriptFuncInfo;

struct HWND__;
using HWND = HWND__*;

namespace LGLStructs
{
	struct ModelInfo;
}

enum class LightTypes;

// Concrete engine class is for internal use.
class EverettEngine : public IEverettEngine
{
public:
	enum class LightTypes
	{
		Direction,
		Point,
		Spot
	};

	EVERETT_API EverettEngine();
	EVERETT_API ~EverettEngine();
	EVERETT_API void CreateAndSetupMainWindow(
		int windowWidth, 
		int windowHeight, 
		const std::string& title,
		bool fullscreen = false,
		bool enableLogger = true
	);

	EVERETT_API void SetDebugLogVisible(bool value = true) override;

	EVERETT_API void SetShaderPath(const std::string& shaderPath);
	EVERETT_API void SetFontPath(const std::string& fontPath);
	EVERETT_API void SetModelPath(const std::string& modelPath);

	EVERETT_API void SetDefaultWASDControls(bool value = true);
	
	EVERETT_API void EnableGizmoCreation();
	EVERETT_API void SetGizmoVisible(bool value = true);

	EVERETT_API void RunRenderWindow();
	EVERETT_API void StopRenderWindow();

	EVERETT_API bool CreateModel(const std::string& path, const std::string& name);
	EVERETT_API bool CreateSolid(const std::string& modelName, const std::string& solidName);
	EVERETT_API bool CreateLight(const std::string& lightName, LightTypes lightType);
	EVERETT_API bool CreateSound(const std::string& path, const std::string& soundName);
	EVERETT_API bool CreateCollider(const std::string& colliderName);

	EVERETT_API std::expected<void, std::string> RenameObject(
		const std::string& oldName, const std::string& newName, std::optional<ObjectTypes> hintType = std::nullopt
	);
	EVERETT_API std::expected<void, std::string> DeleteObject(
		const std::string& name, std::optional<ObjectTypes> hintType = std::nullopt
	);

	EVERETT_API glm::vec3& GetAmbientLightVectorAddr(); 

	EVERETT_API IObjectSim* GetObjectInterface(
		const char* objectName, std::optional<ObjectTypes> hintType = std::nullopt
	) override;
	EVERETT_API ISolidSim* GetSolidInterface(const char* solidName) override;
	EVERETT_API ILightSim* GetLightInterface(const char* lightName) override;
	EVERETT_API ISoundSim* GetSoundInterface(const char* soundName) override;
	EVERETT_API IColliderSim* GetColliderInterface(const char* colliderName) override;
	EVERETT_API ICameraSim* GetCameraInterface() override;

	EVERETT_API bool CheckIfScriptsRunning(bool displayErrorIfYes = true);
	EVERETT_API void SetupScriptDLL(const std::string& dllPath);
	EVERETT_API bool IsDLLLoaded(const std::string& dllPath);
	EVERETT_API void UnsetScript(const std::string& dllPath);
	EVERETT_API std::generator<EverettStructs::BasicFileInfo> GetLoadedScriptDLLs();

	// Requesting for l-value ref prevents creating copy on call and prevents calling co-routine with "in place" temporary
	// ensuring correct execution despite the fact that "path" value will not be modified
	EVERETT_API std::generator<std::string> GetModelInDirList(std::string& path);
	EVERETT_API std::generator<std::string> GetSoundInDirList(std::string& path);

	EVERETT_API std::generator<std::string_view> GetCreatedModels(bool getFullPaths = false);
	// If getAdditionalInfo is set to true - list of solids will include model names, lights will include light type
	// Also if getAdditionalInfo is true, result is created on each iteration, so result must be copied if will be used post iteration.
	// This is because the result is a temporary, and resuing it post iteration is UB. Dangerous, but efficient and copyless.
	EVERETT_API std::generator<std::string_view> GetNamesByObject(ObjectTypes objType, bool getAdditionalInfo = false);
	EVERETT_API static std::generator<std::string_view> GetLightTypeList();

	EVERETT_API static std::generator<std::string_view> GetAllObjectTypeNames();
	EVERETT_API static std::string GetObjectTypeToName(ObjectTypes objectType);
	EVERETT_API static std::optional<ObjectTypes> GetObjectTypeToName(const std::string& objectName);
	
	EVERETT_API std::string ConvertKeyTo(int keyId);

	EVERETT_API int ConvertKeyTo(char c) override;
	EVERETT_API int ConvertKeyTo(const char* keyName) override;

	EVERETT_API void ForceFocusOnWindow(const std::string& name);
	EVERETT_API void AddWindowHandler(HWND windowHandler, const std::string& name);
	EVERETT_API void RemoveWindowHandler(const std::string& name);
	EVERETT_API void AddCurrentWindowHandler(const std::string& name);
	EVERETT_API void CloseWindow(const std::string& name);

	EVERETT_API std::string GetAvailableObjectName(const std::string& name);

	EVERETT_API std::string GetSaveFileType();

	EVERETT_API void AddWorldLoadCallback(std::function<void()> callback);
	EVERETT_API bool SaveWorldToFile(const std::string& filePath);
	EVERETT_API bool LoadWorldFromFile(const std::string& path);
	EVERETT_API void RequestWorldLoad(const char* filePath) override;
	EVERETT_API EverettStructs::AssetPaths GetPathsFromWorldFile(const std::string& filePath);
	EVERETT_API void HidePathsInWorldFile(
		const std::string& originalFilePath, 
		const std::string& hidenFilePath
	);

	EVERETT_API void ResetEngine(const std::optional<EverettStructs::AssetPaths>& assetPaths = std::nullopt);

	EVERETT_API void CreateLogReport() override;
private:
	enum ObjectModificationState : bool
	{
		FoundCorrectOne, CheckNextOne
	};

	std::string shaderPath = "shaders";
	std::string modelPath = "models";
	std::string fontPath = "fonts";

	bool gizmoVisible = false;
	bool gizmoEnabled = false;

	struct ObjectTypeInfo;

	static inline const std::string saveFileType = ".esav";
	std::string defaultShaderProgram;
	std::string defaultRenderTextShaderProgram;
	constexpr static inline char loggerFont[] = "consolab.ttf";
	constexpr static inline char deleteObjErrorMes[] = "Cannot delete whilst scripts are running\n";
	std::function<void(glm::vec4&&)> generalRenderTextBehaviour;

	size_t currentStartSolidIndex;
	size_t nextStartSolidIndex;

	using LightShaderValueNames = std::vector<std::pair<std::string, std::vector<std::string>>>;

	using ModelCollection    = std::unordered_map<std::string, ModelInfo>; 
	using SolidCollection    = std::unordered_map<std::string, SolidSim>;
	using LightCollection    = std::unordered_map<std::string, LightSim>;
	using SoundCollection    = std::unordered_map<std::string, SoundSim>;
	using ColliderCollection = std::unordered_map<std::string, ColliderSim>;

	constexpr static char gizmoModelFile[] = "box.glb";
	constexpr static char gizmoModelName[] = "Gizmo";
	constexpr static glm::vec4 lightGizmoColor            = { 1.0f, 1.0f, 0.0f, 1.0f };
	constexpr static glm::vec4 soundGizmoColor            = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr static glm::vec4 colliderGizmoColor         = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr static glm::vec4 colliderGizmoColorCollided = { 1.0f, 0.0f, 0.0f, 1.0f };

	static bool CheckHintAndType(
		const std::optional<ObjectTypes>& hintType, const std::optional<ObjectTypes>& objectType
	);

	SolidCollection::iterator DeleteSolidImpl(SolidCollection::iterator solidIter);

	ObjectModificationState DeleteModel(const std::string& modelName);
	ObjectModificationState DeleteSolid(const std::string& solidName);
	ObjectModificationState DeleteLight(const std::string& lightName);
	ObjectModificationState DeleteSound(const std::string& soundName);
	ObjectModificationState DeleteCollider(const std::string& colliderName);

	void LoadGizmoModel();
	bool CreateGizmoSolid(
		const std::string& relatedObjModelName, 
		ObjectSim& relatedObject,
		const glm::vec4& gizmoColor
	);

	void AddInteractable(
		int key,
		bool holdable,
		std::function<void()> pressFunc,
		std::function<void()> releaseFunc = nullptr
	) override;
	void AddMouseScrollCallback(std::function<void(double)> callback) override;
	void AddMouseMoveCallback(std::function<void(double, double)> callback) override;

	void ClearExternallyControlledContainers();

	void DeleteSolidsByModel(const std::string& modelName);
	void RemoveSolidPtrFromModel(const std::string& modelName, SolidSim* solidPtr);

	bool CreateModelImpl(const std::string& path, const std::string& name, bool regenerateShader);
	bool CreateSolidImpl(
		const std::string& modelName, const std::string& solidName, bool regenerateShader, bool forceVisible
	);
	bool CreateLightImpl(const std::string& lightName, LightTypes lightType);
	bool CreateSoundImpl(const std::string& path, const std::string& soundName);
	bool CreateColliderImpl(const std::string& colliderName);
	void GenerateShader();

	void LightUpdater();

	ObjectSim* GetObjectFromMap(
		ObjectTypes objectType,
		const std::string& objectName,
		bool logFail = true
	);

	template<typename Sim>
	ObjectSim* GetObjectFromTheMap(std::unordered_map<std::string, Sim>& simMap, const std::string& objectName);

	std::string CheckIfRelativePathToUse(const std::string& path, const std::string& expectedFolder);

	template<typename Sim>
	std::generator<std::string_view> GetNameList(const std::unordered_map<std::string, Sim>& sims);

	std::generator<std::string> GetObjectsInDirList(const std::string& path, const std::vector<std::string>& fileTypes);

	struct ObjectTypeInfo;

	void CheckAndAddToNameTracker(const std::string& name);

	std::generator<std::string_view> GetSolidList(bool getModelNames);
	std::generator<std::string_view> GetLightList(bool getLightTypes);
	std::generator<std::string_view> GetSoundList();
	std::generator<std::string_view> GetColliderList();

	template<typename Sim>
	void SaveObjectsToFile(std::fstream& file);

	template<typename Sim>
	void ApplySimInfoFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo, bool& res);

	void CheckAndLoadRequestedWorld();
	void LoadCameraFromLine(std::string_view& line);
	void LoadSolidFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadLightFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadSoundFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadColliderFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadScriptDLLsFromLine(std::string_view& line);

	void SetLogCallback(bool value = true);
	void SetRenderLoggerCallbacks(bool value = true);
	std::string GetDateTimeStr();

	template<typename... Type>
	void ExecuteVectorOfFuncs(const std::vector<std::function<void(Type...)>>& vectOfFuncs, Type... value);

	template<typename FunctionType, typename... Params>
	void ExecuteFuncForAllSimObjects(FunctionType func, Params&&... values);
	template<typename Sim, typename FunctionType, typename... Params>
	void ExecuteFuncForAllSimObjectsFor(std::unordered_map<std::string, Sim>& container, FunctionType func, Params&&... values);

	std::unique_ptr<LGL> mainLGL;

	std::shared_ptr<CameraSim> camera;
	
	std::unique_ptr<WindowHandleHolder> hwndHolder;
	std::unique_ptr<FileLoader> fileLoader;
	std::unique_ptr<CommandHandler> cmdHandler;
	std::unique_ptr<AnimSystem> animSystem;
	std::unique_ptr<RenderLogger> logger;

	ModelCollection models;
	SolidCollection solids;
	LightCollection lights;
	SoundCollection sounds;
	ColliderCollection colliders;

	static LightShaderValueNames lightShaderValueNames;
	static std::vector<ObjectTypeInfo> objectTypes;

	std::map<int, KeyScriptFuncInfo> keyScriptFuncMap;
	std::vector<std::function<void(double)>> mouseScrollScriptFuncs;
	std::vector<std::function<void(double, double)>> mouseMoveScriptFuncs;

	UnorderedPtrMap<const std::string*, int> allNameTracker;

	std::streambuf* stdOutStreamBuffer;
	std::streambuf* stdErrStreamBuffer;

	std::unique_ptr<CustomOutput> logOutput;
	std::unique_ptr<CustomOutput> errorOutput;
	std::list<std::string> logStrings;

	std::function<void()> worldLoadCallback;
	std::string worldToLoad;
};