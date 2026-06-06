#include <iostream>
#include <cmath>
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

#include "CommonStrEdits.h"

#include "stdEx/type_traitsEx.h"

#include "AnimSystem.h"

#include "CustomOutput.h"
#include "RenderLogger.h"

#define EVERETT_EXPORT
#include "EverettEngine.h"
#include "EverettExceptionInternal.h"

#include "ModelInfo.h"
#include "KeyScriptFuncInfo.h"

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

	mainLGL->SetCursorPositionCallback(
		[this](double xpos, double ypos) {
			camera->RotateByMousePos(static_cast<float>(xpos), static_cast<float>(ypos));
			ExecuteVectorOfFuncs(mouseMoveScriptFuncs, xpos, ypos); 
		}
	);
	mainLGL->SetScrollCallback([this](double xpos, double ypos) { ExecuteVectorOfFuncs(mouseScrollScriptFuncs, ypos); });

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

void EverettEngine::AddInteractable(
	int key,
	bool holdable,
	std::function<void()> pressFunc,
	std::function<void()> releaseFunc
)
{
	bool addNew = false;

	if (!keyScriptFuncMap.contains(key))
	{
		keyScriptFuncMap.try_emplace(key, KeyScriptFuncInfo{});
		addNew = true;
	}

	if (pressFunc)
	{
		keyScriptFuncMap[key].AddPressedFunc(pressFunc, holdable);
	}

	if (releaseFunc)
	{
		keyScriptFuncMap[key].AddReleasedFunc(releaseFunc);
	}

	if (addNew)
	{
		mainLGL->SetInteractable(
			key, true, 
			[this, key]() { keyScriptFuncMap[key].ButtonPressed();  },
			[this, key]() { keyScriptFuncMap[key].ButtonReleased(); }
		);
	}
}

void EverettEngine::AddMouseScrollCallback(std::function<void(double)> callback)
{
	mouseScrollScriptFuncs.push_back(callback);
}

void EverettEngine::AddMouseMoveCallback(std::function<void(double, double)> callback)
{
	mouseMoveScriptFuncs.push_back(callback);
}

// Since some containers are controlled externally (via script funcs by user)
// unloading presents a risk of forgetting the clean up, causing potential crash
// on attempt to call function ptr which is not accessible after unset
void EverettEngine::ClearExternallyControlledContainers()
{
	ExecuteFuncForAllSimObjects(&ColliderSim::ClearCollisionCallbacks);
	keyScriptFuncMap.clear();
	mouseScrollScriptFuncs.clear();
	mouseMoveScriptFuncs.clear();
}

