#include <iostream>
#include <time.h>
#include <cmath>
#include <cstdlib>

#include "LGL.h"
#include "LGLUtils.h"
#include "Verts.h"

#include "MaterialSim.h"
#include "LightSim.h"
#include "SolidSim.h"
#include "CameraSim.h"
#include "SoundSim.h"

#include "FileLoader.h"

#include "CommandHandler.h"

#include "MazeGen.h"

#include "WindowHandleHolder.h"
#include "ScriptFuncStorage.h"

#include "stdEx/mapEx.h"

#define EVERETT_EXPORT
#include "EverettEngine.h"

struct EverettEngine::ModelSolidInfo
{
	LGLStructs::ModelInfo model;
	std::map<std::string, SolidSim> solids;
};

EverettEngine::LightShaderValueNames EverettEngine::lightShaderValueNames =
{
	{"material", { "diffuse", "specular", "shininess" }},
	{"pointLights",
		{
			"position", "diffuse",
			"specular", "constant", "linear",
			"quadratic"
		}
	},
	{"spotLights",
		{
			"position", "direction",
			"diffuse", "specular", "constant",
			"linear", "quadratic", "cutOff",
			"outerCutOff"
		}
	}
};

std::vector<std::pair<EverettEngine::ObjectTypes, std::string>> EverettEngine::objectTypes
{
	{EverettEngine::ObjectTypes::Camera, "Camera"}, 
	{EverettEngine::ObjectTypes::Solid,  "Solid" }, 
	{EverettEngine::ObjectTypes::Light,  "Light" },
	{EverettEngine::ObjectTypes::Sound,  "Sound" }
};

EverettEngine::EverettEngine()
{
	LGL::InitOpenGL(3, 3);
	SoundSim::InitOpenAL();

	fileLoader = std::make_unique<FileLoader>();
	cmdHandler = std::make_unique<CommandHandler>();
}

EverettEngine::~EverettEngine()
{
	mainLGLRenderThread->join();
	LGL::TerminateOpenGL();
}

void EverettEngine::CreateAndSetupMainWindow(int windowHeight, int windowWidth, const std::string& title)
{
	mainLGL = std::make_unique<LGL>();

	mainLGL->CreateWindow(windowHeight, windowWidth, title);

	hwndHolder = std::make_unique<WindowHandleHolder>();
	hwndHolder->AddCurrentWindowHandle("LGL");

	mainLGL->SetAssetOnOpenGLFailure(true);
#if _DEBUG
	std::string debugShaderPath = "\\..\\ProjectEverett\\shaders";
	mainLGL->SetShaderFolder(fileLoader->GetCurrentDir() + debugShaderPath);
#endif
	camera = std::make_unique<CameraSim>(windowHeight, windowWidth);;
	camera->SetMode(CameraSim::Mode::Fly);
	camera->SetGhostMode(true);

	SoundSim::SetCamera(*camera);

	mainLGL->SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	mainLGL->SetCursorPositionCallback(
		[this](double xpos, double ypos) { camera->Rotate(static_cast<float>(xpos), static_cast<float>(ypos)); }
	);
	mainLGL->SetScrollCallback(
		[this](double xpos, double ypos) { camera->Zoom(static_cast<float>(xpos), static_cast<float>(ypos)); }
	);

	std::string walkingDirections = "WSAD";
	for (size_t i = 0; i < walkingDirections.size(); ++i)
	{
		mainLGL->SetInteractable(
			walkingDirections[i],
			[this, i]() { camera->SetPosition(static_cast<CameraSim::Direction>(i)); }
		);
	}
	mainLGL->SetInteractable('R', [this]() { camera->SetPosition(CameraSim::Direction::Up); });

	mainLGL->GetMaxAmountOfVertexAttr();
	mainLGL->CaptureMouse();

	auto cameraMainLoop = [this](){
		camera->SetPosition(CameraSim::Direction::Nowhere);
		camera->ExecuteAllScriptFuncs();
	};

	mainLGLRenderThread = std::make_unique<std::thread>(
		[this, cameraMainLoop](){ mainLGL->RunRenderingCycle(cameraMainLoop); }
	);
}

int EverettEngine::PollForLastKeyPressed()
{
	LastKeyPressPoll lastKeyPressPoll;

	mainLGL->SetKeyPressCallback(
		[&lastKeyPressPoll](int key, int scancode, int action, int mods) { 
			lastKeyPressPoll.KeyPressCallback(key, scancode, action, mods); 
		}
	);
	
	lastKeyPressPoll.WaitForKeyPress();

	mainLGL->SetKeyPressCallback(nullptr);

	return lastKeyPressPoll.GetLastKeyPressedID();
}


std::string EverettEngine::ConvertKeyTo(int keyId)
{
	return LGL::ConvertKeyTo(keyId);
}

