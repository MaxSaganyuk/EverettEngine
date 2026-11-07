#include "SolidToModelManager.h"
#include "EverettExceptionInternal.h"

#include <algorithm>

SolidToModelManager::SolidToModelManager() 
	: initialized(false) {}

void SolidToModelManager::InitializeSTMM(ModelInfo& modelInfo, const std::string* modelName)
{
	modelInfoPtr = &modelInfo;
	fullModelInfoP = &modelInfoPtr->GetFullModelInfo();
	this->modelName = modelName;

	size_t meshAmount = fullModelInfoP->first.lock()->meshes.size();

	visibility = { true, meshAmount, [this]() { modelInfoPtr->RecheckIfAllRelatedSolidsAreVisible(); } };
	texRender  = { !fullModelInfoP->first.lock()->isTextureless, meshAmount };

	currentAnimationIndex = 0;
	lastAnimationTime = 0.0;
	animationSpeed = 1.0;

	initialized = true;

	if (!GetAnimationAmount())
	{
		animStates.NullifyValues();
	}
}

const std::string& SolidToModelManager::GetModelName()
{
	return *modelName;
}

void SolidToModelManager::ResetAnimationTime()
{
	lastAnimationTime = 0.0;
}

bool SolidToModelManager::IsInitialized()
{
	return initialized;
}

std::vector<std::string> SolidToModelManager::GetMeshNames()
{
	CheckIfInitialized();

	if (fullModelInfoP)
	{
		return fullModelInfoP->first.lock()->GetMeshNames();
	}

	return std::vector<std::string>{};
}

size_t SolidToModelManager::GetMeshAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->first.lock()->meshes.size();
}

void SolidToModelManager::SetModelVisibility(bool value)
{
	CheckIfInitialized();

	visibility.SetModel(value);
}

bool SolidToModelManager::GetModelVisibility()
{
	CheckIfInitialized();

	return visibility.GetModel();
}

template<typename Type>
size_t SolidToModelManager::GetIndexByName(const std::string& name, const std::vector<Type>& cont)
{
	CheckIfInitialized();

	auto iter = std::find(cont.begin(), cont.end(), name);

	if (iter != cont.end())
	{
		return iter - cont.begin();
	}

	ThrowExceptionWMessage("Index outside of container range");
}

void SolidToModelManager::SetMeshVisibility(size_t index, bool value)
{
	CheckIfInitialized();

	visibility.SetMesh(index, value);
}

void SolidToModelManager::SetMeshVisibility(const std::string& name, bool value)
{
	CheckIfInitialized();

	visibility.SetMesh(GetIndexByName(name, GetMeshNames()), value);
}

bool SolidToModelManager::GetMeshVisibility(size_t index)
{
	CheckIfInitialized();

	return visibility.GetMesh(index);
}

bool SolidToModelManager::GetMeshVisibility(const std::string& name)
{
	CheckIfInitialized();

	return visibility.GetMesh(GetIndexByName(name, GetMeshNames()));
}

glm::vec3 SolidToModelManager::GetAutoScaleVector()
{
	CheckIfInitialized();

	return fullModelInfoP->first.lock()->GetAutoScaleForModel();
}

void SolidToModelManager::SetModelDefaultColor(const glm::vec4& color)
{
	CheckIfInitialized();

	modelDefaultColor = color;
}

const glm::vec4& SolidToModelManager::GetModelDefaultColor()
{
	CheckIfInitialized();

	return modelDefaultColor;
}

float SolidToModelManager::GetMeshShininess(size_t index)
{
	CheckIfInitialized();

	return fullModelInfoP->first.lock()->meshes[index].mesh.shininess;
}

float SolidToModelManager::GetMeshShininess(const std::string& name)
{
	CheckIfInitialized();

	return GetMeshShininess(GetIndexByName(name, GetMeshNames()));
}

void SolidToModelManager::EnableMeshTextureRender(size_t index, bool value)
{
	CheckIfInitialized();
	
	texRender.SetMesh(index, value);
}

bool SolidToModelManager::IsMeshTextureRenderOn(size_t index)
{
	CheckIfInitialized();

	return texRender.GetMesh(index);
}

void SolidToModelManager::EnableMeshTextureRender(const std::string& name, bool value)
{
	CheckIfInitialized();

	texRender.SetMesh(GetIndexByName(name, GetMeshNames()), value);
}

bool SolidToModelManager::IsMeshTextureRenderOn(const std::string& name)
{
	CheckIfInitialized();

	return texRender.GetMesh(GetIndexByName(name, GetMeshNames()));
}

std::vector<std::string> SolidToModelManager::GetAnimationNames()
{
	CheckIfInitialized();

	auto& animInfoVect = fullModelInfoP->second.lock()->animInfoVect;
	std::vector<std::string> res;
	res.reserve(animInfoVect.size());

	for (auto& animInfo : animInfoVect)
	{
		res.push_back(animInfo.animName);
	}

	return res;
}

size_t SolidToModelManager::GetAnimationAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->second.lock()->animInfoVect.size();
}

void SolidToModelManager::SetAnimation(size_t index)
{
	CheckIfInitialized();

	currentAnimationIndex = index;
}

void SolidToModelManager::SetAnimation(const std::string& name)
{
	CheckIfInitialized();

	currentAnimationIndex = GetIndexByName(name, GetAnimationNames());
}