void EverettEngine::RunRenderWindow()
{
	auto additionalFuncs = [this]() {
		currentStartSolidIndex = 0;
		nextStartSolidIndex = 0;

		CheckAndLoadRequestedWorld();

		ColliderSim::ExecuteBroadCollisionCheck();

		ExecuteFuncForAllSimObjects(&ObjectSim::UpdatePosition);

		std::vector<glm::mat4>& finalTransforms = animSystem->GetFinalTransforms();

		if (!finalTransforms.empty())
		{
			mainLGL->SetShaderUniformValue("Bones", finalTransforms, defaultShaderProgram);
		}

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

int EverettEngine::ConvertKeyTo(char c)
{
	return LGL::ConvertKeyTo(c);
}

int EverettEngine::ConvertKeyTo(const char* keyName)
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

	auto [iter, success] = models.try_emplace(name, pathToUse, std::move(model));

	CheckAndThrowExceptionWMessage(success, "Unexpected inability to add to model map");

	auto& [nameRef, modelSolidInfo] = *iter;
	modelSolidInfo.SetModelNamePtr(nameRef);
	auto modelInfo = modelSolidInfo.GetFullModelInfo().first.lock();

	CheckAndAddToNameTracker(nameRef);

	modelInfo->shaderProgram = defaultShaderProgram;
	modelInfo->render = false;

	modelSolidInfo.SetModelBehaviour([this](const ModelInfo& model)
	{
		// Existence of the lambda implies existence of the model
		auto modelPtr = model.GetFullModelInfo().first.lock();
		auto modelAnimPtr = model.GetFullModelInfo().second.lock();

		bool animationless = modelAnimPtr->animInfoVect.empty();
		mainLGL->SetShaderUniformValue("textureless", static_cast<int>(modelPtr->isTextureless));
		mainLGL->SetShaderUniformValue("animationless", static_cast<int>(animationless));

		if (!animationless)
		{
			for (auto& solidPtr : model.GetRelatedSolids())
			{
				animSystem->ProcessAnimations(*modelAnimPtr, *solidPtr);
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
	});

	modelSolidInfo.SetGeneralMeshBehaviour([this](const ModelInfo& model, int meshIndex)
	{
		// Existence of the lambda implies existence of the model
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
	});

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

		animSystem->IncrementTotalBoneAmount(*modelAnimPtr);
		// We pass start bone index reference for automated reassignment of start bone index in all created
		// solids without looping through the map on deletion of one specific solid removing entire O(n) walkthrough
		newSolid.SetModelStartBoneIndexRef(animSystem->GetLastStartBoneIndexRef());

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

std::expected<void, std::string> EverettEngine::RenameObject(
	const std::string& oldName, const std::string& newName, std::optional<ObjectTypes> hintType
)
{
	if (CheckIfScriptsRunning())
		return std::unexpected("Cannot rename whilst scripts are running\n");
	if (oldName == "Camera" || (hintType.has_value() && hintType == ObjectTypes::Camera))
		return std::unexpected("Cannot rename camera object");
	if (allNameTracker.contains(&newName))
		return std::unexpected("New name already exists");

	auto TryRenameKey = [&]<typename Type>(
		std::unordered_map<std::string, Type>& cont, std::optional<ObjectTypes> objectType = std::nullopt
	){ 
		if (CheckHintAndType(hintType, objectType) && cont.contains(oldName))
		{
			auto node = cont.extract(oldName);
			auto& key = node.key();
			allNameTracker.erase(&key);

			key = newName;
			cont.insert(std::move(node));

			CheckAndAddToNameTracker(key);

			return FoundCorrectOne;
		}

		return CheckNextOne;
	};

	mainLGL->PauseRendering();

	// Monadic-like check
	auto result = TryRenameKey(models) && TryRenameKey(solids, ObjectTypes::Solid) && 
		TryRenameKey(lights, ObjectTypes::Light) && TryRenameKey(sounds, ObjectTypes::Sound) && 
		TryRenameKey(colliders, ObjectTypes::Collider);

	mainLGL->PauseRendering(false);

	if (result != FoundCorrectOne) return std::unexpected("Old name does not exist");

	return {};
}

std::expected<void, std::string> EverettEngine::DeleteObject(
	const std::string& name, std::optional<ObjectTypes> hintType
)
{
	if (CheckIfScriptsRunning())
		return std::unexpected("Cannot rename whilst scripts are running\n");
	if (name == "Camera" || (hintType.has_value() && hintType == ObjectTypes::Camera))
		return std::unexpected("Cannot rename camera object");

	auto DeleteWrapperFunc = [&](
		ObjectModificationState(EverettEngine::*deleter)(const std::string&), const std::string& name,
		std::optional<ObjectTypes> objectType = std::nullopt)
	{
		if (CheckHintAndType(hintType, objectType))
		{
			return (this->*deleter)(name);
		}

		return CheckNextOne;
	};

	mainLGL->PauseRendering();

	// Monadic-like check
	auto result = 
		DeleteWrapperFunc(&EverettEngine::DeleteModel, name) &&
		DeleteWrapperFunc(&EverettEngine::DeleteSolid, name, ObjectTypes::Solid) &&
		DeleteWrapperFunc(&EverettEngine::DeleteLight, name, ObjectTypes::Light) &&
		DeleteWrapperFunc(&EverettEngine::DeleteSound, name, ObjectTypes::Sound) &&
		DeleteWrapperFunc(&EverettEngine::DeleteCollider, name, ObjectTypes::Collider);

	mainLGL->PauseRendering(false);

	if (result != FoundCorrectOne) return std::unexpected("Old name does not exist");

	return {};
}

bool EverettEngine::CheckIfScriptsRunning(bool displayErrorIfYes)
{
	if (fileLoader->dllLoader.AnyDLLLoaded())
	{
		if (displayErrorIfYes)
		{
			std::cerr << "Cannot do this action whilst scripts are running\n";
			std::cerr << "Unload dlls and try again\n";
		}

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

EverettEngine::ObjectModificationState EverettEngine::DeleteModel(const std::string& modelName)
{
	auto iter = models.find(modelName);

	if (iter != models.end())
	{
		mainLGL->DeleteModel(modelName);
		DeleteSolidsByModel(modelName);
	
		allNameTracker.erase(&iter->first);
		models.erase(modelName);

		GenerateShader();
		
		return FoundCorrectOne;
	}

	return CheckNextOne;
}

EverettEngine::ObjectModificationState EverettEngine::DeleteSolid(const std::string& solidName)
{
	auto iter = solids.find(solidName);

	if (iter != solids.end())
	{
		RemoveSolidPtrFromModel(iter->second.GetModelName(), &iter->second);
		DeleteSolidImpl(iter);

		GenerateShader();

		return FoundCorrectOne;
	}

	return CheckNextOne;
}

bool EverettEngine::CheckHintAndType(
	const std::optional<ObjectTypes>& hintType, const std::optional<ObjectTypes>& objectType
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

EverettEngine::SolidCollection::iterator EverettEngine::DeleteSolidImpl(SolidCollection::iterator solidIter)
{
	allNameTracker.erase(&solidIter->first);
	animSystem->DecrementTotalBoneAmount(solidIter->second);
	return solids.erase(solidIter);
}

EverettEngine::ObjectModificationState EverettEngine::DeleteLight(const std::string& lightName)
{
	auto iter = lights.find(lightName);

	if (iter != lights.end())
	{
		allNameTracker.erase(&iter->first);
		lights.erase(lightName);

		if (gizmoEnabled)
		{
			DeleteSolid(lightName + gizmoModelName);
		}

		return FoundCorrectOne;
	}
	
	return CheckNextOne;
}

EverettEngine::ObjectModificationState EverettEngine::DeleteSound(const std::string& soundName)
{
	auto iter = sounds.find(soundName);

	if (iter != sounds.end())
	{
		allNameTracker.erase(&(*iter).first);
		sounds.erase(soundName);

		if (gizmoEnabled)
		{
			DeleteSolid(soundName + gizmoModelName);
		}

		return FoundCorrectOne;
	}

	return CheckNextOne;
}

EverettEngine::ObjectModificationState EverettEngine::DeleteCollider(const std::string& colliderName)
{
	auto iter = colliders.find(colliderName);

	if (iter != colliders.end())
	{
		allNameTracker.erase(&iter->first);
		colliders.erase(colliderName);

		if (gizmoEnabled)
		{
			DeleteSolid(colliderName + gizmoModelName);
		}

		return FoundCorrectOne;
	}

	return CheckNextOne;
}

glm::vec3& EverettEngine::GetAmbientLightVectorAddr()
{
	return LightSim::SGetAmbientLightColorVectorAddr();
}

IObjectSim* EverettEngine::GetObjectInterface(const char* objectName, std::optional<ObjectTypes> hintType)
{
	IObjectSim* objectPtr = nullptr;

	if (!hintType)
	{
		for (size_t objectType = 0; objectType < std::to_underlying(ObjectTypes::_SIZE); ++objectType)
		{
			if ((objectType == 0) == (std::string_view(objectName) == "Camera"))
			{
				objectPtr = GetObjectFromMap(static_cast<ObjectTypes>(objectType), objectName, false);
			}

			if (objectPtr) break;
		}
	}
	else
	{
		objectPtr = GetObjectFromMap(hintType.value(), objectName);
	}

	return objectPtr;
}

ISolidSim* EverettEngine::GetSolidInterface(const char* solidName)
{
	return dynamic_cast<ISolidSim*>(GetObjectFromMap(ObjectTypes::Solid, solidName));
}

ILightSim* EverettEngine::GetLightInterface(const char* lightName)
{
	return dynamic_cast<ILightSim*>(GetObjectFromMap(ObjectTypes::Light, lightName));
}

ISoundSim* EverettEngine::GetSoundInterface(const char* soundName)
{
	return dynamic_cast<ISoundSim*>(GetObjectFromMap(ObjectTypes::Sound, soundName));
}

IColliderSim* EverettEngine::GetColliderInterface(const char* colliderName)
{
	return dynamic_cast<IColliderSim*>(GetObjectFromMap(ObjectTypes::Collider, colliderName));
}

ICameraSim* EverettEngine::GetCameraInterface()
{
	return dynamic_cast<ICameraSim*>(GetObjectFromMap(ObjectTypes::Camera, ""));
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

	if (fileLoader->dllLoader.LoadDLL(dllPath))
	{
		fileLoader->dllLoader.ExecuteScriptInitFor(dllPath, dynamic_cast<IEverettEngine&>(*this));
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
		mainLGL->PauseRendering();
		ClearExternallyControlledContainers();
		mainLGL->PauseRendering(false);

		fileLoader->dllLoader.UnloadScriptDLL(dllPath);
	}
}

template<typename... Type>
void EverettEngine::ExecuteVectorOfFuncs(const std::vector<std::function<void(Type...)>>& vectOfFuncs, Type... value)
{
	for (auto& func : vectOfFuncs)
	{
		func(std::forward<Type>(value)...);
	}
}

template<typename FunctionType, typename... Params>
void EverettEngine::ExecuteFuncForAllSimObjects(FunctionType func, Params&&... values)
{
	static_assert(
		std::is_base_of_v<ObjectSim, typename stdEx::MemberFuncInfo<FunctionType>::ClassType>, 
		"Must accept ObjectSim or derived member func"
	);

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

std::generator<EverettStructs::BasicFileInfo> EverettEngine::GetLoadedScriptDLLs()
{
	return fileLoader->dllLoader.GetLoadedScriptDlls();
}

std::generator<std::string> EverettEngine::GetObjectsInDirList(
	const std::string& path, 
	const std::vector<std::string>& fileTypes
)
{
	for (auto&& file : fileLoader->GetFilesInDir(path))
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

std::generator<std::string> EverettEngine::GetModelInDirList(std::string& path)
{
	static std::vector<std::string> fileTypes{ ".glb", ".dae", ".fbx", ".obj" };
	return GetObjectsInDirList(path, fileTypes);
}

std::generator<std::string> EverettEngine::GetSoundInDirList(std::string& path)
{
	static std::vector<std::string> fileTypes{ ".wav" };
	return GetObjectsInDirList(path, fileTypes);
}

std::string EverettEngine::GetSaveFileType()
{
	return saveFileType;
}

ObjectSim* EverettEngine::GetObjectFromMap(
	EverettEngine::ObjectTypes objectType,
	const std::string& objectName,
	bool logFail
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
		std::unreachable();
	}

#ifdef _DEBUG
	if (object)
	{
		std::cout << "Retrieved object " << objectName << " from " << object->GetThisObjectTypeNameStr() << " map\n";
	}
	else if (logFail)
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

	solids.clear();
	models.clear();
	lights.clear();
	sounds.clear();
	colliders.clear();

	ClearExternallyControlledContainers();

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

void EverettEngine::AddWorldLoadCallback(std::function<void()> callback)
{
	worldLoadCallback = callback;
}

void EverettEngine::RequestWorldLoad(const char* filePath)
{
	worldToLoad = filePath;
}

void EverettEngine::CheckAndLoadRequestedWorld()
{
	if (!worldToLoad.empty())
	{
		LoadWorldFromFile(worldToLoad);
		worldToLoad.clear();
	}
}

bool EverettEngine::SaveWorldToFile(const std::string& filePath)
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

	if (fileLoader->dllLoader.AnyDLLLoaded())
	{
		file << ("ScriptDLLs*" + SimSerializer::GetValueToSaveFrom(fileLoader->dllLoader.GetLoadedScriptDlls()) + '\n');
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

			SetupScriptDLL(CheckIfRelativePathToUse(dllPath, "scripts"));
		}
	}
	else
	{
		for (auto& [dllName, dllPath] : legacyDllInfo)
		{
			SetupScriptDLL(CheckIfRelativePathToUse(dllPath, "scripts"));
		}
	}
}

bool EverettEngine::LoadWorldFromFile(const std::string& filePath)
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

			switch (GetObjectTypeToName(objectInfo[ObjectInfoNames::ObjectType]).value_or(ObjectTypes::_SIZE))
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
				ThrowExceptionWMessage("Unexpected object type name in file");
			}
		}
	}
	GenerateShader();

	if (worldLoadCallback)
	{
		worldLoadCallback();
	}
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

			switch (GetObjectTypeToName(objectInfo[ObjectInfoNames::ObjectType]).value_or(ObjectTypes::_SIZE))
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
				ThrowExceptionWMessage("Unexpected object type name in file");
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
std::generator<std::string_view> EverettEngine::GetNameList(const std::unordered_map<std::string, Sim>& sims)
{
	for (auto& [name, _] : sims)
	{
		co_yield name;
	}
}

std::generator<std::string_view> EverettEngine::GetCreatedModels(bool getFullPaths)
{
	for (const auto& [modelName, model] : models)
	{
		if (modelName != gizmoModelName)
		{
			co_yield (getFullPaths ? model.GetModelPath() : modelName);
		}
	}
}

std::generator<std::string_view> EverettEngine::GetAllObjectTypeNames()
{
	for (auto& objectNamePair : objectTypes)
	{
		co_yield objectNamePair.nameStr;
	}
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

std::optional<EverettEngine::ObjectTypes> EverettEngine::GetObjectTypeToName(const std::string& objectName)
{
	for (auto& objectNamePair : objectTypes)
	{
		if (objectNamePair.nameStr == objectName || objectNamePair.interfaceNameStr == objectName)
		{
			return objectNamePair.nameEnum;
		}
	}

	return {};
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

std::generator<std::string_view> EverettEngine::GetNamesByObject(ObjectTypes objType, bool getAdditionalInfo)
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
		std::unreachable();
	}
}

std::generator<std::string_view> EverettEngine::GetSolidList(bool getModelNames)
{
	for (auto& [solidName, solid] : solids)
	{
		std::string modelName = solid.GetModelName();

		if (modelName != gizmoModelName)
		{
			co_yield ((getModelNames ? modelName + '.' : "") + solidName);
		}
	}
}

std::generator<std::string_view> EverettEngine::GetLightList(bool getLightTypes)
{
	auto GetLightsWType = [this]() -> std::generator<std::string_view>
	{
		for (auto& [lightName, light] : lights)
		{
			co_yield (light.GetCurrentLightType() + '.' + lightName);
		}
	};

	 return getLightTypes ? GetLightsWType() : GetNameList(lights);
}

std::generator<std::string_view> EverettEngine::GetSoundList()
{
	return GetNameList(sounds);
}

std::generator<std::string_view> EverettEngine::GetColliderList()
{
	return GetNameList(colliders);
}

std::generator<std::string_view> EverettEngine::GetLightTypeList()
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