int EverettEngine::ConvertKeyTo(const std::string& keyName)
{
	return LGL::ConvertKeyTo(keyName);
}

bool EverettEngine::CreateModel(const std::string& path, const std::string& name)
{
	MSM.emplace(name, ModelSolidInfo{});
	LGLStructs::ModelInfo& newModel = MSM[name].model;
	
	if (!fileLoader->LoadModel(path, name, newModel))
	{
		MSM.erase(name);
		return false;
	}

	newModel.shaderProgram = "lightComb";
	newModel.render = false;
	newModel.behaviour = [this, name]()
	{
		LightUpdater();

		if(MSM.find(name) != MSM.end())
		{
			for (auto& solid : MSM.at(name).solids)
			{
				LGLUtils::SetShaderUniformStruct(
					*mainLGL, 
					lightShaderValueNames[0].first, 
					lightShaderValueNames[0].second, 
					0, 
					1, 
					0.5f
				);

				glm::mat4& modelMatrix = solid.second.GetModelMatrixAddr();
				mainLGL->SetShaderUniformValue("model", modelMatrix);
				mainLGL->SetShaderUniformValue("inv", glm::inverse(modelMatrix));

				if (SolidSim::CheckForCollision(*camera, solid.second))
				{
					camera->SetLastPosition();
				}
			}
		}
	};

	mainLGL->CreateModel(newModel);

	fileLoader->FreeTextureData();

	return true;
}

bool EverettEngine::CreateSolid(const std::string& modelName, const std::string& solidName)
{
	MSM[modelName].solids.emplace(solidName, camera->GetPositionVectorAddr() + camera->GetFrontVectorAddr());
	MSM[modelName].model.render = true;

	return true;
}

void EverettEngine::CreateLight(const std::string& lightName, LightTypes lightType)
{
	lights[lightType].emplace(
		lightName,
		LightSim{
			static_cast<LightSim::LightTypes>(lightType),
			camera->GetPositionVectorAddr(),
			glm::vec3(1.0f, 1.0f, 1.0f),
			camera->GetFrontVectorAddr()
		}
	);
}

void EverettEngine::CreateSound(const std::string& path, const std::string& soundName)
{
	sounds.emplace(
		soundName,
		SoundSim{
			path,
			glm::vec3(camera->GetPositionVectorAddr())
		}
	);
	sounds[soundName].Play();
}

void EverettEngine::LightUpdater()
{
	mainLGL->SetShaderUniformValue("proj", camera->GetProjectionMatrixAddr());
	mainLGL->SetShaderUniformValue("view", camera->GetViewMatrixAddr());

	mainLGL->SetShaderUniformValue("dirLightAmount",   static_cast<int>(lights[LightTypes::Direction].size()));
	mainLGL->SetShaderUniformValue("pointLightAmount", static_cast<int>(lights[LightTypes::Point].size()));
	mainLGL->SetShaderUniformValue("spotLightAmount",  static_cast<int>(lights[LightTypes::Spot].size()));
	mainLGL->SetShaderUniformValue("ambient", glm::vec3(0.4f, 0.4f, 0.4f));
	
	int index = 0;
	for (auto& light : lights[LightTypes::Point])
	{
		LightSim::Attenuation atten = light.second.GetAttenuation();

		LGLUtils::SetShaderUniformArrayAt(
			*mainLGL,
			lightShaderValueNames[1].first,
			index++,
			lightShaderValueNames[1].second,
			light.second.GetPositionVectorAddr(), glm::vec3(0.4f, 0.4f, 0.4f),
			glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, atten.linear,
			atten.quadratic
		);
	}

	index = 0;
	for (auto& light : lights[LightTypes::Spot])
	{
		LightSim::Attenuation atten = light.second.GetAttenuation();

		LGLUtils::SetShaderUniformArrayAt(
			*mainLGL,
			lightShaderValueNames[2].first,
			index++,
			lightShaderValueNames[2].second,
			light.second.GetPositionVectorAddr(), light.second.GetFrontVectorAddr(),
			glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f,
			atten.linear, atten.quadratic, glm::cos(glm::radians(12.5f)),
			glm::cos(glm::radians(17.5f))
		);
	}


	mainLGL->SetShaderUniformValue("viewPos", camera->GetPositionVectorAddr());
}

void EverettEngine::SetScriptToObject(
	ObjectTypes objectType,
	const std::string& subtypeName,
	const std::string& objectName,
	const std::string& dllPath,
	const std::string& dllName
)
{
	if (fileLoader)
	{
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncWeakPtr;

		fileLoader->GetScriptFuncFromDLL(
			dllPath,
			dllName,
			objectType == ObjectTypes::Camera ? "Camera" : objectName,
			scriptFuncWeakPtr
		);

		SolidSim* object = GetObjectFromMap(objectType, subtypeName, objectName);

		if (object)
		{
			object->AddScriptFunc(dllName, scriptFuncWeakPtr);
			object->ExecuteScriptFunc();
		}
	}
}

