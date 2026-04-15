#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <format>
#include <ranges>

#include "LGL.h"
#include "LGLUtils.h"

#include "MaterialSim.h"
#include "LightSim.h"
#include "SolidSim.h"
#include "CameraSim.h"
#include "SoundSim.h"
#include "ColliderSim.h"

#include "FileLoader.h"

#include "CommandHandler.h"

#include "WindowHandleHolder.h"
#include "ScriptFuncStorage.h"

#include "CommonStrEdits.h"

#include "stdEx/mapEx.h"
#include "stdEx/rangesEx.h"

#include "AnimSystem.h"

#include "CustomOutput.h"
#include "RenderLogger.h"

#define EVERETT_EXPORT
#include "EverettEngine.h"
#include "EverettException.h"

#include "SolidToModelManager.h"
#include "ModelInfo.h"

#include "ShaderGenerator.h"
#include "ConceptUtils.h"

using namespace EverettStructs;

//#define BONE_TEST

// OPTIMIZATION_TEST - to view uncapped FPS, UNOPTIMIZED_TEST - to compare to unoptimal logic
//#define OPTIMIZATION_TEST
//#define UNOPTIMIZED_TEST

#define ENABLE_OPTIMIZATIONS true

#ifdef OPTIMIZATION_TEST
#define ENABLE_VSYNC false
#ifdef UNOPTIMIZED_TEST
#define ENABLE_OPTIMIZATIONS false
#endif
#else
#define ENABLE_VSYNC true
#endif // OPTIMIZATION_TEST


using ObjectInfoNames = SimSerializer::ObjectInfoNames;

struct EverettEngine::ObjectTypeInfo
{
	ObjectTypes nameEnum;
	std::string nameStr;
	std::string interfaceNameStr;
	std::type_index pureType;
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

#define ToStr(str) #str

std::vector<EverettEngine::ObjectTypeInfo> EverettEngine::objectTypes
{
	{EverettEngine::ObjectTypes::Camera,   CameraSim::GetObjectTypeNameStr(), ToStr(ICameraSim), typeid(CameraSim)},
	{EverettEngine::ObjectTypes::Solid,    SolidSim::GetObjectTypeNameStr(), ToStr(ISolidSim), typeid(SolidSim)},
	{EverettEngine::ObjectTypes::Light,    LightSim::GetObjectTypeNameStr(), ToStr(ILightSim), typeid(LightSim)},
	{EverettEngine::ObjectTypes::Sound,    SoundSim::GetObjectTypeNameStr(), ToStr(ISolidSim), typeid(SoundSim)},
	{EverettEngine::ObjectTypes::Collider, ColliderSim::GetObjectTypeNameStr(), ToStr(IColliderSim), typeid(ColliderSim)}
};

#undef ToStr

std::vector<std::string> EverettEngine::lightTypes = LightSim::GetLightTypeNames();

struct EverettEngine::KeyScriptFuncInfo
{
	ScriptFuncStorage<void> pressedFuncs;
	ScriptFuncStorage<void> releasedFuncs;
};

EverettEngine::EverettEngine()
{
	logOutput = std::make_unique<CustomOutput>();
	errorOutput = std::make_unique<CustomOutput>();
	stdOutStreamBuffer = std::cout.rdbuf();
	stdErrStreamBuffer = std::cerr.rdbuf();

	SetLogCallback();
	
	LGL::InitOpenGL(3, 3);
	SoundSim::InitOpenAL();

	mainLGL    = std::make_unique<LGL>();
	fileLoader = std::make_unique<FileLoader>();
	animSystem = std::make_unique<AnimSystem>();
	cmdHandler = std::make_unique<CommandHandler>();
	hwndHolder = std::make_unique<WindowHandleHolder>();

	mouseScrollScriptFuncs = std::make_unique<ScriptFuncStorage<double>>();
		
	ObjectSim::InitializeObjectGraph();
}

EverettEngine::~EverettEngine()
{
	SetLogCallback(false);
	SoundSim::TerminateOpenAL();
	SetRenderLoggerCallbacks(false);
	LGL::TerminateOpenGL();
}

void EverettEngine::CreateAndSetupMainWindow(
	int windowWidth, 
	int windowHeight, 
	const std::string& title, 
	bool fullscreen,
	bool enableLogger
)
{
	mainLGL->CreateWindow(windowWidth, windowHeight, title, fullscreen);

	hwndHolder->AddCurrentWindowHandle("LGL");

	mainLGL->SetAssetOnOpenGLFailure(true);
	mainLGL->SetShaderFolder(FileLoader::GetCurrentDir() + '\\' + shaderPath);

	if (enableLogger)
	{
		fileLoader->fontLoader.LoadFontFromPath(fontPath + std::string("\\") + loggerFont, 16);

		generalRenderTextBehaviour = [this](glm::vec4&& colorToUse)
			{
				mainLGL->SetShaderUniformValue(
					"proj",
					glm::ortho(
						0.0f,
						static_cast<float>(mainLGL->GetCurrentWindowWidth()),
						0.0f,
						static_cast<float>(mainLGL->GetCurrentWindowHeight())
					)
				);
				mainLGL->SetShaderUniformValue("textColor", colorToUse);
			};

		defaultRenderTextShaderProgram = "rText";

		logger = std::make_unique<RenderLogger>(
			static_cast<float>(windowWidth),
			static_cast<float>(windowHeight),
			fileLoader->fontLoader.GetAllGlyphTextures(loggerFont),
			defaultRenderTextShaderProgram,
			[this]() { generalRenderTextBehaviour({ 1.0f, 1.0f, 1.0f, 1.0f }); },
			[this]() { generalRenderTextBehaviour({ 1.0f, 0.0f, 0.0f, 1.0f }); },
			[this](const std::string& labelName, LGLStructs::TextInfo& text) { mainLGL->CreateText(labelName, text); },
			[this](const std::string& labelName) { mainLGL->DeleteText(labelName); }
		);

		fileLoader->fontLoader.FreeFaceInfoByFont(loggerFont, true);

		SetRenderLoggerCallbacks();
	}

	camera = std::make_shared<CameraSim>(windowWidth, windowHeight);
	camera->SetMode(CameraSim::Mode::Fly);

	mainLGL->SetFramebufferSizeCallback([this](int width, int height) { 
		camera->SetAspect(width, height);

		if (logger)
		{
			logger->UpdateTextPos(static_cast<float>(width), static_cast<float>(height));
		}
	});

	SoundSim::SetCamera(camera);
	camera->SetPositionChangeCallback(SoundSim::UpdateCameraPosition);
	camera->SetRotationChangeCallback(SoundSim::UpdateCameraPosition);

	mainLGL->SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });

	cursorCaptureCallback =
		[this](double xpos, double ypos) { camera->RotateByMousePos(static_cast<float>(xpos), static_cast<float>(ypos)); };
	mainLGL->SetCursorPositionCallback(cursorCaptureCallback);

	mainLGL->SetScrollCallback([this](double xpos, double ypos) { mouseScrollScriptFuncs->ExecuteAllScriptFuncs(&ypos); });

	mainLGL->SetRenderDeltaCallback(ObjectSim::SetRenderDeltaTime);

	mainLGL->GetMaxAmountOfVertexAttr();
	mainLGL->CaptureMouse(true);

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

	mainLGL->EnableVSync(ENABLE_VSYNC);
	mainLGL->EnableUniformValueBatchSending(ENABLE_OPTIMIZATIONS);
	mainLGL->EnableUniformValueHashing(ENABLE_OPTIMIZATIONS);
}

