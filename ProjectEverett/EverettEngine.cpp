#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "LGL.h"
#include "LGLUtils.h"

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

using ObjectInfoNames = SimSerializer::ObjectInfoNames;

struct EverettEngine::ObjectTypeInfo
{
	ObjectTypes nameEnum;
	std::string nameStr;
	std::type_index pureType;
};

struct EverettEngine::ModelSolidInfo
{
	std::string modelPath;
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

std::vector<EverettEngine::ObjectTypeInfo> EverettEngine::objectTypes
{
	{EverettEngine::ObjectTypes::Camera, CameraSim::GetObjectTypeNameStr(), typeid(CameraSim)},
	{EverettEngine::ObjectTypes::Solid,  SolidSim::GetObjectTypeNameStr(),  typeid(SolidSim)},
	{EverettEngine::ObjectTypes::Light,  LightSim::GetObjectTypeNameStr(),  typeid(LightSim)},
	{EverettEngine::ObjectTypes::Sound,  SoundSim::GetObjectTypeNameStr(),  typeid(SoundSim)}
};

std::vector<std::string> EverettEngine::lightTypes = LightSim::GetLightTypeNames();

struct EverettEngine::KeyScriptFuncInfo
{
	bool holdable;
	ScriptFuncStorage pressedFuncs;
	ScriptFuncStorage releasedFuncs;
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
	mainLGL->SetShaderFolder(FileLoader::GetCurrentDir() + debugShaderPath);
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
			true,
			[this, i]() { camera->SetPosition(static_cast<CameraSim::Direction>(i)); }
		);
	}
	mainLGL->SetInteractable('R', true, [this]() { camera->SetPosition(CameraSim::Direction::Up); });

	mainLGL->GetMaxAmountOfVertexAttr();
	mainLGL->CaptureMouse(true);

	auto additionalFuncs = [this]() {
		std::lock_guard lock(lglMutex);
		std::vector<glm::mat4>& finalTransforms = animSystem->GetFinalTransforms();

		if (!finalTransforms.empty())
		{
			mainLGL->SetShaderUniformValue("Bones", finalTransforms);
		}
		animSystem->ResetFinalTransforms();

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
	if (MSM.find(name) != MSM.end())
	{
		return true;
	}

	auto resPair = MSM.emplace(name, std::move(ModelSolidInfo{}));
	
	LGLStructs::ModelInfo& newModel = MSM[name].model.first;
	AnimSystem::ModelAnim& newModelAnim = MSM[name].model.second;
	MSM[name].modelPath = path;

	if (!fileLoader->modelLoader.LoadModel(path, name, newModel, newModelAnim))
	{
		MSM.erase(name);
		return false;
	}

	CheckAndAddToNameTracker(resPair.first->first);

	newModel.shaderProgram = defaultShaderProgram;
	newModel.render = false;

	newModel.modelBehaviour = [this, name]()
	{
		// Existence of the lambda implies existence of the model
		auto& model = MSM[name];
		auto& [modelPath, modelInfo, solidInfo] = model;

		bool animationless = model.model.second.animInfoVect.empty();
		mainLGL->SetShaderUniformValue("textureless", static_cast<int>(modelInfo.first.isTextureless));
		mainLGL->SetShaderUniformValue("animationless", static_cast<int>(animationless));

		if (!animationless)
		{
			for (auto& [solidName, solid] : solidInfo)
			{
				if (solid.IsModelAnimationPlaying())
				{
					animSystem->ProcessAnimations(
						modelInfo.second,
						solid.GetModelCurrentAnimationTime(),
						solid.GetModelAnimation(),
						solid.GetModelCurrentStartingBoneIndex()
					);
				}
			}
		}
	};

	newModel.generalMeshBehaviour = [this, name, modelReset = false](int meshIndex) mutable
	{
		// Existence of the lambda implies existence of the model
		auto& model = MSM[name];

		glm::mat4 emptyMatrix = glm::mat4();

		if (!model.solids.empty())
		{
			modelReset = false;

			for (auto& [solidName, solid] : model.solids)
			{
				glm::mat4& modelMatrix = solid.GetModelMeshVisibility(meshIndex)
					? solid.GetModelMatrixAddr() : emptyMatrix;

				mainLGL->SetShaderUniformValue("model", modelMatrix);
				mainLGL->SetShaderUniformValue("inv", glm::inverse(modelMatrix));

				if (!model.model.second.animInfoVect.empty())
				{
					mainLGL->SetShaderUniformValue("startingBoneIndex", static_cast<int>(
						solid.GetModelCurrentStartingBoneIndex()
						)
					);
				}
			}
		}
		else if (!modelReset)
		{
			modelReset = true;

			mainLGL->SetShaderUniformValue("model", emptyMatrix);
			mainLGL->SetShaderUniformValue("inv", emptyMatrix);
		}
	};

	mainLGL->CreateModel(name, newModel);

	fileLoader->modelLoader.FreeTextureData();

	return true;
}

