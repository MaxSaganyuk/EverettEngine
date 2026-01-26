#include "SolidToModelManager.h"
#include "EverettException.h"

SolidToModelManager::SolidToModelManager() 
	: initialized(false) {}

void SolidToModelManager::InitializeSTMM(FullModelInfo& fullModelInfoRef)
{
	fullModelInfoP = &fullModelInfoRef;
	meshVisibility.resize(fullModelInfoP->first.lock()->meshes.size());
	std::fill(meshVisibility.begin(), meshVisibility.end(), true);
	currentAnimationIndex = 0;
	lastAnimationTime = 0.0;
	animationSpeed = 1.0;

	initialized = true;

	if (!GetAnimationAmount())
	{
		animStates.NullifyValues();
	}
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

	return std::vector<std::string>();
}

size_t SolidToModelManager::GetMeshAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->first.lock()->meshes.size();
}

void SolidToModelManager::SetAllMeshVisibility(bool value)
{
	CheckIfInitialized();

	std::fill(meshVisibility.begin(), meshVisibility.end(), value);
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
	return 0;
}

void SolidToModelManager::SetMeshVisibility(size_t index, bool value)
{
	CheckIfInitialized();

	meshVisibility[index] = value;
}

void SolidToModelManager::SetMeshVisibility(const std::string& name, bool value)
{
	CheckIfInitialized();

	if (fullModelInfoP)
	{
		meshVisibility[GetIndexByName(name, GetMeshNames())] = value;
	}
}

bool SolidToModelManager::GetMeshVisibility(size_t index)
{
	CheckIfInitialized();

	return meshVisibility[index];
}

bool SolidToModelManager::GetMeshVisibility(const std::string& name)
{
	CheckIfInitialized();

	return meshVisibility[GetIndexByName(name, GetMeshNames())];
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

std::vector<std::string> SolidToModelManager::GetAnimationNames()
{
	CheckIfInitialized();

	std::vector<std::string> res;

	for (auto& animInfo : fullModelInfoP->second.lock()->animInfoVect)
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
	return animationSpeed;
}

void SolidToModelManager::SetAnimationSpeed(double speed)
{
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

void SolidToModelManager::SetAnimationPlaybackCallback(std::function<void(bool, bool, bool)> callback)
{
	animStates.SetStateChangeCallback(callback);
}

double SolidToModelManager::GetAnimationTimeTicks(double currentTime)
{
	double animDuration = fullModelInfoP->second.lock()->animInfoVect[currentAnimationIndex].animDuration;

	double timeInTicks = 
		currentTime * animationSpeed * fullModelInfoP->second.lock()->animInfoVect[currentAnimationIndex].ticksPerSecond;

	double animationTime = std::fmod(timeInTicks, animDuration);

	if (!animStates.IsLooped() && animationTime < lastAnimationTime)
	{
		timeInTicks = lastAnimationTime = 0.0;
		animStates.Stop();
	}

	lastAnimationTime = animationTime;

	return animationTime;
}

double SolidToModelManager::GetCurrentAnimationTime()
{
	CheckIfInitialized();

	return GetAnimationTimeTicks(animStates.GetCurrentTime());
}

void SolidToModelManager::AppendStartingBoneIndex(size_t staringBoneIndex)
{
	CheckIfInitialized();

	startingBoneIndexes.push_back(staringBoneIndex);
}

size_t SolidToModelManager::GetCurrentStartingBoneIndex()
{
	CheckIfInitialized();

	return startingBoneIndexes[currentAnimationIndex];
}

void SolidToModelManager::CheckIfInitialized()
{
	CheckAndThrowExceptionWMessage(initialized, "SolidToModelManager is uninitialized");
}