void EverettEngine::SetDebugLogVisible(bool value)
{
	if (logger)
	{
		logger->EnableRender(value);
	}
}

void EverettEngine::SetDefaultWASDControls(bool value)
{
	std::string walkingDirections = "WSAD";
	for (size_t i = 0; i < walkingDirections.size(); ++i)
	{
		mainLGL->SetInteractable(
			walkingDirections[i],
			true,
			value ? 
			[this, i]() { camera->MoveInDirection(static_cast<CameraSim::Direction>(i)); } : std::function<void()>(nullptr)
		);
	}
}

void EverettEngine::EnableGizmoCreation()
{
	gizmoEnabled = true;
	gizmoVisible = true;
	LoadGizmoModel();
}

void EverettEngine::SetGizmoVisible(bool value)
{
	gizmoVisible = value;

	ModelInfo& gizmoModel = models[gizmoModelName];
	gizmoModel.GetFullModelInfo().first.lock()->render = gizmoModel.GetRelatedSolids().size() && gizmoVisible;
}

void EverettEngine::LoadGizmoModel()
{
	CreateModel(FileLoader::GetCurrentDir() + '\\' + modelPath + '\\' + gizmoModelFile, gizmoModelName);

	models[gizmoModelName].GetFullModelInfo().first.lock()->lineMode = true;
}

bool EverettEngine::CreateGizmoSolid(
	const std::string& relatedObjModelName,
	ObjectSim& relatedObject,
	const glm::vec4& gizmoColor
)
{
	std::string gizmoSolidName = relatedObjModelName + gizmoModelName;
	
	if (CreateSolidImpl(gizmoModelName, gizmoSolidName, true, gizmoVisible))
	{
		SolidSim& gizmoSolid = solids[gizmoSolidName];
		gizmoSolid.GetPositionVectorAddr() = relatedObject.GetPositionVectorAddr();
		gizmoSolid.GetScaleVectorAddr() = relatedObject.GetScaleVectorAddr();
		gizmoSolid.SetOrientation(relatedObject.GetOrientationAddr());
		gizmoSolid.ForceModelUpdate();
		gizmoSolid.EnableAutoModelUpdates();
		gizmoSolid.SetModelDefaultColor(gizmoColor);

		ColliderSim* collider = dynamic_cast<ColliderSim*>(&relatedObject);

		if (collider)
		{
			collider->AddPersistentCollisionCallback({
				[&gizmoSolid]() { gizmoSolid.SetModelDefaultColor(colliderGizmoColorCollided); },
				[&gizmoSolid]() { gizmoSolid.SetModelDefaultColor(colliderGizmoColor); }
			});
		}

		relatedObject.LinkObject(gizmoSolid);

		return true;
	}

	return false;
}

void EverettEngine::SetInteractable(
	char key,
	bool holdable,
	std::function<void()> pressFunc,
	std::function<void()> releaseFunc
)
{
	mainLGL->SetInteractable(LGL::ConvertKeyTo(std::string{ key }), holdable, pressFunc, releaseFunc);
}

void EverettEngine::RunRenderWindow()
{
	auto additionalFuncs = [this]() {
		currentStartSolidIndex = 0;
		nextStartSolidIndex = 0;

		ColliderSim::ExecuteBroadCollisionCheck();

		ExecuteFuncForAllSimObjects(&ObjectSim::UpdatePosition);

		std::vector<glm::mat4>& finalTransforms = animSystem->GetFinalTransforms();

		if (!finalTransforms.empty())
		{
			mainLGL->SetShaderUniformValue("Bones", finalTransforms, defaultShaderProgram);
		}
		animSystem->ResetFinalTransforms();

		if (models.size())
		{
			LightUpdater();
		}

		logOutput->ExecuteManualCallbacks();
		errorOutput->ExecuteManualCallbacks();

		fileLoader->dllLoader.ExecuteAllMainScriptFuncs();
	};

	mainLGL->RunRenderingCycle(additionalFuncs);
}

void EverettEngine::StopRenderWindow()
{
	mainLGL->StopRenderingCycle();
}

void EverettEngine::SetShaderPath(const std::string& shaderPath)
{
	this->shaderPath = shaderPath;
}

void EverettEngine::SetFontPath(const std::string& fontPath)
{
	this->fontPath = fontPath;
}

void EverettEngine::SetModelPath(const std::string& modelPath)
{
	this->modelPath = modelPath;
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
	return CreateModelImpl(path, name, !models.size());
}

