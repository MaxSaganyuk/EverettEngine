#pragma once

#ifdef EVERETT_EXPORT
#define EVERETT_API __declspec(dllexport)
#else
#define EVERETT_API __declspec(dllimport)
#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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

#include "WindowHandleHolder.h"
#include "UnorderedPtrMap.h"

#include "interfaces/IObjectSim.h"
#include "interfaces/ISolidSim.h"
#include "interfaces/ILightSim.h"
#include "interfaces/ISoundSim.h"
#include "interfaces/ICameraSim.h"

class FileLoader;
class ObjectSim;
class CameraSim;
class SolidSim;
class LightSim;
class SoundSim;
class CommandHandler;
class LGL;
class WindowHandleHolder;
class ScriptFuncStorage;
class AnimSystem;

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

	struct LightParams
	{
		float constant;
		float linear;
		float quadratic;

		glm::vec3 direction;
		float cutOff;
		float outerCutOff;
	};

	enum class ObjectTypes
	{
		Camera,
		Solid,
		Light,
		Sound,
		_SIZE
	};

	EVERETT_API EverettEngine();
	EVERETT_API ~EverettEngine();
	EVERETT_API void CreateAndSetupMainWindow(int windowWidth, int windowHeight, const std::string& title);

	EVERETT_API bool CreateModel(const std::string& path, const std::string& name);
	EVERETT_API bool CreateSolid(const std::string& modelName, const std::string& solidName);
	EVERETT_API bool CreateLight(const std::string& lightName, LightTypes lightType);
	EVERETT_API bool CreateSound(const std::string& path, const std::string& soundName);

	EVERETT_API bool DeleteModel(const std::string& modelName);
	EVERETT_API bool DeleteSolid(const std::string& solidName);
	EVERETT_API bool DeleteLight(const std::string& lightName);
	EVERETT_API bool DeleteSound(const std::string& soundName);

	EVERETT_API IObjectSim* GetObjectInterface(
		ObjectTypes objectType,
		const std::string& subtypeName,
		const std::string& objectName
	);
	EVERETT_API ISolidSim* GetSolidInterface(
		const std::string& modelName,
		const std::string& solidName
	);
	EVERETT_API ILightSim* GetLightInterface(
		const std::string& lightTypeName,
		const std::string& lightName
	);
	EVERETT_API ISoundSim* GetSoundInterface(
		const std::string& soundName
	);
	EVERETT_API ICameraSim* GetCameraInterface();

	EVERETT_API void SetScriptToObject(
		ObjectTypes objectType, 
		const std::string& subtypeName, 
		const std::string& objectName, 
		const std::string& dllPath,
		const std::string& dllName
	);
	EVERETT_API bool IsObjectScriptSet(
		ObjectTypes objectType,
		const std::string& subtypeName,
		const std::string& objectName,
		const std::string& dllName
	);

	EVERETT_API void SetScriptToKey(
		const std::string& keyName,
		bool holdable,
		const std::string& dllPath,
		const std::string& dllName
	);
	EVERETT_API bool IsKeyScriptSet(
		const std::string& keyName,
		const std::string& dllName
	);

	EVERETT_API void UnsetScript(const std::string& dllName);

	EVERETT_API std::vector<std::string> GetModelInDirList(const std::string& path);
	EVERETT_API std::vector<std::string> GetSoundInDirList(const std::string& path);

	EVERETT_API std::vector<std::string> GetCreatedModels();
	EVERETT_API std::vector<std::string> GetNamesByObject(ObjectTypes objType);
	EVERETT_API static std::vector<std::string> GetLightTypeList();

	EVERETT_API static std::vector<std::string> GetAllObjectTypeNames();
	EVERETT_API static std::string GetObjectTypeToName(ObjectTypes objectType);
	EVERETT_API static ObjectTypes GetObjectTypeToName(const std::string& objectName);

	// Must be called on separate thread
	EVERETT_API int PollForLastKeyPressed();

	EVERETT_API static std::string ConvertKeyTo(int keyId);
	EVERETT_API static int         ConvertKeyTo(const std::string& keyName);

	EVERETT_API void ForceFocusOnWindow(const std::string& name);
	EVERETT_API void AddWindowHandler(HWND windowHandler, const std::string& name);
	EVERETT_API void AddCurrentWindowHandler(const std::string& name);

	EVERETT_API std::string GetAvailableObjectName(const std::string& name);

	EVERETT_API std::string GetSaveFileType();

	EVERETT_API bool SaveDataToFile(const std::string& filePath);
	EVERETT_API bool LoadDataFromFile(const std::string& filePath);
