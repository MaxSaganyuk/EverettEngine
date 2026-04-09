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
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <chrono>
#include <typeindex>
#include <optional>

#include "UnorderedPtrMap.h"

#include "interfaces/IObjectSim.h"
#include "interfaces/ISolidSim.h"
#include "interfaces/ILightSim.h"
#include "interfaces/ISoundSim.h"
#include "interfaces/ICameraSim.h"
#include "interfaces/IColliderSim.h"

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

template<typename ParamType>
class ScriptFuncStorage;

struct HWND__;
using HWND = HWND__*;

namespace LGLStructs
{
	struct ModelInfo;
}

enum class LightTypes;

class EverettEngine
{
public:
	enum class LightTypes
	{
		Direction,
		Point,
		Spot
	};

	enum class ObjectTypes
	{
		Camera,
		Solid,
		Light,
		Sound,
		Collider,
		_SIZE
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

	EVERETT_API void SetDebugLogVisible(bool value = true);

	EVERETT_API void SetShaderPath(const std::string& shaderPath);
	EVERETT_API void SetFontPath(const std::string& fontPath);
	EVERETT_API void SetModelPath(const std::string& modelPath);

	EVERETT_API void SetDefaultWASDControls(bool value = true);
	
	EVERETT_API void EnableGizmoCreation();
	EVERETT_API void SetGizmoVisible(bool value = true);

	EVERETT_API void SetInteractable(
		char key, 
		bool holdable,
		std::function<void()> pressFunc, 
		std::function<void()> releaseFunc = nullptr
	);

	EVERETT_API void RunRenderWindow();
	EVERETT_API void StopRenderWindow();

	EVERETT_API bool CreateModel(const std::string& path, const std::string& name);
	EVERETT_API bool CreateSolid(const std::string& modelName, const std::string& solidName);
	EVERETT_API bool CreateLight(const std::string& lightName, LightTypes lightType);
	EVERETT_API bool CreateSound(const std::string& path, const std::string& soundName);
	EVERETT_API bool CreateCollider(const std::string& colliderName);

	EVERETT_API bool DeleteModel(const std::string& modelName);
	EVERETT_API bool DeleteSolid(const std::string& solidName);
	EVERETT_API bool DeleteLight(const std::string& lightName);
	EVERETT_API bool DeleteSound(const std::string& soundName);
	EVERETT_API bool DeleteCollider(const std::string& colliderName);

	EVERETT_API glm::vec3& GetAmbientLightVectorAddr(); 

	EVERETT_API IObjectSim* GetObjectInterface(
		ObjectTypes objectType,
		const std::string& objectName
	);
	EVERETT_API ISolidSim* GetSolidInterface(
		const std::string& solidName
	);
	EVERETT_API ILightSim* GetLightInterface(
		const std::string& lightName
	);
	EVERETT_API ISoundSim* GetSoundInterface(
		const std::string& soundName
	);
	EVERETT_API IColliderSim* GetColliderInterface(
		const std::string& colliderName
	);
	EVERETT_API ICameraSim* GetCameraInterface();

	EVERETT_API void SetupScriptDLL(const std::string& dllPath);
	EVERETT_API bool IsObjectScriptSet(
		ObjectTypes objectType,
		const std::string& objectName,
		const std::string& dllName
	);
	EVERETT_API bool IsDLLLoaded(const std::string& dllPath);
	EVERETT_API void UnsetScript(const std::string& dllPath);
	EVERETT_API std::vector<std::pair<std::string, std::string>> GetLoadedScriptDLLs();

	EVERETT_API std::vector<std::string> GetModelInDirList(const std::string& path);
	EVERETT_API std::vector<std::string> GetSoundInDirList(const std::string& path);

	EVERETT_API std::vector<std::string> GetCreatedModels(bool getFullPaths = false);
	// If getAdditionalInfo is set to true - list of solids will include model names, lights will include light type
	EVERETT_API std::vector<std::string> GetNamesByObject(ObjectTypes objType, bool getAdditionalInfo = false);
	EVERETT_API static std::vector<std::string> GetLightTypeList();

	EVERETT_API static std::vector<std::string> GetAllObjectTypeNames();
	EVERETT_API static std::string GetObjectTypeToName(ObjectTypes objectType);
	EVERETT_API static ObjectTypes GetObjectTypeToName(const std::string& objectName);

	EVERETT_API static std::string ConvertKeyTo(int keyId);
	EVERETT_API static int         ConvertKeyTo(const std::string& keyName);