bool EverettEngine::CreateModelImpl(const std::string& path, const std::string& name, bool regenerateShader)
{
	if (models.contains(name)) return true;
	
	ModelInfo::FullModelInfo model;
	std::string pathToUse = CheckIfRelativePathToUse(path, "models");

	if (!fileLoader->modelLoader.LoadModel(pathToUse, name, model.first, model.second)) return false;

	auto [iter, success] = models.try_emplace(name, name, pathToUse, std::move(model));

	CheckAndThrowExceptionWMessage(success, "Unexpected inability to add to model map");

	auto& [nameRef, modelSolidInfo] = *iter;
	auto modelInfo = modelSolidInfo.GetFullModelInfo().first.lock();

	CheckAndAddToNameTracker(nameRef);

	modelInfo->shaderProgram = defaultShaderProgram;
	modelInfo->render = false;

	modelInfo->modelBehaviour = [this, name]()
	{
		// Existence of the lambda implies existence of the model
		auto& model = models[name];
		auto modelPtr = model.GetFullModelInfo().first.lock();
		auto modelAnimPtr = model.GetFullModelInfo().second.lock();

		bool animationless = modelAnimPtr->animInfoVect.empty();
		mainLGL->SetShaderUniformValue("textureless", static_cast<int>(modelPtr->isTextureless));
		mainLGL->SetShaderUniformValue("animationless", static_cast<int>(animationless));

		if (!animationless)
		{
			for (auto& solidPtr : model.GetRelatedSolids())
			{
				SolidSim& solid = *solidPtr;

				if (solid.IsModelAnimationPlaying())
				{
					animSystem->ProcessAnimations(*modelAnimPtr, solid);
				}
			}
		}

		if (!model.GetRelatedSolids().empty())
		{
			size_t index = currentStartSolidIndex = nextStartSolidIndex;
			for (auto& solidPtr : model.GetRelatedSolids())
			{
				SolidSim& solid = *solidPtr;

				glm::mat4& modelMatrix = solid.GetModelMatrixAddr();

				LGLUtils::SetShaderUniformArrayAt(*mainLGL, "models", index, modelMatrix);
				LGLUtils::SetShaderUniformArrayAt(*mainLGL, "invs", index, glm::inverse(modelMatrix));
				if (solid.GetModelAnimationAmount())
				{
					LGLUtils::SetShaderUniformArrayAt(
						*mainLGL, "startingBoneIndexes", index, static_cast<int>(solid.GetModelCurrentStartingBoneIndex())
					);
				}

				if (modelPtr->isTextureless)
				{
					LGLUtils::SetShaderUniformArrayAt(*mainLGL, "defaultColors", index, solid.GetModelDefaultColor());
				}

				++index;
			}
			nextStartSolidIndex = index;
		}
	};

	modelInfo->generalMeshBehaviour = [this, name](int meshIndex)
	{
		// Existence of the lambda implies existence of the model
		auto& model = models[name];

		size_t index = currentStartSolidIndex;
		for (auto& solidPtr : model.GetRelatedSolids())
		{
			SolidSim& solid = *solidPtr;

			mainLGL->SetShaderUniformValue("solidIndex", static_cast<int>(index));
			mainLGL->SetShaderUniformValue("meshVisibility", static_cast<int>(solid.GetModelMeshVisibility(meshIndex)));
			mainLGL->SetShaderUniformValue(
				lightShaderValueNames[0].first + '.' + lightShaderValueNames[0].second[2], 
				solid.GetModelMeshShininess(meshIndex)
			);

			++index;
		}
	};

	if (regenerateShader)
	{
		GenerateShader();
	}

	mainLGL->CreateModel(name, modelInfo);

	return true;
}

bool EverettEngine::CreateSolid(const std::string& modelName, const std::string& solidName)
{
	return CreateSolidImpl(modelName, solidName, true, true);
}

bool EverettEngine::CreateSolidImpl(
	const std::string& modelName, const std::string& solidName, bool regenerateShader, bool forceVisible
)
{
	if (solids.contains(solidName))
	{
		return true;
	}

	auto modelPtr = models[modelName].GetFullModelInfo().first.lock();
	auto modelAnimPtr = models[modelName].GetFullModelInfo().second.lock();

	auto [iter, success] = solids.try_emplace(
		solidName, camera->GetPositionVectorAddr() + camera->GetFrontVector()
	);

	if (success)
	{ 
		// We use solidNameRef to add to tracker exactly because we need reference to a string
		// which maps are storing, not simply the string
		auto& [solidNameRef, newSolid] = *iter;

		models[modelName].InsertRelatedSolid(newSolid);

		for (auto& animInfo : modelAnimPtr->animInfoVect)
		{
			newSolid.AppendModelStartingBoneIndex(animSystem->GetTotalBoneAmount());
			animSystem->IncrementTotalBoneAmount(*modelAnimPtr);
		}

		if (forceVisible)
		{
			modelPtr->render = true;
		}

		CheckAndAddToNameTracker(solidNameRef);

		if (regenerateShader)
		{
			GenerateShader();
		}

		return true;
	}

	return false;
}

void EverettEngine::GenerateShader()
{
	ShaderGenerator shaderGen;
	size_t totalBoneAmount = animSystem->GetTotalBoneAmount();
	size_t totalSolidAmount = solids.size();

	std::string filePath = FileLoader::GetCurrentDir() + '\\' + shaderPath + '\\' + defaultShaderProgram;

	shaderGen.LoadPreSources(filePath);
	if (totalBoneAmount > 1)
	{
		shaderGen.SetValueToDefine("BONE_AMOUNT", totalBoneAmount);
	}
	if (totalSolidAmount > 1)
	{
		shaderGen.SetValueToDefine("SOLID_AMOUNT", totalSolidAmount);
	}
	shaderGen.GenerateShaderFiles(filePath);

	mainLGL->RecompileShader(defaultShaderProgram);
}

bool EverettEngine::CreateLight(const std::string& lightName, LightTypes lightType)
{
	bool res = CreateLightImpl(lightName, lightType);
	
	if (res && gizmoEnabled)
	{
		res = CreateGizmoSolid(lightName, lights[lightName], lightGizmoColor);
	}

	return res;
}