size_t SolidToModelManager::GetAnimation()
{
	CheckIfInitialized();

	return currentAnimationIndex;
}

double SolidToModelManager::GetAnimationSpeed()
{
	CheckIfInitialized();

	return animationSpeed;
}

void SolidToModelManager::SetAnimationSpeed(double speed)
{
	CheckIfInitialized();

	animationSpeed = speed;

	//auto requiredDiff = currentAnimationTime - startAnimationTime;
	//requiredDiff /= animationSpeed;
	//currentAnimationTime = std::chrono::system_clock::now();
	//startAnimationTime = currentAnimationTime - requiredDiff;
}

void SolidToModelManager::PlayAnimation(bool loop)
{
	CheckIfInitialized();

	if (!animStates.IsPlaying())
	{
		lastAnimationTime = 0.0;
	}

	animStates.Play(loop);
}

void SolidToModelManager::PauseAnimation()
{
	CheckIfInitialized();

	animStates.Pause();
}

void SolidToModelManager::StopAnimation()
{
	CheckIfInitialized();

	resetAnim = true;
	animStates.Stop();
	ResetAnimationTime();
}

bool SolidToModelManager::IsAnimationPlaying()
{
	CheckIfInitialized();

	return animStates.IsPlaying();
}

bool SolidToModelManager::IsAnimationPaused()
{
	CheckIfInitialized();

	return animStates.IsPaused();
}

bool SolidToModelManager::IsAnimationLooped()
{
	CheckIfInitialized();

	return animStates.IsLooped();
}

void SolidToModelManager::SetAnimationPlaybackCallback(std::function<void(bool, bool, bool)>&& callback)
{
	CheckIfInitialized();

	animStates.SetStateChangeCallback(std::move(callback));
}

double SolidToModelManager::GetAnimationTimeTicks(double currentTime)
{
	double animDuration = fullModelInfoP->second.lock()->animInfoVect[currentAnimationIndex].animDuration;

	double timeInTicks = 
		currentTime * animationSpeed * fullModelInfoP->second.lock()->animInfoVect[currentAnimationIndex].ticksPerSecond;

	double animationTime = std::fmod(timeInTicks, animDuration);

	if (!animStates.IsLooped() && animationTime < lastAnimationTime)
	{
		StopAnimation();
		animationTime = lastAnimationTime;
	}

	lastAnimationTime = animationTime;

	return animationTime;
}

double SolidToModelManager::GetCurrentAnimationTime()
{
	CheckIfInitialized();

	return GetAnimationTimeTicks(animStates.GetCurrentTime());
}

void SolidToModelManager::SetStartBoneIndexRef(AutoIndex startingBoneIndex)
{
	CheckIfInitialized();

	startBoneIndexPtr = startingBoneIndex;
}

size_t SolidToModelManager::GetCurrentStartingBoneIndex()
{
	CheckIfInitialized();

	return startBoneIndexPtr;
}

size_t SolidToModelManager::GetModelBoneAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->second.lock()->boneAmount;
}

bool SolidToModelManager::IsAnimationResetRequired()
{
	CheckIfInitialized();

	if (resetAnim)
	{
		resetAnim = false;
		return true;
	}

	return false;
}

void SolidToModelManager::EnableModelTextureRender(bool value)
{
	CheckIfInitialized();

	texRender.SetModel(value);
}

bool SolidToModelManager::IsModelTextureRenderOn()
{
	CheckIfInitialized();

	return texRender.GetModel();
}

void SolidToModelManager::CheckIfInitialized()
{
	CheckAndThrowExceptionWMessage(initialized, "SolidToModelManager is uninitialized");
}

SolidToModelManager::ModelAndMesh::ModelAndMesh(
	bool value, size_t meshAmount, std::function<void()> triggerOnBoolChange
)
{
	model = value;
	meshes.resize(meshAmount, model);

	this->triggerOnBoolChange = std::move(triggerOnBoolChange);
}

void SolidToModelManager::ModelAndMesh::SetModel(bool value)
{
	bool previous = model;
	model = value;
	std::fill(meshes.begin(), meshes.end(), model);

	if (triggerOnBoolChange && previous != model)
	{
		triggerOnBoolChange();
	}
}

bool SolidToModelManager::ModelAndMesh::GetModel() const
{
	return model;
}

void SolidToModelManager::ModelAndMesh::SetMesh(size_t index, bool value)
{
	CheckAndThrowExceptionWMessage((index < meshes.size()), "Mesh index is invalid for this solid");

	bool previous = model;
	meshes[index] = value;
	RecalcModelFromMeshes();

	if (triggerOnBoolChange && previous != model)
	{
		triggerOnBoolChange();
	}
}

bool SolidToModelManager::ModelAndMesh::GetMesh(size_t index) const
{
	CheckAndThrowExceptionWMessage((index < meshes.size()), "Mesh index is invalid for this solid");

	return meshes[index];
}

void SolidToModelManager::ModelAndMesh::RecalcModelFromMeshes()
{
	model = std::any_of(meshes.begin(), meshes.end(), [](bool val) { return val; });
}

std::vector<bool>& SolidToModelManager::ModelAndMesh::GetMeshVect() noexcept
{
	return meshes;
}