	EVERETT_API void ForceFocusOnWindow(const std::string& name);
	EVERETT_API void AddWindowHandler(HWND windowHandler, const std::string& name);
	EVERETT_API void RemoveWindowHandler(const std::string& name);
	EVERETT_API void AddCurrentWindowHandler(const std::string& name);
	EVERETT_API void CloseWindow(const std::string& name);

	EVERETT_API std::string GetAvailableObjectName(const std::string& name);

	EVERETT_API std::string GetSaveFileType();

	EVERETT_API bool SaveDataToFile(const std::string& filePath);
	EVERETT_API bool LoadDataFromFile(const std::string& filePath);
	EVERETT_API EverettStructs::AssetPaths GetPathsFromWorldFile(const std::string& filePath);
	EVERETT_API void HidePathsInWorldFile(
		const std::string& originalFilePath, 
		const std::string& hidenFilePath
	);

	EVERETT_API void ResetEngine(const std::optional<EverettStructs::AssetPaths>& assetPaths = std::nullopt);

	EVERETT_API void CreateLogReport();
private:
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
	std::function<void(glm::vec4&&)> generalRenderTextBehaviour;

	size_t currentStartSolidIndex;
	size_t nextStartSolidIndex;

	std::function<void(double, double)> cursorCaptureCallback;

	struct ModelSolidInfo;

	using LightShaderValueNames = std::vector<std::pair<std::string, std::vector<std::string>>>;

	using ModelCollection    = std::unordered_map<std::string, ModelSolidInfo>; 
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

	SolidCollection::iterator DeleteSolidImpl(SolidCollection::iterator solidIter);

	void LoadGizmoModel();
	bool CreateGizmoSolid(
		const std::string& relatedObjModelName, 
		ObjectSim& relatedObject,
		const glm::vec4& gizmoColor
	);

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
		const std::string& objectName
	);

	template<typename Sim>
	ObjectSim* GetObjectFromTheMap(std::unordered_map<std::string, Sim>& simMap, const std::string& objectName);

	void SetScriptToObject(
		ObjectTypes objectType,
		const std::string& objectName,
		std::string_view rawFuncName,
		const std::string& dllPath,
		const std::string& dllName
	);
	void SetScriptToKey(
		const std::string& keyName,
		std::string_view pressedFuncName,
		std::string_view releasedFuncName,
		bool holdable,
		const std::string& dllPath,
		const std::string& dllName
	);
	void SetScriptToMouseScroll(
		std::string_view rawFuncName,
		const std::string& dllPath,
		const std::string& dllName
	);

	std::string CheckIfRelativePathToUse(const std::string& path, const std::string& expectedFolder);

	template<typename Sim>
	std::vector<std::string> GetNameList(const std::unordered_map<std::string, Sim>& sims);

	std::vector<std::string> GetObjectsInDirList(const std::string& path, const std::vector<std::string>& fileTypes);

	struct ObjectTypeInfo;

	void CheckAndAddToNameTracker(const std::string& name);

	static std::type_index GetObjectPureTypeToName(ObjectTypes objectType);
	static std::type_index GetObjectPureTypeToName(const std::string& objectName);
	static ObjectTypes GetObjectPureTypeToName(std::type_index objectType);

	std::vector<std::string> GetSolidList(bool getModelNames);
	std::vector<std::string> GetLightList(bool getLightTypes);
	std::vector<std::string> GetSoundList();
	std::vector<std::string> GetColliderList();

	template<typename Sim>
	void SaveObjectsToFile(std::fstream& file);

	template<typename Sim>
	void ApplySimInfoFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo, bool& res);

	void LoadCameraFromLine(std::string_view& line);
	void LoadSolidFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadLightFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadSoundFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadColliderFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadScriptDLLsFromLine(std::string_view& line);

	void SetLogCallback(bool value = true);
	void SetRenderLoggerCallbacks(bool value = true);
	std::string GetDateTimeStr();

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
	static std::vector<std::string> lightTypes;

	struct KeyScriptFuncInfo;
	struct MouseScrollScriptFuncInfo;

	std::map<std::string, KeyScriptFuncInfo> keyScriptFuncMap;
	std::unique_ptr<ScriptFuncStorage<double>> mouseScrollScriptFuncs;

	UnorderedPtrMap<const std::string*, int> allNameTracker;

	std::streambuf* stdOutStreamBuffer;
	std::streambuf* stdErrStreamBuffer;

	std::unique_ptr<CustomOutput> logOutput;
	std::unique_ptr<CustomOutput> errorOutput;
	std::list<std::string> logStrings;
};