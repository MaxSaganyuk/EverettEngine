#include <iostream>
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

#include "CommonStrEdits.h"

#include "stdEx/mapEx.h"

#include "AnimSystem.h"

#define EVERETT_EXPORT
#include "EverettEngine.h"

#include "SolidToModelManager.h"

#include "ShaderGenerator.h"

//#define BONE_TEST

struct EverettEngine::ModelSolidInfo
{
	SolidToModelManager::FullModelInfo model;
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
	mainLGL->SetShaderFolder(fileLoader->GetCurrentDir() + debugShaderPath);
#endif
	camera = std::make_unique<CameraSim>(windowHeight, windowWidth);;
	camera->SetMode(CameraSim::Mode::Fly);
	camera->SetGhostMode(true);

	SoundSim::SetCamera(*camera);

	mainLGL->SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });

	cursorCaptureCallback =
		[this](double xpos, double ypos) { camera->Rotate(static_cast<float>(xpos), static_cast<float>(ypos)); };
	mainLGL->SetCursorPositionCallback(cursorCaptureCallback);

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
	mainLGL->CaptureMouse(true);

	auto additionalFuncs = [this]() {
		std::vector<glm::mat4> finalTransforms(animSystem->GetTotalBoneAmount(), glm::mat4(1.0f));
		animSystem->ProcessAnimations(
			std::chrono::duration<double>(std::chrono::system_clock::now() - startTime).count(), finalTransforms
		);

		mainLGL->SetShaderUniformValue("Bones", finalTransforms);

		camera->SetPosition(CameraSim::Direction::Nowhere);
		camera->ExecuteAllScriptFuncs();

		LightUpdater();

		LGLUtils::SetShaderUniformStruct(
			*mainLGL,
			lightShaderValueNames[0].first,
			lightShaderValueNames[0].second,
			0,
			1,
			0.5f
		);
	};

#ifdef BONE_TEST
	defaultShaderProgram = "boneTest";

	static bool j = true;
	mainLGL->SetInteractable(LGL::ConvertKeyTo("Y"), [this]()
		{
			static int i = 0;
			if (j)
			{
				j = false;
				mainLGL->SetShaderUniformValue("boneIDChoice", ++i);
			}
		},
		[this]()
		{
			j = true;
		}
	);

	mainLGL->SetShaderUniformValue("boneIDChoice", 0);
#else
	defaultShaderProgram = "lightCombAndBone";
#endif
	mainLGLRenderThread = std::make_unique<std::thread>(
		[this, additionalFuncs]() { mainLGL->RunRenderingCycle(additionalFuncs); }
	);

	startTime = std::chrono::system_clock::now();

	animSystem = std::make_unique<AnimSystem>();
}

int EverettEngine::PollForLastKeyPressed()
{
	LastKeyPressPoll lastKeyPressPoll;

	mainLGL->SetKeyPressCallback(
		[&lastKeyPressPoll](int key, int scancode, int action, int mods) { 
			lastKeyPressPoll.KeyPressCallback(key, scancode, action, mods); 
		}
	);
	mainLGL->SetCursorPositionCallback(nullptr);
	
	lastKeyPressPoll.WaitForKeyPress();

	mainLGL->SetKeyPressCallback(nullptr);
	mainLGL->SetCursorPositionCallback(cursorCaptureCallback);

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
	auto resPair = MSM.emplace(name, std::move(ModelSolidInfo{}));
	
	LGLStructs::ModelInfo& newModel = MSM[name].model.first;
	AnimSystem::ModelAnim& newModelAnim = MSM[name].model.second;

	if (!fileLoader->LoadModel(path, name, newModel, newModelAnim))
	{
		MSM.erase(name);
		return false;
	}

	CheckAndAddToNameTracker((*resPair.first).first);
	animSystem->AddModelAnim(newModelAnim);

	newModel.shaderProgram = defaultShaderProgram;
	newModel.render = false;

	ShaderGenerator shaderGen;
#ifdef _DEBUG
	std::string filePath = fileLoader->GetCurrentDir() + debugShaderPath + '\\' + defaultShaderProgram;

	shaderGen.LoadPreSources(filePath);
	shaderGen.SetValueToDefine("BONE_AMOUNT", animSystem->GetTotalBoneAmount());
	shaderGen.GenerateShaderFiles(filePath);
#else
#error Shader file generation is not implemented fo release configuration
#endif

	newModel.modelBehaviour = [this, name]()
	{
		// Existence of the lambda implies existence of the model
		auto& model = MSM[name];

		mainLGL->SetShaderUniformValue("textureless", static_cast<int>(model.model.first.isTextureless));
		mainLGL->SetShaderUniformValue("startingBoneIndex", static_cast<int>(model.model.second.startingBoneIndex));
	};

	newModel.generalMeshBehaviour = [this, name](int meshIndex)
	{
		// Existence of the lambda implies existence of the model
		auto& model = MSM[name];

		glm::mat4 emptyMatrix = glm::mat4();

		for (auto& solid : model.solids)
		{
			glm::mat4& modelMatrix = solid.second.GetModelMeshVisibility(meshIndex)
				? solid.second.GetModelMatrixAddr() : emptyMatrix;

			mainLGL->SetShaderUniformValue("model", modelMatrix);
			mainLGL->SetShaderUniformValue("inv", glm::inverse(modelMatrix));
		}
	};

	mainLGL->CreateModel(name, newModel);
	if (MSM.size() > 1)
	{
		mainLGL->RecompileShader(defaultShaderProgram);
	}

	fileLoader->FreeTextureData();

	return true;
}

