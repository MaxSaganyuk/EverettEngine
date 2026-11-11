#pragma once

#define EVERETT_EXPORT
#include "EverettEngine.h"

#include "FileLoader.h"

#include "LightSim.h"
#include "SolidSim.h"
#include "CameraSim.h"
#include "SoundSim.h"
#include "ColliderSim.h"

using LightShaderValueNames = std::vector<std::pair<std::string, std::vector<std::string>>>;

constexpr static char saveFileType[] = ".esav";
constexpr static char loggerFont[] = "consolab.ttf";
constexpr static char deleteObjErrorMes[] = "Cannot delete whilst scripts are running\n";
constexpr static char gizmoModelFile[] = "box.glb";
constexpr static char gizmoModelName[] = "Gizmo";

constexpr static glm::vec4 lightGizmoColor = ColorManager::GetColorVec4(ColorManager::Colors::YELLOW);
constexpr static glm::vec4 soundGizmoColor = ColorManager::GetColorVec4(ColorManager::Colors::BLUE);
constexpr static glm::vec4 colliderGizmoColor = ColorManager::GetColorVec4(ColorManager::Colors::GREEN);
constexpr static glm::vec4 colliderGizmoColorCollided = ColorManager::GetColorVec4(ColorManager::Colors::RED);

inline bool CheckHintAndType(
	const std::optional<EverettEngine::ObjectTypes>& hintType, const std::optional<EverettEngine::ObjectTypes>& objectType
)
{
	// If hint provided, but container does not have sim object type - ignore, because it is definitly invalid.
	bool check = !(hintType && !objectType);

	// If both hint and sim type provided - check only if equivalent.
	if (check && hintType && objectType)
	{
		check = hintType == objectType;
	}

	return check;
}

template<typename Sim>
inline ObjectSim* GetObjectFromTheMap(
	std::unordered_map<std::string, Sim>& simMap, const std::string& objectName
)
{
	auto iter = simMap.find(objectName);

	if (iter != simMap.end())
	{
		return dynamic_cast<ObjectSim*>(&iter->second);
	}

	return nullptr;
}

inline std::string CheckIfRelativePathToUse(const std::string& path, const std::string& expectedFolder)
{
	return path.find(':') != std::string::npos ? path : expectedFolder + "//" + path;
}

inline std::string EnsurePathHasSaveFileType(const std::string& path)
{
	return path.find('.') != std::string::npos ? path : path + '.' + saveFileType;
}

template<typename Sim>
inline std::generator<std::string_view> GetNameList(const std::unordered_map<std::string, Sim>& sims)
{
	for (auto& [name, _] : sims)
	{
		co_yield name;
	}
}

inline std::generator<std::string> GetObjectsInDirList(
	const std::string& path,
	const std::vector<std::string>& fileTypes
)
{
	for (auto&& file : FileLoader::GetFilesInDir(path))
	{
		for (auto& fileType : fileTypes)
		{
			if (file.find(fileType) != file.npos)
			{
				co_yield file;
				break;
			}
		}
	}
}

inline std::string GetDateTimeStr()
{
	auto now = std::chrono::zoned_time{
		std::chrono::current_zone(), std::chrono::system_clock::now()
	}.get_local_time();

	auto today = std::chrono::floor<std::chrono::days>(now);
	std::chrono::year_month_day ymd = std::chrono::year_month_day{ today };
	std::chrono::hh_mm_ss hms = std::chrono::hh_mm_ss{ now - today };

	return std::format(
		"{:02}-{:02}-{:04}-{:02}-{:02}-{:02}",
		static_cast<unsigned>(ymd.day()),
		static_cast<unsigned>(ymd.month()),
		static_cast<int>     (ymd.year()),
		static_cast<unsigned>(hms.hours().count()),
		static_cast<unsigned>(hms.minutes().count()),
		static_cast<unsigned>(hms.seconds().count())
	);
}

template<typename... Type>
inline void ExecuteVectorOfFuncs(const std::vector<std::function<void(Type...)>>& vectOfFuncs, Type... value)
{
	for (auto& func : vectOfFuncs)
	{
		func(std::forward<Type>(value)...);
	}
}

struct ObjectTypeInfo
{
	EverettEngine::ObjectTypes nameEnum;
	std::string nameStr;
	std::string interfaceNameStr;
};

inline LightShaderValueNames lightShaderValueNames =
{
	{ "material",{ "diffuse", "specular", "shininess" } },

	{ "pointLights",
		{
			"position", "diffuse",
			"specular", "constant", "linear",
			"quadratic"
		}
	},
	{ "spotLights",
		{
			"position", "direction",
			"diffuse", "specular", "constant",
			"linear", "quadratic", "cutOff",
			"outerCutOff"
		}
	}
};

#define ToStr(str) #str

inline std::vector<ObjectTypeInfo> objectTypes
{
	{ EverettEngine::ObjectTypes::Camera,   CameraSim::GetObjectTypeNameStr(), ToStr(ICameraSim) },
	{ EverettEngine::ObjectTypes::Solid,    SolidSim::GetObjectTypeNameStr(), ToStr(ISolidSim) },
	{ EverettEngine::ObjectTypes::Light,    LightSim::GetObjectTypeNameStr(), ToStr(ILightSim) },
	{ EverettEngine::ObjectTypes::Sound,    SoundSim::GetObjectTypeNameStr(), ToStr(ISolidSim) },
	{ EverettEngine::ObjectTypes::Collider, ColliderSim::GetObjectTypeNameStr(), ToStr(IColliderSim) }
};

#undef ToStr