bool EverettEngine::CreateLightImpl(const std::string& lightName, LightTypes lightType)
{
	if (lights.contains(lightName))
	{
		return true;
	}

	auto [iter, success] = lights.try_emplace(
		lightName,
		static_cast<LightSim::LightTypes>(lightType),
		camera->GetPositionVectorAddr(),
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	if (success)
	{
		auto& [lightNameRef, newLight] = *iter;

		newLight.SetScaleVector(glm::vec3{ 0.25 });
		newLight.SetOrientation(camera->GetOrientationAddr());
		CheckAndAddToNameTracker(lightNameRef);

		return true;
	}

	return false;
}

bool EverettEngine::CreateSound(const std::string& path, const std::string& soundName)
{
	bool res = CreateSoundImpl(path, soundName);

	if (res && gizmoEnabled)
	{
		res = CreateGizmoSolid(soundName, sounds[soundName], soundGizmoColor);
	}

	return res;
}

bool EverettEngine::CreateSoundImpl(const std::string& path, const std::string& soundName)
{
	if (sounds.find(soundName) != sounds.end())
	{
		return true;
	}

	std::string pathToUse = CheckIfRelativePathToUse(path, "sounds");
	WavData wavData = fileLoader->audioLoader.GetWavDataFromFile(pathToUse);

	if (wavData)
	{
		auto [iter, success] = sounds.try_emplace(
			soundName,
			std::move(wavData)
		);

		if (success)
		{
			auto& [soundNameRef, newSound] = *iter;

			newSound.SetPositionVector(camera->GetPositionVectorAddr() + camera->GetFrontVector());
			newSound.SetScaleVector(glm::vec3{ 0.25 });
			CheckAndAddToNameTracker(soundNameRef);

			return true;
		}
	}

	return false;
}

bool EverettEngine::CreateCollider(const std::string& colliderName)
{
	bool res = CreateColliderImpl(colliderName);

	if (res && gizmoEnabled)
	{
		res = CreateGizmoSolid(colliderName, colliders[colliderName], colliderGizmoColor);
	}

	return res;
}

bool EverettEngine::CreateColliderImpl(const std::string& colliderName)
{
	if (colliders.contains(colliderName))
	{
		return true;
	}

	auto [iter, success] = colliders.try_emplace(colliderName, camera->GetPositionVectorAddr() + camera->GetFrontVector());

	if (success)
	{
		auto& [colliderNameRef, _] = *iter;

		CheckAndAddToNameTracker(colliderNameRef);

		return true;
	}

	return false;
}

bool EverettEngine::CheckIfScriptsRunning()
{
	if (fileLoader->dllLoader.AnyDLLLoaded())
	{
		std::cerr << "Cannot delete whilst scripts are running\n";
		std::cerr << "Unload dlls and try again\n";

		return true;
	}

	return false;
}

void EverettEngine::DeleteSolidsByModel(const std::string& modelName)
{
	for (auto iter = solids.begin(); iter != solids.end();)
	{
		if (iter->second.GetModelName() == modelName)
		{
			iter = DeleteSolidImpl(iter);
		}
		else
		{
			++iter;
		}
	}
}

void EverettEngine::RemoveSolidPtrFromModel(const std::string& modelName, SolidSim* solidPtr)
{
	auto iter = models.find(modelName);
	
	if (iter != models.end())
	{
		iter->second.EraseFromRelatedSolids(*solidPtr);
	}
}

bool EverettEngine::DeleteModel(const std::string& modelName)
{
	if (CheckIfScriptsRunning())
	{
		return false;
	}

	bool res = false;

	mainLGL->PauseRendering();

	auto iter = models.find(modelName);

	if (iter != models.end())
	{
		mainLGL->DeleteModel(modelName);
		DeleteSolidsByModel(modelName);
	
		allNameTracker.erase(&iter->first);
		models.erase(modelName);

		res = true;
	}
	GenerateShader();
	
	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteSolid(const std::string& solidName)
{
	if (CheckIfScriptsRunning())
	{
		return false;
	}

	bool res = false;

	mainLGL->PauseRendering();

	auto iter = solids.find(solidName);

	if (iter != solids.end())
	{
		RemoveSolidPtrFromModel(iter->second.GetModelName(), &iter->second);
		DeleteSolidImpl(iter);

		res = true;
	}

	GenerateShader();

	mainLGL->PauseRendering(false);

	return res;
}

EverettEngine::SolidCollection::iterator EverettEngine::DeleteSolidImpl(SolidCollection::iterator solidIter)
{
	allNameTracker.erase(&solidIter->first);
	return solids.erase(solidIter);
}

bool EverettEngine::DeleteLight(const std::string& lightName)
{
	if (CheckIfScriptsRunning())
	{
		return false;
	}

	bool res = false;

	mainLGL->PauseRendering();

	auto iter = lights.find(lightName);

	if (iter != lights.end())
	{
		allNameTracker.erase(&iter->first);
		lights.erase(lightName);

		res = true;
	}

	if (gizmoEnabled)
	{
		DeleteSolid(lightName + gizmoModelName);
	}

	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteSound(const std::string& soundName)
{
	if (CheckIfScriptsRunning())
	{
		return false;
	}

	bool res = false;

	mainLGL->PauseRendering();

	auto iter = sounds.find(soundName);

	if (iter != sounds.end())
	{
		allNameTracker.erase(&(*iter).first);
		sounds.erase(soundName);

		res = true;
	}

	if (gizmoEnabled)
	{
		DeleteSolid(soundName + gizmoModelName);
	}

	mainLGL->PauseRendering(false);

	return res;
}

bool EverettEngine::DeleteCollider(const std::string& colliderName)
{
	if (CheckIfScriptsRunning())
	{
		return false;
	}

	bool res = false;

	mainLGL->PauseRendering();

	auto iter = colliders.find(colliderName);

	if (iter != colliders.end())
	{
		allNameTracker.erase(&iter->first);
		colliders.erase(colliderName);

		res = true;
	}

	if (gizmoEnabled)
	{
		DeleteSolid(colliderName + gizmoModelName);
	}

	return res;
}

glm::vec3& EverettEngine::GetAmbientLightVectorAddr()
{
	return LightSim::SGetAmbientLightColorVectorAddr();
}

IObjectSim* EverettEngine::GetObjectInterface(ObjectTypes objectType, const std::string& objectName)
{
	return dynamic_cast<IObjectSim*>(GetObjectFromMap(objectType, objectName));
}

ISolidSim* EverettEngine::GetSolidInterface(const std::string& solidName)
{
	return dynamic_cast<ISolidSim*>(GetObjectFromMap(ObjectTypes::Solid, solidName));
}

ILightSim* EverettEngine::GetLightInterface(const std::string& lightName)
{
	return dynamic_cast<ILightSim*>(GetObjectFromMap(ObjectTypes::Light, lightName));
}

ISoundSim* EverettEngine::GetSoundInterface(const std::string& soundName)
{
	return dynamic_cast<ISoundSim*>(GetObjectFromMap(ObjectTypes::Sound, soundName));
}

ICameraSim* EverettEngine::GetCameraInterface()
{
	return dynamic_cast<ICameraSim*>(GetObjectFromMap(ObjectTypes::Camera, ""));
}

IColliderSim* EverettEngine::GetColliderInterface(const std::string& colliderName)
{
	return dynamic_cast<IColliderSim*>(GetObjectFromMap(ObjectTypes::Collider, colliderName));
}

void EverettEngine::LightUpdater()
{
	mainLGL->SetShaderUniformValue("proj", camera->GetProjectionMatrixAddr(), defaultShaderProgram);
	mainLGL->SetShaderUniformValue("view", camera->GetViewMatrixAddr());

	mainLGL->SetShaderUniformValue(
		"dirLightAmount", static_cast<int>(LightSim::GetAmountOfLightsByType(LightSim::LightTypes::Direction))
	);
	mainLGL->SetShaderUniformValue(
		"pointLightAmount", static_cast<int>(LightSim::GetAmountOfLightsByType(LightSim::LightTypes::Point))
	);
	mainLGL->SetShaderUniformValue(
		"spotLightAmount", static_cast<int>(LightSim::GetAmountOfLightsByType(LightSim::LightTypes::Spot))
	);
	mainLGL->SetShaderUniformValue("ambient", LightSim::SGetAmbientLightColorVectorAddr());
	
	std::array<size_t, LightSim::LightTypes::_SIZE> lightCounter{0, 0, 0};

	for (auto& [_, light] : lights)
	{
		LightSim::LightTypes lightType = light.GetLightType();
		LightSim::Attenuation atten = light.GetAttenuation();

		int lightIndex = static_cast<int>(lightType);

		switch (lightType)
		{
		case ILightSim::Direction:
			// Unimplemented
			break;
		case ILightSim::Point:
			LGLUtils::SetShaderUniformArrayAt(
				*mainLGL,
				lightShaderValueNames[lightIndex].first,
				lightCounter[lightIndex]++,
				lightShaderValueNames[lightIndex].second,
				light.GetPositionVectorAddr(), light.GetColorVectorAddr(),
				glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, atten.linear,
				atten.quadratic
			);
			break;
		case ILightSim::Spot:
			LGLUtils::SetShaderUniformArrayAt(
				*mainLGL,
				lightShaderValueNames[lightIndex].first,
				lightCounter[lightIndex]++,
				lightShaderValueNames[lightIndex].second,
				light.GetPositionVectorAddr(), light.GetFrontVector(),
				light.GetColorVectorAddr(), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f,
				atten.linear, atten.quadratic, glm::cos(glm::radians(12.5f)),
				glm::cos(glm::radians(17.5f))
			);
			break;
		default:
			break;
		}
	}

	mainLGL->SetShaderUniformValue("viewPos", camera->GetPositionVectorAddr());

	mainLGL->SetShaderUniformValue(lightShaderValueNames[0].first + '.' + lightShaderValueNames[0].second[0], 0);
	mainLGL->SetShaderUniformValue(lightShaderValueNames[0].first + '.' + lightShaderValueNames[0].second[1], 1);
}

void EverettEngine::SetupScriptDLL(const std::string& dllPath)
{
	if (fileLoader->dllLoader.IsDLLLoaded(dllPath)) return;

	std::string dllName = FileLoader::GetFileFromPath(dllPath);

	auto simScriptFuncNameMultimap = fileLoader->dllLoader.GetSimScriptFuncNamesFromDll(dllPath);

	for (auto& scriptFuncNameStruct : simScriptFuncNameMultimap)
	{
		auto& [rawFuncName, objectName, interfaceTypeName] = scriptFuncNameStruct.second;

		SetScriptToObject(GetObjectTypeToName(interfaceTypeName), objectName, rawFuncName, dllPath, dllName);
	}

	auto keybindScriptFuncNameMap = fileLoader->dllLoader.GetKeybindScriptFuncNamesFromDll(dllPath);

	for (auto& scriptFuncNameStruct : keybindScriptFuncNameMap)
	{
		auto& [rawPressedFuncName, rawReleasedFuncName, holdable] = scriptFuncNameStruct.second;

		if (scriptFuncNameStruct.first == "MouseScroll")
		{
			SetScriptToMouseScroll(rawPressedFuncName, dllPath, dllName);
		}
		else
		{
			SetScriptToKey(scriptFuncNameStruct.first, rawPressedFuncName, rawReleasedFuncName, holdable, dllPath, dllName);
		}
	}
}

bool EverettEngine::IsDLLLoaded(const std::string& dllPath)
{
	if (fileLoader)
	{
		return fileLoader->dllLoader.IsDLLLoaded(dllPath);
	}

	return false;
}

void EverettEngine::UnsetScript(const std::string& dllPath)
{
	if (fileLoader)
	{ 
		// Since non persistent colliders can only be added via script funcs by user
		// unloading presents a risk of forgetting the clean up, causing potential crash
		// on attempt to call function ptr which is not accessible after unset
		mainLGL->PauseRendering();
		for (auto& [_, collider] : colliders)
		{
			if (collider.IsScriptFuncAdded(FileLoader::GetFileFromPath(dllPath)))
			{
				collider.ClearCollisionCallbacks();
			}
		}
		mainLGL->PauseRendering(false);

		fileLoader->dllLoader.UnloadScriptDLL(dllPath);
	}
}

bool EverettEngine::IsObjectScriptSet(
	ObjectTypes objectType,
	const std::string& objectName,
	const std::string& dllName
)
{
	ObjectSim* object = GetObjectFromMap(objectType, objectName);
	
	return object && object->IsScriptFuncRunnable(dllName);
}

void EverettEngine::SetScriptToObject(
	ObjectTypes objectType,
	const std::string& objectName,
	std::string_view rawFuncName,
	const std::string& dllPath,
	const std::string& dllName
)
{
	ObjectSim* object = GetObjectFromMap(objectType, objectName);

	if (object)
	{
		ScriptFuncWeakPtr scriptFuncWeakPtr;

		if (fileLoader->dllLoader.GetScriptFuncFromDLL(dllPath, rawFuncName.data(), scriptFuncWeakPtr))
		{
			if (!object->IsScriptFuncAdded(dllName))
			{
				object->AddScriptFunc(dllPath, dllName, scriptFuncWeakPtr);
			}

			object->ExecuteScriptFunc(dllName);
		}
	}
}

void EverettEngine::SetScriptToKey(
	const std::string& keyName,
	std::string_view pressedFuncName,
	std::string_view releasedFuncName,
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

		ScriptFuncWeakPtr scriptFuncPress;
		ScriptFuncWeakPtr scriptFuncRelease;
		bool anyFuncAdded{};

		if (!pressedFuncName.empty())
		{
			anyFuncAdded |= fileLoader->dllLoader.GetScriptFuncFromDLL(dllPath, pressedFuncName.data(), scriptFuncPress);
		}

		if (!releasedFuncName.empty())
		{
			anyFuncAdded |= fileLoader->dllLoader.GetScriptFuncFromDLL(dllPath, releasedFuncName.data(), scriptFuncRelease);
		}

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
					scriptFuncPressWrapper = [this, keyName]() { keyScriptFuncMap[keyName].pressedFuncs.ExecuteAllScriptFuncs(); };
				}

				if (scriptFuncRelease.lock())
				{
					scriptFuncReleaseWrapper = [this, keyName]() { keyScriptFuncMap[keyName].releasedFuncs.ExecuteAllScriptFuncs(); };
				}

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

void EverettEngine::SetScriptToMouseScroll(
	std::string_view rawFuncName,
	const std::string& dllPath,
	const std::string& dllName
)
{
	ScriptFuncWeakPtr scriptFuncWeakPtr;

	fileLoader->dllLoader.GetScriptFuncFromDLL(dllPath, rawFuncName.data(), scriptFuncWeakPtr);

	if (scriptFuncWeakPtr.lock())
	{
		mouseScrollScriptFuncs->AddScriptFunc(dllPath, dllName, scriptFuncWeakPtr);
	}
}

template<typename FunctionType, typename... Params>
void EverettEngine::ExecuteFuncForAllSimObjects(FunctionType func, Params&&... values)
{
	static_assert(ConfirmMemberOf<ObjectSim, FunctionType, Params...>, "Must accept ObjectSim or derived member func");

	if constexpr (ConfirmMemberOf<CameraSim, FunctionType, Params...>)
	{
		if (camera)
		{
			((*camera).*func)(std::forward<Params>(values)...);
		}
	}
	if constexpr (ConfirmMemberOf<SolidSim, FunctionType, Params...>)
	{
		ExecuteFuncForAllSimObjectsFor(solids, func, std::forward<Params>(values)...);
	}
	if constexpr (ConfirmMemberOf<LightSim, FunctionType, Params...>)
	{
		ExecuteFuncForAllSimObjectsFor(lights, func, std::forward<Params>(values)...);
	}
	if constexpr (ConfirmMemberOf<SoundSim, FunctionType, Params...>)
	{
		ExecuteFuncForAllSimObjectsFor(sounds, func, std::forward<Params>(values)...);
	}
	if constexpr (ConfirmMemberOf<ColliderSim, FunctionType, Params...>)
	{
		ExecuteFuncForAllSimObjectsFor(colliders, func, std::forward<Params>(values)...);
	}
}

template<typename Sim, typename FunctionType, typename... Params>
void EverettEngine::ExecuteFuncForAllSimObjectsFor(
	std::unordered_map<std::string, Sim>& container, FunctionType func, Params&&... values
)
{
	for (auto& [_, sim] : container)
	{
		(sim.*func)(std::forward<Params>(values)...);
	}
}

std::string EverettEngine::CheckIfRelativePathToUse(const std::string& path, const std::string& expectedFolder)
{
	return path.find(':') != std::string::npos ? path : expectedFolder + "//" + path;
}

std::vector<EverettStructs::BasicFileInfo> EverettEngine::GetLoadedScriptDLLs()
{
	return fileLoader->dllLoader.GetLoadedScriptDlls();
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
		object = GetObjectFromTheMap(solids, objectName);
		break;
	case EverettEngine::ObjectTypes::Light:
		object = GetObjectFromTheMap(lights, objectName);
		break;
	case EverettEngine::ObjectTypes::Sound:
		object = GetObjectFromTheMap(sounds, objectName);
		break;
	case EverettEngine::ObjectTypes::Collider:
		object = GetObjectFromTheMap(colliders, objectName);
		break;
	default:
		ThrowExceptionWMessage("Unreachable");
	}

#ifdef _DEBUG
	if (object)
	{
		std::cout << "Retrieved object " << objectName << " from " << object->GetThisObjectTypeNameStr() << " map\n";
	}
	else
	{
		std::cerr << "Failed to retrieve object " << objectName << " from maps\n";
	}
#endif

	return object;
}

template<typename Sim>
ObjectSim* EverettEngine::GetObjectFromTheMap(
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

template<typename Sim>
void EverettEngine::SaveObjectsToFile(std::fstream& file)
{
	// Shame there's no constexpr switch
	static_assert(std::is_base_of_v<ObjectSim, Sim>);

	if constexpr (std::is_same_v<Sim, CameraSim>)
	{
		file << camera->GetSimInfoToSave("");
	}
	else if constexpr (std::is_same_v<Sim, SolidSim>)
	{
		for (auto& [solidName, solid] : solids)
		{
			std::string modelName = solid.GetModelName();

			if (modelName != gizmoModelName)
			{
				file << solid.GetSimInfoToSave(modelName + '*' + solidName + '*' + models[modelName].GetModelPath());
			}
		}
	}
	else if constexpr (std::is_same_v<Sim, LightSim>)
	{
		for (auto& [lightName, light] : lights)
		{
			file << light.GetSimInfoToSave(light.GetCurrentLightType() + '*' + lightName);
		}
	}
	else if constexpr (std::is_same_v<Sim, SoundSim>)
	{
		for (auto& [soundName, sound] : sounds)
		{
			file << sound.GetSimInfoToSave(soundName);
		}
	}
	else if constexpr (std::is_same_v<Sim, ColliderSim>)
	{
		for (auto& [colliderName, collider] : colliders)
		{
			file << collider.GetSimInfoToSave(colliderName);
		}
	}
}

void EverettEngine::ResetEngine(const std::optional<AssetPaths>& assetPaths)
{
	mainLGL->PauseRendering();

	if (assetPaths)
	{
		for (auto& [modelName, modelInfo] : models)
		{
			if (!assetPaths.value().modelPaths.contains(modelInfo.GetModelPath()))
			{
				mainLGL->DeleteModel(modelName);
			}
		}
	}
	else
	{
		SetRenderLoggerCallbacks(false);
		mainLGL->ResetLGL();

		SetRenderLoggerCallbacks();
	}

	camera->ClearScriptFuncMap();
	solids.clear();
	models.clear();
	lights.clear();
	sounds.clear();
	colliders.clear();

	keyScriptFuncMap.clear();
	allNameTracker.clear();

	if (fileLoader)
	{
		if (assetPaths)
		{
			fileLoader->DeleteAllAbsentAssets(assetPaths.value());
		}
		else
		{
			fileLoader->DeleteAllAbsentAssets();
		}
	}

	if (gizmoEnabled)
	{
		LoadGizmoModel();
	}

	mainLGL->PauseRendering(false);
}

bool EverettEngine::SaveDataToFile(const std::string& filePath)
{
	std::string realFilePath = filePath + saveFileType;
	std::fstream file(realFilePath, std::ios::out);

	file << SimSerializer::GetLatestVersionStr();
	file << "AmbientLight*" << SimSerializer::GetValueToSaveFrom(LightSim::SGetAmbientLightColorVectorAddr()) << '\n';

	SaveObjectsToFile<CameraSim>(file);
	SaveObjectsToFile<SolidSim>(file);
	SaveObjectsToFile<LightSim>(file);
	SaveObjectsToFile<SoundSim>(file);
	SaveObjectsToFile<ColliderSim>(file);

	auto loadedScriptDlls = fileLoader->dllLoader.GetLoadedScriptDlls();

	if (!loadedScriptDlls.empty())
	{
		file << ("ScriptDLLs*" + SimSerializer::GetValueToSaveFrom(loadedScriptDlls) + '\n');
	}

	file.close();

	return true;
}

void EverettEngine::LoadCameraFromLine(std::string_view& line)
{
	bool res = false;

	ApplySimInfoFromLine<CameraSim>(line, {"", "", "Camera", ""}, res);

	CheckAndThrowExceptionWMessage(res, "Camera setup from file failed");
}

template<typename Sim>
void EverettEngine::ApplySimInfoFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo, bool& res)
{
	Sim* createdObject = dynamic_cast<Sim*>(GetObjectFromMap(
		GetObjectPureTypeToName(typeid(Sim)),
		objectInfo[ObjectInfoNames::ObjectName])
	);

	if (createdObject)
	{
		res = createdObject->SetSimInfoToLoad(line);

		if (!res) return;

		if constexpr (std::is_base_of_v<SolidSim, Sim>) 
		{
			createdObject->ForceModelUpdate();
		}
	}
}

void EverettEngine::LoadSolidFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;

	if (CreateModelImpl(objectInfo[ObjectInfoNames::Path], objectInfo[ObjectInfoNames::SubtypeName], false) &&
		CreateSolidImpl(objectInfo[ObjectInfoNames::SubtypeName], objectInfo[ObjectInfoNames::ObjectName], false, true)
	)
	{
		ApplySimInfoFromLine<SolidSim>(line, objectInfo, res);
	}

	CheckAndThrowExceptionWMessage(res, "Solid creation from file failed");
}

void EverettEngine::LoadLightFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;
	std::string lightName = objectInfo[ObjectInfoNames::ObjectName];
	LightTypes lightType = static_cast<LightTypes>(LightSim::GetTypeToName(objectInfo[ObjectInfoNames::SubtypeName]));

	if (CreateLightImpl(lightName, lightType))
	{
		ApplySimInfoFromLine<LightSim>(line, objectInfo, res);
		if (gizmoEnabled)
		{
			CreateGizmoSolid(lightName, lights[lightName], lightGizmoColor);
		}
	}

	CheckAndThrowExceptionWMessage(res, "Light creation from file failed");
}