bool EverettEngine::CreateSolid(const std::string& modelName, const std::string& solidName)
{
	if (MSM[modelName].solids.find(solidName) != MSM[modelName].solids.end())
	{
		return true;
	}

	SolidSim newSolid(camera->GetPositionVectorAddr() + camera->GetFrontVectorAddr());
	newSolid.SetBackwardsModelAccess(MSM[modelName].model);

	for (auto& animInfo : MSM[modelName].model.second.animInfoVect)
	{
		newSolid.AppendModelStartingBoneIndex(animSystem->GetTotalBoneAmount());
		animSystem->IncrementTotalBoneAmount(MSM[modelName].model.second);
	}

	ShaderGenerator shaderGen;
	size_t totalBoneAmount = animSystem->GetTotalBoneAmount();

#ifdef _DEBUG
	std::string filePath = fileLoader->GetCurrentDir() + debugShaderPath + '\\' + defaultShaderProgram;

	shaderGen.LoadPreSources(filePath);
	if (totalBoneAmount)
	{
		shaderGen.SetValueToDefine("BONE_AMOUNT", totalBoneAmount);
	}
	shaderGen.GenerateShaderFiles(filePath);

	mainLGL->RecompileShader(defaultShaderProgram);
#else
#error Shader file generation is not implemented fo release configuration
#endif


	auto resPair = MSM[modelName].solids.emplace(solidName, std::move(newSolid));

	if (resPair.second)
	{
		MSM[modelName].model.first.render = true;

		CheckAndAddToNameTracker(resPair.first->first);

		return true;
	}

	return false;
}

bool EverettEngine::CreateLight(const std::string& lightName, LightTypes lightType)
{
	if (lights[lightType].find(lightName) != lights[lightType].end())
	{
		return true;
	}

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
		CheckAndAddToNameTracker(resPair.first->first);

		return true;
	}

	return false;
}

bool EverettEngine::CreateSound(const std::string& path, const std::string& soundName)
{
	if (sounds.find(soundName) != sounds.end())
	{
		return true;
	}

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
		CheckAndAddToNameTracker(resPair.first->first);

		return true;
	}

	return false;
}