private:
#ifdef _DEBUG
	constexpr static inline char debugShaderPath[] = "\\..\\ProjectEverett\\shaders";
#endif
	struct ObjectTypeInfo;

	static inline const std::string saveFileType = ".esav";
	std::string defaultShaderProgram;

	size_t currentStartSolidIndex;
	size_t nextStartSolidIndex;

	std::function<void(double, double)> cursorCaptureCallback;

	struct ModelSolidInfo;

	using ModelSolidsMap = std::unordered_map<std::string, ModelSolidInfo>;

	using LightShaderValueNames = std::vector<std::pair<std::string, std::vector<std::string>>>;
	using LightCollection = std::map<LightTypes, std::map<std::string, LightSim>>;
	using SoundCollection = std::map<std::string, SoundSim>;

	bool CreateSolidImpl(const std::string& modelName, const std::string& solidName, bool regenerateShader);
	void GenerateShader();

	size_t GetCreatedSolidAmount();

	void LightUpdater();

	ObjectSim* GetObjectFromMap(
		ObjectTypes objectType,
		const std::string& subtypeName,
		const std::string& objectName
	);

	void SetScriptToObjectImpl(
		ObjectSim* object,
		const std::string& objectName,
		const std::string& dllPath,
		const std::string& dllName
	);

	template<typename Sim>
	std::vector<std::string> GetNameList(const std::map<std::string, Sim>& sims);

	std::vector<std::string> GetObjectsInDirList(const std::string& path, const std::vector<std::string>& fileTypes);

	struct ObjectTypeInfo;

	void CheckAndAddToNameTracker(const std::string& name);

	static std::type_index GetObjectPureTypeToName(ObjectTypes objectType);
	static std::type_index GetObjectPureTypeToName(const std::string& objectName);
	static ObjectTypes GetObjectPureTypeToName(std::type_index objectType);

	std::vector<std::string> GetSolidList();
	std::vector<std::string> GetLightList();
	std::vector<std::string> GetSoundList();

	template<typename Sim>
	void SaveObjectsToFile(std::fstream& file);

	template<typename Sim>
	void ApplySimInfoFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo, bool& res);

	void LoadCameraFromLine(std::string_view& line);
	void LoadSolidFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadLightFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadSoundFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo);
	void LoadKeybindsFromLine(std::string_view& line);

	void ResetEngine();

	std::unique_ptr<LGL> mainLGL;
	std::unique_ptr<std::thread> mainLGLRenderThread;

	std::unique_ptr<FileLoader> fileLoader;
	std::unique_ptr<CommandHandler> cmdHandler;

	std::unique_ptr<AnimSystem> animSystem;

	ModelSolidsMap MSM;
	LightCollection lights;
	SoundCollection sounds;

	static LightShaderValueNames lightShaderValueNames;
	static std::vector<ObjectTypeInfo> objectTypes;
	static std::vector<std::string> lightTypes;

	std::unique_ptr<CameraSim> camera;

	std::unique_ptr<WindowHandleHolder> hwndHolder;

	struct KeyScriptFuncInfo;

	std::map<std::string, KeyScriptFuncInfo> keyScriptFuncMap;

	UnorderedPtrMap<const std::string*, int> allNameTracker;

	class LastKeyPressPoll
	{
	public:
		LastKeyPressPoll();

		void KeyPressCallback(int key, int scancode, int action, int mods);
		void WaitForKeyPress();
		int GetLastKeyPressedID();

	private:
		std::mutex mux;
		std::condition_variable cv;
	    int lastKeyPressedID;
		bool isValidKeyPress;
	};
};