void EverettEngine::LoadSoundFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;
	std::string soundName = objectInfo[ObjectInfoNames::ObjectName];

	if (CreateSoundImpl(objectInfo[ObjectInfoNames::Path], soundName))
	{
		ApplySimInfoFromLine<SoundSim>(line, objectInfo, res);
		if (gizmoEnabled)
		{
			CreateGizmoSolid(soundName, sounds[soundName], soundGizmoColor);
		}
	}

	CheckAndThrowExceptionWMessage(res, "Sound creation from file failed");
}

void EverettEngine::LoadColliderFromLine(std::string_view& line, const std::array<std::string, 4>& objectInfo)
{
	bool res = false;
	std::string colliderName = objectInfo[ObjectInfoNames::ObjectName];

	if (CreateColliderImpl(colliderName))
	{
		ApplySimInfoFromLine<ColliderSim>(line, objectInfo, res);
		if (gizmoEnabled)
		{
			CreateGizmoSolid(colliderName, colliders[colliderName], colliderGizmoColor);
		}
	}

	CheckAndThrowExceptionWMessage(res, "Collider creation from file failed");
}

void EverettEngine::LoadScriptDLLsFromLine(std::string_view& line)
{
	size_t objectInfoAmount = std::count(line.begin(), line.end(), '*');
	CheckAndThrowExceptionWMessage(
		static_cast<bool>(objectInfoAmount == 1 || objectInfoAmount == 4),
		"Invalid keybind info amount during world load"
	);

	for (size_t i = 0; i < objectInfoAmount; ++i)
	{
		line.remove_prefix(line.find('*') + 1);
	}

	std::vector<std::pair<std::string, std::string>> legacyDllInfo;
	std::vector<EverettStructs::BasicFileInfo> dllInfo;

	SimSerializer::SetValueToLoadFrom(line, legacyDllInfo, 2, 12);

	if (legacyDllInfo.empty())
	{
		SimSerializer::SetValueToLoadFrom(line, dllInfo, 12);

		for (auto& [dllPath, dllName, dllHash] : dllInfo)
		{
			if (dllHash != FileLoader::GetFileHash(dllPath))
			{
				std::cerr << dllName << " was changed, can't guarantee correct execution\n";
			}

			SetupScriptDLL(dllPath);
		}
	}
	else
	{
		for (auto& [dllName, dllPath] : legacyDllInfo)
		{
			SetupScriptDLL(dllPath);
		}
	}
}