bool EverettEngine::CreateSolid(const std::string& modelName, const std::string& solidName)
{
	SolidSim newSolid(camera->GetPositionVectorAddr() + camera->GetFrontVectorAddr());
	newSolid.SetBackwardsModelAccess(MSM[modelName].model);

	auto resPair = MSM[modelName].solids.emplace(solidName, std::move(newSolid));

	if (resPair.second)
	{
		MSM[modelName].model.first.render = true;

		CheckAndAddToNameTracker((*resPair.first).first);

		return true;
	}

	return false;
}

bool EverettEngine::CreateLight(const std::string& lightName, LightTypes lightType)
{
	auto resPair = lights[lightType].emplace(
		lightName,
		LightSim{
			static_cast<LightSim::LightTypes>(lightType),
			camera->GetPositionVectorAddr(),
			glm::vec3(1.0f, 1.0f, 1.0f),
			camera->GetFrontVectorAddr()
		}
	);

	if (resPair.second)
	{
		CheckAndAddToNameTracker((*resPair.first).first);

		return true;
	}

	return false;
}

bool EverettEngine::CreateSound(const std::string& path, const std::string& soundName)
{
	auto resPair = sounds.emplace(
		soundName,
		SoundSim{
			path,
			glm::vec3(camera->GetPositionVectorAddr())
		}
	);

	if (resPair.second)
	{
		sounds[soundName].Play();
		CheckAndAddToNameTracker((*resPair.first).first);

		return true;
	}

	return false;
}

EVERETT_API IObjectSim* EverettEngine::GetObjectInterface(
	ObjectTypes objectType,
	const std::string& subtypeName,
	const std::string& objectName
)
{
	return dynamic_cast<IObjectSim*>(GetObjectFromMap(objectType, subtypeName, objectName));
}

EVERETT_API ISolidSim* EverettEngine::GetSolidInterface(
	const std::string& modelName,
	const std::string& solidName
)
{
	return dynamic_cast<ISolidSim*>(GetObjectFromMap(ObjectTypes::Solid, modelName, solidName));
}

EVERETT_API ILightSim* EverettEngine::GetLightInterface(
	const std::string& lightTypeName,
	const std::string& lightName
)
{
	return dynamic_cast<ILightSim*>(GetObjectFromMap(ObjectTypes::Light, lightTypeName, lightName));
}

EVERETT_API ISoundSim* EverettEngine::GetSoundInterface(
	const std::string& soundName
)
{
	return dynamic_cast<ISoundSim*>(GetObjectFromMap(ObjectTypes::Sound, "", soundName));
}

EVERETT_API ICameraSim* EverettEngine::GetCameraInterface()
{
	return dynamic_cast<ICameraSim*>(GetObjectFromMap(ObjectTypes::Camera, "", ""));
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
			objectType == ObjectTypes::Camera ? "Camera" : objectName,
			scriptFuncWeakPtr
		);

		ObjectSim* object = GetObjectFromMap(objectType, subtypeName, objectName);

		if (object && scriptFuncWeakPtr.lock())
		{
			object->AddScriptFunc(dllName, scriptFuncWeakPtr);
		}

		if (object->IsScriptFuncAdded(dllName))
		{
			object->ExecuteScriptFunc(dllName);
		}
	}
}

