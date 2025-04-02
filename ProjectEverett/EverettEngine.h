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

class FileLoader;
class CameraSim;
class SolidSim;
class LightSim;
class SoundSim;
class CommandHandler;
class LGL;

namespace LGLStructs
{
	class ModelInfo;
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
		Sound
	};

	EVERETT_API EverettEngine();
	EVERETT_API ~EverettEngine();
	EVERETT_API void CreateAndSetupMainWindow(int windowWidth, int windowHeight, const std::string& title);
	EVERETT_API bool CreateModel(const std::string& path, const std::string& name);
	EVERETT_API bool CreateSolid(const std::string& modelName, const std::string& solidName);
	EVERETT_API void CreateLight(const std::string& lightName, LightTypes lightType);
	EVERETT_API void CreateSound(const std::string& path, const std::string& soundName);

	EVERETT_API std::vector<glm::vec3> GetObjectParamsByName(
		ObjectTypes objectType,
		const std::string& subtypeName, 
		const std::string& objectName
	);
	EVERETT_API void SetObjectParamsByName(
		ObjectTypes objectType,
		const std::string& subtypeName, 
		const std::string& objectName, 
		const std::vector<glm::vec3>& params
	);

	EVERETT_API void SetScriptToObject(
		ObjectTypes objectType, 
		const std::string& subtypeName, 
		const std::string& objectName, 
		const std::string& dllPath,
		const std::string& dllName
	);
	EVERETT_API void UnsetScriptFromObject(const std::string& objectName);
	EVERETT_API bool IsObjectScriptSet(const std::string& dllPath);


	EVERETT_API std::vector<std::string> GetModelInDirList(const std::string& path);
	EVERETT_API std::vector<std::string> GetSoundInDirList(const std::string& path);

	EVERETT_API std::vector<std::string> GetCreatedModels();
	EVERETT_API std::vector<std::string> GetNamesByObject(ObjectTypes objType);
	EVERETT_API std::vector<std::string> GetLightTypeList();

	EVERETT_API static std::vector<std::string> GetAllObjectTypeNames();
	EVERETT_API static std::string GetObjectTypeToName(ObjectTypes objectType);
	EVERETT_API static ObjectTypes GetObjectTypeToName(const std::string& objectName);
private:
	struct ModelSolidInfo;

	using ModelSolidsMap = std::unordered_map<std::string, ModelSolidInfo>;

	using LightShaderValueNames = std::vector<std::pair<std::string, std::vector<std::string>>>;
	using LightCollection = std::map<LightTypes, std::map<std::string, LightSim>>;
	using SoundCollection = std::map<std::string, SoundSim>;

	void LightUpdater();

	SolidSim* GetObjectFromMap(
		ObjectTypes objectType,
		const std::string& subtypeName,
		const std::string& objectName
	);

	template<typename Sim>
	std::vector<std::string> GetNameList(const std::map<std::string, Sim>& sims);

	std::vector<std::string> GetObjectsInDirList(const std::string& path, const std::vector<std::string>& fileTypes);

	std::vector<std::string> GetSolidList();
	std::vector<std::string> GetLightList();
	std::vector<std::string> GetSoundList();

	std::unique_ptr<LGL> mainLGL;
	std::unique_ptr<std::thread> mainLGLRenderThread;

	std::unique_ptr<FileLoader> fileLoader;
	std::unique_ptr<CommandHandler> cmdHandler;

	ModelSolidsMap MSM;
	LightCollection lights;
	SoundCollection sounds;

	static LightShaderValueNames lightShaderValueNames;
	static std::vector<std::pair<ObjectTypes, std::string>> objectTypes;

	std::unique_ptr<CameraSim> camera;
};