void EverettEngine::UnsetScriptFromObject(const std::string& dllPath)
{
	if(fileLoader)
	{ 
		fileLoader->UnloadScriptDLL(dllPath);
	}
}

bool EverettEngine::IsObjectScriptSet(
	ObjectTypes objectType,
	const std::string& subtypeName,
	const std::string& objectName,
	const std::string& dllName
)
{
	return GetObjectFromMap(objectType, subtypeName, objectName)->IsScriptFuncAdded(dllName);
}

void EverettEngine::SetScriptToKey(
	const std::string& keyName,
	const std::string& dllPath,
	const std::string& dllName
)
{
	if (fileLoader && !keyName.empty())
	{
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncPress;
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncRelease;

		fileLoader->GetScriptFuncFromDLL(dllPath, dllName, "Key" + keyName + "Pressed", scriptFuncPress);
		fileLoader->GetScriptFuncFromDLL(dllPath, dllName, "Key" + keyName + "Released", scriptFuncRelease);

		bool addNewKey = false;

		if (keyScriptFuncMap.find(keyName) == keyScriptFuncMap.end())
		{
			keyScriptFuncMap.emplace(keyName, std::pair<ScriptFuncStorage, ScriptFuncStorage>{});
			addNewKey = true;
		}

		keyScriptFuncMap[keyName].first.AddScriptFunc(dllName, scriptFuncPress);
		if (scriptFuncRelease.lock())
		{
			keyScriptFuncMap[keyName].second.AddScriptFunc(dllName, scriptFuncRelease);
		}

		if (addNewKey)
		{
			std::function<void()> scriptFuncPressWrapper = [this, keyName]() { keyScriptFuncMap[keyName].first.ExecuteAllScriptFuncs(nullptr); };
			std::function<void()> scriptFuncReleaseWrapper = nullptr;

			if (scriptFuncRelease.lock())
			{
				scriptFuncReleaseWrapper = [this, keyName]() { keyScriptFuncMap[keyName].second.ExecuteAllScriptFuncs(nullptr); };
			}

			mainLGL->SetInteractable(
				LGL::ConvertKeyTo(keyName),
				scriptFuncPressWrapper,
				scriptFuncReleaseWrapper
			);
		}
	}
}

void EverettEngine::UnsetScriptFromKey(const std::string& objectName)
{
	//TODO
}

bool EverettEngine::IsKeyScriptSet(
	const std::string& keyName,
	const std::string& dllName
)
{
	return keyScriptFuncMap[keyName].first.IsScriptFuncAdded(dllName);
}

std::vector<glm::vec3> EverettEngine::GetObjectParamsByName(
	EverettEngine::ObjectTypes objectType,
	const std::string& subtypeName, 
	const std::string& objectName
)
{
	SolidSim* solid = GetObjectFromMap(objectType, subtypeName, objectName);

	if (solid)
	{
		return { solid->GetPositionVectorAddr(), solid->GetScaleVectorAddr(), solid->GetFrontVectorAddr() };
	}
	else
	{
		return { {} };
	}
}

void EverettEngine::SetObjectParamsByName(
	EverettEngine::ObjectTypes objectType,
	const std::string& modelName,
	const std::string& objectName,
	const std::vector<glm::vec3>& params
)
{
	SolidSim* solid = GetObjectFromMap(objectType, modelName, objectName);

	if (solid)
	{
		solid->GetPositionVectorAddr() = params[0];
		solid->GetScaleVectorAddr()    = params[1];
		solid->GetFrontVectorAddr()    = params[2];

		solid->ForceModelUpdate();
	}
}

std::vector<std::string> EverettEngine::GetObjectsInDirList(
	const std::string& path, 
	const std::vector<std::string>& fileTypes
)
{
	auto FilterNonModelFiles = [&fileTypes](std::vector<std::string>& files)
	{
		std::vector<std::string> modelFiles;

		for (auto& file : files)
		{
			for (auto& fileType : fileTypes)
			{
				if (file.find(fileType) != file.npos)
				{
					modelFiles.push_back(file);
					break;
				}
			}
		}

		return modelFiles;
	};

	std::vector<std::string> files;

	fileLoader->GetFilesInDir(files, path);

	return FilterNonModelFiles(files);
}

std::vector<std::string> EverettEngine::GetModelInDirList(const std::string& path)
{
	return GetObjectsInDirList(path, { ".glb", ".obj" });
}