void EverettEngine::UnsetScript(const std::string& dllPath)
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
	return GetObjectFromMap(objectType, subtypeName, objectName)->IsScriptFuncRunnable(dllName);
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

		// Using bitwise or to avoid short-circuiting second call
		bool anyFuncAdded = 
			fileLoader->GetScriptFuncFromDLL(dllPath, "Key" + keyName + "Pressed", scriptFuncPress) | 
			fileLoader->GetScriptFuncFromDLL(dllPath, "Key" + keyName + "Released", scriptFuncRelease);

		if (anyFuncAdded)
		{
			bool addNewKey = false;

			if (keyScriptFuncMap.find(keyName) == keyScriptFuncMap.end())
			{
				keyScriptFuncMap.emplace(keyName, std::pair<ScriptFuncStorage, ScriptFuncStorage>{});
				addNewKey = true;
			}

			if (scriptFuncPress.lock() && !keyScriptFuncMap[keyName].first.IsScriptFuncAdded(dllName))
			{
				keyScriptFuncMap[keyName].first.AddScriptFunc(dllName, scriptFuncPress);
			}

			if (scriptFuncRelease.lock() && !keyScriptFuncMap[keyName].second.IsScriptFuncAdded(dllName))
			{
				keyScriptFuncMap[keyName].second.AddScriptFunc(dllName, scriptFuncRelease);
			}

			if (addNewKey)
			{
				std::function<void()> scriptFuncPressWrapper = nullptr;
				std::function<void()> scriptFuncReleaseWrapper = nullptr;

				if (scriptFuncPress.lock())
				{
					scriptFuncPressWrapper = [this, keyName]() { keyScriptFuncMap[keyName].first.ExecuteAllScriptFuncs(nullptr); };
				}

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
}

bool EverettEngine::IsKeyScriptSet(
	const std::string& keyName,
	const std::string& dllName
)
{
	auto& currentKeyPair = keyScriptFuncMap[keyName];

	return currentKeyPair.first.IsScriptFuncRunnable(dllName) || currentKeyPair.second.IsScriptFuncRunnable(dllName);
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
	return GetObjectsInDirList(path, { ".glb", ".dae", ".fbx", ".obj" });
}

std::vector<std::string> EverettEngine::GetSoundInDirList(const std::string& path)
{
	return GetObjectsInDirList(path, {".wav"});
}

ObjectSim* EverettEngine::GetObjectFromMap(
	EverettEngine::ObjectTypes objectType, 
	const std::string& subtypeName, 
	const std::string& objectName
)
{
	ObjectSim* object = nullptr;

	switch (objectType)
	{
	case EverettEngine::ObjectTypes::Camera:
		object = dynamic_cast<ObjectSim*>(camera.get());
		break;
	case EverettEngine::ObjectTypes::Solid:
		object = dynamic_cast<ObjectSim*>(&MSM[subtypeName].solids[objectName]);
		break;
	case EverettEngine::ObjectTypes::Light:
		object = dynamic_cast<ObjectSim*>(
			&lights[static_cast<EverettEngine::LightTypes>(LightSim::GetTypeToName(subtypeName))][objectName]
		);
		break;
	case EverettEngine::ObjectTypes::Sound:
		object = dynamic_cast<ObjectSim*>(&sounds[objectName]);
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
		mainLGL->CaptureMouse("LGL" != name);
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

void EverettEngine::CheckAndAddToNameTracker(const std::string& name)
{
	std::string namePure = CommonStrEdits::RemoveDigitsFromStringEnd(name);

	if (allNameTracker.find(&namePure) != allNameTracker.end())
	{
		++allNameTracker[&namePure];
	}
	else
	{
		allNameTracker.emplace(&name, 1);
	}
}

std::string EverettEngine::GetAvailableObjectName(const std::string& name)
{
	if (allNameTracker.find(&name) != allNameTracker.end())
	{
		return (name + std::to_string(allNameTracker[&name]));
	}

	return name;
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