bool EverettEngine::DeleteModel(const std::string& modelName)
{
	bool res = false;

	mainLGL->PauseRendering();

	auto iter = MSM.find(modelName);

	if (iter != MSM.end())
	{
		mainLGL->DeleteModel(modelName);
		
		for (auto currentSolidIter = iter->second.solids.begin(); 
			 currentSolidIter != iter->second.solids.end(); 
			 ++currentSolidIter
		)
		{
			allNameTracker.erase(&currentSolidIter->first);
		}

		allNameTracker.erase(&iter->first);
		MSM.erase(modelName);

		res = true;
	}
	
	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteSolid(const std::string& solidName)
{
	bool res = false;

	mainLGL->PauseRendering();

	for (auto& [modelName, modelInfo] : MSM)
	{
		auto iter = modelInfo.solids.find(solidName);

		if (iter != modelInfo.solids.end())
		{
			allNameTracker.erase(&iter->first);
			modelInfo.solids.erase(solidName);

			res = true;
		}
	}

	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteLight(const std::string& lightName)
{
	bool res = false;

	mainLGL->PauseRendering();

	for (auto& [lightType, lightCollection] : lights)
	{
		auto iter = lightCollection.find(lightName);

		if (iter != lightCollection.end())
		{
			allNameTracker.erase(&(*iter).first);
			lightCollection.erase(lightName);

			res = true;
		}
	}

	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteSound(const std::string& soundName)
{
	bool res = false;

	mainLGL->PauseRendering();

	auto iter = sounds.find(soundName);

	if (iter != sounds.end())
	{
		allNameTracker.erase(&(*iter).first);
		sounds.erase(soundName);

		res = true;
	}

	mainLGL->PauseRendering(false);

	return res;
}

IObjectSim* EverettEngine::GetObjectInterface(
	ObjectTypes objectType,
	const std::string& subtypeName,
	const std::string& objectName
)
{
	return dynamic_cast<IObjectSim*>(GetObjectFromMap(objectType, subtypeName, objectName));
}

ISolidSim* EverettEngine::GetSolidInterface(
	const std::string& modelName,
	const std::string& solidName
)
{
	return dynamic_cast<ISolidSim*>(GetObjectFromMap(ObjectTypes::Solid, modelName, solidName));
}

ILightSim* EverettEngine::GetLightInterface(
	const std::string& lightTypeName,
	const std::string& lightName
)
{
	return dynamic_cast<ILightSim*>(GetObjectFromMap(ObjectTypes::Light, lightTypeName, lightName));
}

ISoundSim* EverettEngine::GetSoundInterface(
	const std::string& soundName
)
{
	return dynamic_cast<ISoundSim*>(GetObjectFromMap(ObjectTypes::Sound, "", soundName));
}

ICameraSim* EverettEngine::GetCameraInterface()
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
	for (auto& [lightName, light] : lights[LightTypes::Point])
	{
		LightSim::Attenuation atten = light.GetAttenuation();

		LGLUtils::SetShaderUniformArrayAt(
			*mainLGL,
			lightShaderValueNames[1].first,
			index++,
			lightShaderValueNames[1].second,
			light.GetPositionVectorAddr(), glm::vec3(0.4f, 0.4f, 0.4f),
			glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, atten.linear,
			atten.quadratic
		);
	}

	index = 0;
	for (auto& [lightName, light] : lights[LightTypes::Spot])
	{
		LightSim::Attenuation atten = light.GetAttenuation();

		LGLUtils::SetShaderUniformArrayAt(
			*mainLGL,
			lightShaderValueNames[2].first,
			index++,
			lightShaderValueNames[2].second,
			light.GetPositionVectorAddr(), light.GetFrontVectorAddr(),
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
	SetScriptToObjectImpl(
		GetObjectFromMap(objectType, subtypeName, objectName), 
		objectType == ObjectTypes::Camera ? "Camera" : objectName, 
		dllPath, 
		dllName
	);
}

void EverettEngine::SetScriptToObjectImpl(
	ObjectSim* object,
	const std::string& objectName,
	const std::string& dllPath,
	const std::string& dllName
)
{
	if (fileLoader && object)
	{
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncWeakPtr;

		fileLoader->dllloader.GetScriptFuncFromDLL(
			dllPath,
			objectName,
			scriptFuncWeakPtr
		);

		if (scriptFuncWeakPtr.lock())
		{
			object->AddScriptFunc(dllPath, dllName, scriptFuncWeakPtr);
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
		fileLoader->dllloader.UnloadScriptDLL(dllPath);
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
	bool holdable,
	const std::string& dllPath,
	const std::string& dllName
)
{
	if (fileLoader && !keyName.empty())
	{
		bool isPressedExist = false;
		bool isReleasedExist = false;
		bool addNewKey = keyScriptFuncMap.find(keyName) == keyScriptFuncMap.end();

		if (!addNewKey)
		{
			isPressedExist = keyScriptFuncMap[keyName].pressedFuncs.IsScriptFuncAdded(dllName);
			isReleasedExist = keyScriptFuncMap[keyName].releasedFuncs.IsScriptFuncAdded(dllName);
		}

		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncPress;
		std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc> scriptFuncRelease;

		// Using bitwise or to avoid short-circuiting second call
		bool anyFuncAdded = 
			fileLoader->dllloader.GetScriptFuncFromDLL(dllPath, "Key" + keyName + "Pressed", scriptFuncPress) | 
			fileLoader->dllloader.GetScriptFuncFromDLL(dllPath, "Key" + keyName + "Released", scriptFuncRelease);

		if (anyFuncAdded)
		{
			if (addNewKey)
			{
				keyScriptFuncMap.emplace(keyName, KeyScriptFuncInfo{});
			}

			if (scriptFuncPress.lock() && !isPressedExist)
			{
				keyScriptFuncMap[keyName].pressedFuncs.AddScriptFunc(dllPath, dllName, scriptFuncPress);
			}

			if (scriptFuncRelease.lock() && !isReleasedExist)
			{
				keyScriptFuncMap[keyName].releasedFuncs.AddScriptFunc(dllPath, dllName, scriptFuncRelease);
			}

			if (addNewKey)
			{
				std::function<void()> scriptFuncPressWrapper = nullptr;
				std::function<void()> scriptFuncReleaseWrapper = nullptr;

				if (scriptFuncPress.lock())
				{
					scriptFuncPressWrapper = [this, keyName]() { keyScriptFuncMap[keyName].pressedFuncs.ExecuteAllScriptFuncs(nullptr); };
				}

				if (scriptFuncRelease.lock())
				{
					scriptFuncReleaseWrapper = [this, keyName]() { keyScriptFuncMap[keyName].releasedFuncs.ExecuteAllScriptFuncs(nullptr); };
				}

				keyScriptFuncMap[keyName].holdable = holdable;

				mainLGL->SetInteractable(
					LGL::ConvertKeyTo(keyName),
					holdable,
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
	auto& [holdable, pressedFunc, releasedFunc] = keyScriptFuncMap[keyName];

	return pressedFunc.IsScriptFuncRunnable(dllName) || releasedFunc.IsScriptFuncRunnable(dllName);
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

std::string EverettEngine::GetSaveFileType()
{
	return saveFileType;
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
void EverettEngine::SaveObjectsToFile(std::fstream& file)
{
	// Shame there's no constexpr switch
	if constexpr (std::is_same_v<Sim, CameraSim>)
	{
		file << camera->GetSimInfoToSave("");
	}
	else if constexpr (std::is_same_v<Sim, SolidSim>)
	{
		for (auto& [modelName, model] : MSM)
		{
			for (auto& [solidName, solid] : model.solids)
			{
				file << solid.GetSimInfoToSave(modelName + '*' + solidName + '*' + model.modelPath);
			}
		}
	}
	else if constexpr (std::is_same_v<Sim, LightSim>)
	{
		for (auto& [lightType, lightInfo] : lights)
		{
			for (auto& [lightName, light] : lightInfo)
			{
				file << light.GetSimInfoToSave(light.GetCurrentLightType() + '*' + lightName);
			}
		}
	}
	else if constexpr (std::is_same_v<Sim, SoundSim>)
	{
		for (auto& [soundName, sound] : sounds)
		{
			file << sound.GetSimInfoToSave(soundName);
		}
	}
	else
	{
		static_assert(true && "Unacceptable type");
	}
}

void EverettEngine::ResetEngine()
{
	MSM.clear();
	lights.clear();
	sounds.clear();
	keyScriptFuncMap.clear();
	allNameTracker.clear();

	if (fileLoader)
	{
		fileLoader->dllloader.FreeDllData();
	}

	mainLGL->ResetLGL();
}

bool EverettEngine::SaveDataToFile(const std::string& filePath)
{
	std::string realFilePath = filePath + saveFileType;
	std::fstream file(realFilePath, std::ios::out);

	file << SimSerializer::GetSerializerVersion() + '\n';

	SaveObjectsToFile<CameraSim>(file);
	SaveObjectsToFile<SolidSim>(file);
	SaveObjectsToFile<LightSim>(file);
	SaveObjectsToFile<SoundSim>(file);

	for (auto& keybindPair : keyScriptFuncMap)
	{
		std::string keybindBegin = "Keybind*" + keybindPair.first + '*' + std::to_string(keybindPair.second.holdable) + '*';

		file <<
			keybindBegin +
			"Pressed*" +
			SimSerializer::GetValueToSaveFrom(keybindPair.second.pressedFuncs.GetAddedScriptDLLs())
			+ '\n';

		file <<
			keybindBegin +
			"Released*" +
			SimSerializer::GetValueToSaveFrom(keybindPair.second.releasedFuncs.GetAddedScriptDLLs())
			+ '\n';
	}

	file.close();

	return true;
}

void EverettEngine::LoadCameraFromLine(std::string_view& line)
{
	camera->SetSimInfoToLoad(line);
	camera->ForceModelUpdate();
}

template<typename Sim>
void EverettEngine::ApplySimInfoFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo, bool& res)
{
	Sim* createdObject = dynamic_cast<Sim*>(GetObjectFromMap(
		GetObjectPureTypeToName(typeid(Sim)),
		objectInfo[ObjectInfoNames::SubtypeName],
		objectInfo[ObjectInfoNames::ObjectName])
		);

	if (createdObject)
	{
		res = createdObject->SetSimInfoToLoad(line);

		if (!res) return;

		if constexpr (std::is_same_v<Sim, SolidSim>) 
		{
			createdObject->ForceModelUpdate();
		}

		std::vector<std::pair<std::string, std::string>> objectScriptDllInfo = createdObject->GetTempScriptDLLInfo();
		for (auto& dllNamePair : objectScriptDllInfo)
		{
			SetScriptToObjectImpl(createdObject, objectInfo[ObjectInfoNames::ObjectName], dllNamePair.second, dllNamePair.first);
		}

		res = true;
	}
}

void EverettEngine::LoadSolidFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;

	if (CreateModel(objectInfo[ObjectInfoNames::Path], objectInfo[ObjectInfoNames::SubtypeName]) &&
		CreateSolid(objectInfo[ObjectInfoNames::SubtypeName], objectInfo[ObjectInfoNames::ObjectName]))
	{
		ApplySimInfoFromLine<SolidSim>(line, objectInfo, res);
	}

	assert(res && "Solid creation from file failed");
}

void EverettEngine::LoadLightFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;

	if (CreateLight(
		objectInfo[ObjectInfoNames::ObjectName],
		static_cast<LightTypes>(LightSim::GetTypeToName(objectInfo[ObjectInfoNames::SubtypeName])))
		)
	{
		ApplySimInfoFromLine<LightSim>(line, objectInfo, res);
	}

	assert(res && "Light creation from file failed");
}

void EverettEngine::LoadSoundFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;

	if (CreateSound(objectInfo[ObjectInfoNames::Path], objectInfo[ObjectInfoNames::ObjectName]))
	{
		ApplySimInfoFromLine<SoundSim>(line, objectInfo, res);
	}

	assert(res && "Sound creation from file failed");
}

void EverettEngine::LoadKeybindsFromLine(std::string_view& line)
{
	size_t objectInfoAmount = std::count(line.begin(), line.end(), '*');
	assert(objectInfoAmount == 4 && "Invalid keybind info amount during world load");

	line.remove_prefix(line.find('*') + 1);
	std::string_view keyname = line.substr(0, line.find('*'));
	line.remove_prefix(line.find('*') + 1);

	bool holdable = std::stoi(std::string(line.substr(0, line.find('*'))));
	line.remove_prefix(line.find('*') + 1);
	line.remove_prefix(line.find('*') + 1);

	std::vector<std::pair<std::string, std::string>> dllInfo;
	SimSerializer::SetValueToLoadFrom(line, dllInfo);

	for (auto& dllPair : dllInfo)
	{
		SetScriptToKey(std::string(keyname), holdable, dllPair.second, dllPair.first);
	}
}

bool EverettEngine::LoadDataFromFile(const std::string& filePath)
{
	std::fstream file(filePath, std::ios::in);
	std::string lineLoader;
	std::string_view line;
	std::array<std::string, ObjectInfoNames::_SIZE> objectInfo{};

	std::getline(file, lineLoader);
	line = lineLoader;
	if (!SimSerializer::CheckSerializerVersion(line))
	{
		assert(false && "Invalid savefile version");
		return false;
	}

	mainLGL->PauseRendering();
	ResetEngine();
	while (!file.eof())
	{
		std::getline(file, lineLoader);
		line = lineLoader;

		if (line.substr(0, line.find('*')) == "Keybind")
		{
			LoadKeybindsFromLine(line);
		}
		else if (!line.empty())
		{
			SimSerializer::GetObjectInfo(line, objectInfo);

			switch (GetObjectTypeToName(objectInfo[ObjectInfoNames::ObjectType]))
			{
			case EverettEngine::ObjectTypes::Camera:
				LoadCameraFromLine(line);
				break;
			case EverettEngine::ObjectTypes::Solid:
				LoadSolidFromLine(line, objectInfo);
				break;
			case EverettEngine::ObjectTypes::Light:
				LoadLightFromLine(line, objectInfo);
				break;
			case EverettEngine::ObjectTypes::Sound:
				LoadSoundFromLine(line, objectInfo);
				break;
			default:
				assert(false && "unreachable");
			}
		}
	}
	mainLGL->PauseRendering(false);

	return true;
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
		objectNames.push_back(objectNamePair.nameStr);
	}

	return objectNames;
}
std::string EverettEngine::GetObjectTypeToName(ObjectTypes objectType)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameEnum == objectType)
		{
			return objectNamePair.nameStr;
		}
	}

	assert(false && "Nonexistent type");
}

EverettEngine::ObjectTypes EverettEngine::GetObjectTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameStr == objectName)
		{
			return objectNamePair.nameEnum;
		}
	}

	assert(false && "Nonexistent name");
}

std::type_index EverettEngine::GetObjectPureTypeToName(ObjectTypes objectType)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameEnum == objectType)
		{
			return objectNamePair.pureType;
		}
	}

	assert(false && "Nonexistent name");
}

std::type_index EverettEngine::GetObjectPureTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameStr == objectName)
		{
			return objectNamePair.pureType;
		}
	}

	assert(false && "Nonexistent name");
}

EverettEngine::ObjectTypes EverettEngine::GetObjectPureTypeToName(std::type_index pureType)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.pureType == pureType)
		{
			return objectNamePair.nameEnum;
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
	int number;
	std::string namePure = CommonStrEdits::RemoveDigitsFromStringEnd(name, number);

	if (allNameTracker.find(&namePure) != allNameTracker.end())
	{
		int& currentNumber = allNameTracker[&namePure];

		if (number > currentNumber)
		{
			currentNumber = number;
		}

		++currentNumber;
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