bool EverettEngine::LoadDataFromFile(const std::string& filePath)
{
	std::string pathToUse = CheckIfRelativePathToUse(filePath, "worlds");

	std::fstream file(pathToUse, std::ios::in);
	std::string lineLoader;
	std::string_view line;
	std::array<std::string, ObjectInfoNames::_SIZE> objectInfo{};

	AssetPaths loadedFiles = GetPathsFromWorldFile(pathToUse);
	if (gizmoEnabled)
	{
		LoadGizmoModel();
	}

	std::getline(file, lineLoader);
	line = lineLoader;
	SimSerializer::GetVersionFromLine(line);

	ResetEngine(loadedFiles);
	mainLGL->PauseRendering();
	while (!file.eof())
	{
		std::getline(file, lineLoader);
		line = lineLoader;
		std::string_view lineTitle = line.substr(0, line.find('*'));

		if (lineTitle == "AmbientLight")
		{
			SimSerializer::SetValueToLoadFrom(line, LightSim::SGetAmbientLightColorVectorAddr(), 4);
		}
		else if (lineTitle == "Keybind" || lineTitle == "ScriptDLLs") // Keybind kept for backwards comp.
		{
			LoadScriptDLLsFromLine(line);
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
			case EverettEngine::ObjectTypes::Collider:
				LoadColliderFromLine(line, objectInfo);
				break;
			default:
				ThrowExceptionWMessage("Unreachable");
			}
		}
	}
	GenerateShader();
	mainLGL->PauseRendering(false);

	return true;
}