std::vector<std::string> EverettEngine::GetSoundInDirList(const std::string& path)
{
	return GetObjectsInDirList(path, {".wav"});
}

SolidSim* EverettEngine::GetObjectFromMap(
	EverettEngine::ObjectTypes objectType, 
	const std::string& subtypeName, 
	const std::string& objectName
)
{
	SolidSim* object = nullptr;

	switch (objectType)
	{
	case EverettEngine::ObjectTypes::Camera:
		object = dynamic_cast<SolidSim*>(camera.get());
		break;
	case EverettEngine::ObjectTypes::Solid:
		object = &MSM[subtypeName].solids[objectName];
		break;
	case EverettEngine::ObjectTypes::Light:
		object = dynamic_cast<SolidSim*>(
			&lights[static_cast<EverettEngine::LightTypes>(LightSim::GetTypeToName(subtypeName))][objectName]
		);
		break;
	case EverettEngine::ObjectTypes::Sound:
		object = dynamic_cast<SolidSim*>(&sounds[objectName]);
		break;
	default:
		break;
	}

	return object;
}

template<typename Sim>
std::vector<std::string> EverettEngine::GetNameList(const std::map<std::string, Sim>& sims)
{
	std::vector<std::string> names;
	names.reserve(sims.size());

	for (auto& sim : sims)
	{
		names.push_back(sim.first);
	}

	return names;
}

std::vector<std::string> EverettEngine::GetCreatedModels()
{
	std::vector<std::string> createdModels;

	for (auto& model : MSM)
	{
		createdModels.push_back(model.first);
	}

	return createdModels;
}

std::vector<std::string> EverettEngine::GetAllObjectTypeNames()
{
	std::vector<std::string> objectNames;

	for (auto& objectNamePair : objectTypes)
	{
		objectNames.push_back(objectNamePair.second);
	}

	return objectNames;
}

std::string EverettEngine::GetObjectTypeToName(ObjectTypes objectType)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.first == objectType)
		{
			return objectNamePair.second;
		}
	}

	assert(false && "Nonexistent type");
}

EverettEngine::ObjectTypes EverettEngine::GetObjectTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.second == objectName)
		{
			return objectNamePair.first;
		}
	}

	assert(false && "Nonexistent name");
}

std::vector<std::string> EverettEngine::GetNamesByObject(ObjectTypes objType)
{
	switch (objType)
	{
	case ObjectTypes::Solid:
		return GetSolidList();
	case ObjectTypes::Light:
		return GetLightList();
	case ObjectTypes::Sound:
		return GetSoundList();
	default:
		assert(false && "unreachable");
	}
}

std::vector<std::string> EverettEngine::GetSolidList()
{
	std::vector<std::string> solidNames;

	for (auto& model : MSM)
	{
		solidNames.push_back('.' + model.first);
		std::vector<std::string> modelSolidNames = GetNameList(model.second.solids);
		solidNames.insert(solidNames.end(), modelSolidNames.begin(), modelSolidNames.end());
	}

	return solidNames;
}

std::vector<std::string> EverettEngine::GetLightList()
{
	std::vector<std::string> lightNames;

	std::vector<std::string> lightTypes = LightSim::GetLightTypeNames();

	for (auto& light : lights)
	{
		lightNames.push_back('.' + lightTypes[static_cast<int>(light.first)]);
		std::vector<std::string> lightSolidNames = GetNameList(light.second);
		lightNames.insert(lightNames.end(), lightSolidNames.begin(), lightSolidNames.end());
	}

	return lightNames;
}

std::vector<std::string> EverettEngine::GetSoundList()
{
	return GetNameList(sounds);
}

std::vector<std::string> EverettEngine::GetLightTypeList()
{
	return LightSim::GetLightTypeNames();
}

void EverettEngine::ForceFocusOnWindow(const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->BringWindowOnTop(name);
	}
}

void EverettEngine::AddWindowHandler(HWND windowHandler, const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->AddWindowHandle(windowHandler, name);
	}
}

void EverettEngine::AddCurrentWindowHandler(const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->AddCurrentWindowHandle(name);
	}
}

EverettEngine::LastKeyPressPoll::LastKeyPressPoll()
{
	lastKeyPressedID = -1;
	isValidKeyPress = false;
}

void EverettEngine::LastKeyPressPoll::KeyPressCallback(int key, int scancode, int action, int mods)
{
	if (action)
	{
		lastKeyPressedID = key;
		isValidKeyPress = true;
		cv.notify_one();
	}
}

void EverettEngine::LastKeyPressPoll::WaitForKeyPress()
{
	std::unique_lock<std::mutex> lk(mux);
	cv.wait(lk, [this]() { return isValidKeyPress; });
}

int EverettEngine::LastKeyPressPoll::GetLastKeyPressedID()
{
	return lastKeyPressedID;
}