AssetPaths EverettEngine::GetPathsFromWorldFile(const std::string& filePath)
{
	AssetPaths assetPaths;

	std::fstream file(filePath, std::ios::in);
	std::string lineLoader;
	std::string_view line;
	std::array<std::string, ObjectInfoNames::_SIZE> objectInfo{};

	std::getline(file, lineLoader);
	line = lineLoader;
	SimSerializer::GetVersionFromLine(line);

	while (!file.eof())
	{
		std::getline(file, lineLoader);
		line = lineLoader;
		std::string_view lineTitle = line.substr(0, line.find('*'));

		if (!(line.empty() || lineTitle == "AmbientLight" || lineTitle == "ScriptDLLs" || lineTitle == "Keybind"))
		{
			SimSerializer::GetObjectInfo(line, objectInfo);

			switch (GetObjectTypeToName(objectInfo[ObjectInfoNames::ObjectType]))
			{
			case EverettEngine::ObjectTypes::Camera:
			case EverettEngine::ObjectTypes::Light:
			case EverettEngine::ObjectTypes::Collider:
				break;
			case EverettEngine::ObjectTypes::Solid:
				assetPaths.modelPaths.insert(objectInfo[ObjectInfoNames::Path]);
				break;
			case EverettEngine::ObjectTypes::Sound:
				assetPaths.soundPaths.insert(objectInfo[ObjectInfoNames::Path]);
				break;
			default:
				ThrowExceptionWMessage("Unreachable");
			}
		}

		while (line.find(':') != std::string::npos)
		{
			line.remove_prefix(line.find(':') - 1);
			assetPaths.scriptPaths.insert(std::string(line.substr(0, line.find('.') + 4)));
			line.remove_prefix(line.find('.') + 4);
		}
	}

	return assetPaths;
}

void EverettEngine::HidePathsInWorldFile(
	const std::string& originalFilePath,
	const std::string& hidenFilePath
)
{
	std::fstream originalFile(originalFilePath, std::ios::in);
	std::string line;
	std::string hidenFileContent;

	while (!originalFile.eof())
	{
		std::getline(originalFile, line);
		size_t startPointOfPath;

		while ((startPointOfPath = line.find(':') - 1) != std::string::npos - 1)
		{
			size_t endPointOfPath = line.find('.', startPointOfPath) + 4;
			std::string fileName = line.substr(startPointOfPath, endPointOfPath - startPointOfPath);
			fileName = fileName.substr(fileName.rfind('\\') + 1);
			line.replace(startPointOfPath, endPointOfPath - startPointOfPath, fileName);
		}

		hidenFileContent += line;
		hidenFileContent += '\n';
	}

	std::fstream hidenFile(hidenFilePath, std::ios::out);
	hidenFile << hidenFileContent;
	hidenFile.close();
}

template<typename Sim>
std::vector<std::string> EverettEngine::GetNameList(const std::unordered_map<std::string, Sim>& sims)
{
	std::vector<std::string> names;
	names.reserve(sims.size());

	for (auto& sim : sims)
	{
		names.push_back(sim.first);
	}

	return names;
}

std::vector<std::string> EverettEngine::GetCreatedModels(bool getFullPaths)
{
	std::vector<std::string> createdModels;

	for (const auto& [modelName, model] : models)
	{
		if (modelName != gizmoModelName)
		{
			createdModels.push_back(getFullPaths ? model.GetModelPath() : modelName);
		}
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

	ThrowExceptionWMessage("Nonexistent type");
}

EverettEngine::ObjectTypes EverettEngine::GetObjectTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameStr == objectName || objectNamePair.interfaceNameStr == objectName)
		{
			return objectNamePair.nameEnum;
		}
	}

	ThrowExceptionWMessage("Nonexistent type");
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

	ThrowExceptionWMessage("Nonexistent type");
}

std::type_index EverettEngine::GetObjectPureTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameStr == objectName || objectNamePair.interfaceNameStr == objectName)
		{
			return objectNamePair.pureType;
		}
	}

	ThrowExceptionWMessage("Nonexistent type");
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

	ThrowExceptionWMessage("Nonexistent type");
}

std::vector<std::string> EverettEngine::GetNamesByObject(ObjectTypes objType, bool getAdditionalInfo)
{
	switch (objType)
	{
	case ObjectTypes::Solid:
		return GetSolidList(getAdditionalInfo);
	case ObjectTypes::Light:
		return GetLightList(getAdditionalInfo);
	case ObjectTypes::Sound:
		return GetSoundList();
	case ObjectTypes::Collider:
		return GetColliderList();
	default:
		ThrowExceptionWMessage("Unreachable");
	}
}

std::vector<std::string> EverettEngine::GetSolidList(bool getModelNames)
{
	std::vector<std::string> solidNames;

	for (auto& [solidName, solid] : solids)
	{
		std::string modelName = solid.GetModelName();
		if (modelName != gizmoModelName)
		{
			solidNames.push_back((getModelNames ? solid.GetModelName() + '.' : "") + solidName);
		}
	}

	return solidNames;
}

std::vector<std::string> EverettEngine::GetLightList(bool getLightTypes)
{
	if (!getLightTypes)
	{
		return GetNameList(lights);
	}

	std::vector<std::string> lightNames;

	for (auto& [lightName, light] : lights)
	{
		lightNames.push_back(light.GetCurrentLightType() + '.' + lightName);
	}

	return lightNames;
}

std::vector<std::string> EverettEngine::GetSoundList()
{
	return GetNameList(sounds);
}

std::vector<std::string> EverettEngine::GetColliderList()
{
	return GetNameList(colliders);
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

void EverettEngine::RemoveWindowHandler(const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->RemoveWindowHandle(name);
	}
}

void EverettEngine::AddCurrentWindowHandler(const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->AddCurrentWindowHandle(name);
	}
}

void EverettEngine::CloseWindow(const std::string& name)
{
	if (hwndHolder)
	{
		hwndHolder->CloseWindow(name);
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

std::string EverettEngine::GetDateTimeStr()
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

void EverettEngine::SetLogCallback(bool value)
{
	std::cout.set_rdbuf(value ? logOutput->GetStreamBuffer() : stdOutStreamBuffer);
	std::cerr.set_rdbuf(value ? errorOutput->GetStreamBuffer() : stdErrStreamBuffer);

	if (value)
	{
		logOutput->SetEndlineCallback("AllLog", [this](const std::string& str) { logStrings.push_back(str); });
		errorOutput->SetEndlineCallback("AllLog", [this](const std::string& str) { logStrings.push_back("ERROR: " + str); });

		EverettException::SetLogReportCreator([this]() { CreateLogReport(); });
	}
	else
	{
		logOutput->RemoveEndlineCallback("AllLog");
		errorOutput->RemoveEndlineCallback("AllLog");

		EverettException::SetLogReportCreator(nullptr);
	}
}

void EverettEngine::SetRenderLoggerCallbacks(bool value)
{
	if (logger)
	{
		if (value)
		{
			logger->CreateLogMessage("Trigger render text shader load and custom output buffer set");
			logOutput->SetEndlineCallback(
				"RenderLogger", [this](const std::string& str) { logger->CreateLogMessage(str); }, true
			);
			errorOutput->SetEndlineCallback(
				"RenderLogger", [this](const std::string& str) { logger->CreateErrorMessage(str); }, true
			);
		}
		else
		{
			logOutput->RemoveEndlineCallback("RenderLogger");
			errorOutput->RemoveEndlineCallback("RenderLogger");
		}
	}
}

void EverettEngine::CreateLogReport()
{
	std::fstream file("EverettEngineLogReport-" + GetDateTimeStr() + ".txt", std::ios::out);

	for (auto& str : logStrings)
	{
		file << str << '\n';
	}

	file.close();
}

std::string EverettEngine::GetAvailableObjectName(const std::string& name)
{
	if (allNameTracker.find(&name) != allNameTracker.end())
	{
		return (name + std::to_string(allNameTracker[&name]));
	}

	return name;
}