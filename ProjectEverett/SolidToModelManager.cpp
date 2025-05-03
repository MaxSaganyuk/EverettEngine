#include "SolidToModelManager.h"

SolidToModelManager::SolidToModelManager() 
	: initialized(false) {}

void SolidToModelManager::InitializeSTMM(FullModelInfo& fullModelInfoRef)
{
	fullModelInfoP = &fullModelInfoRef;
	meshVisibility.resize(fullModelInfoP->first.meshes.size());
	std::fill(meshVisibility.begin(), meshVisibility.end(), true);
	currentAnimationIndex = 0;
	lastAnimationTime = 0.0;

	initialized = true;
}

void SolidToModelManager::ResetAnimationTime()
{
	lastAnimationTime = 0.0;
	startAnimationTime = std::chrono::system_clock::now();
	currentAnimationTime = startAnimationTime;
}

std::vector<std::string> SolidToModelManager::GetMeshNames()
{
	CheckIfInitialized();

	if (fullModelInfoP)
	{
		return fullModelInfoP->first.GetMeshNames();
	}

	return std::vector<std::string>();
}

size_t SolidToModelManager::GetMeshAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->first.meshes.size();
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

	assert(false && "Index outside of container range");
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

std::vector<std::string> SolidToModelManager::GetAnimationNames()
{
	CheckIfInitialized();

	std::vector<std::string> res;

	for (auto& animInfo : fullModelInfoP->second.animInfoVect)
	{
		res.push_back(animInfo.animName);
	}

	return res;
}

size_t SolidToModelManager::GetAnimationAmount()
{
	CheckIfInitialized();

	return fullModelInfoP->second.animInfoVect.size();
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

void SolidToModelManager::PlayAnimation(bool loop)
{
	CheckIfInitialized();

	if (!animStates.playing)
	{
		lastAnimationTime = 0.0;
		startAnimationTime = std::chrono::system_clock::now();
	}
	else if (animStates.paused)
	{
		auto requiredDiff = currentAnimationTime - startAnimationTime;
		currentAnimationTime = std::chrono::system_clock::now();
		startAnimationTime = currentAnimationTime - requiredDiff;
	}

	animStates.playing = true;
	animStates.paused = false;
	animStates.looped = loop;
}

void SolidToModelManager::PauseAnimation()
{
	CheckIfInitialized();

	animStates.paused = true;
}

void SolidToModelManager::StopAnimation()
{
	CheckIfInitialized();

	animStates.ResetValues();
	ResetAnimationTime();
}

bool SolidToModelManager::IsAnimationPlaying()
{
	CheckIfInitialized();

	return animStates.playing;
}

bool SolidToModelManager::IsAnimationPaused()
{
	CheckIfInitialized();

	return animStates.paused;
}

double SolidToModelManager::GetAnimationTimeTicks(double currentTime)
{
	double animDuration = fullModelInfoP->second.animInfoVect[currentAnimationIndex].animDuration;

	double timeInTicks = currentTime * fullModelInfoP->second.animInfoVect[currentAnimationIndex].ticksPerSecond;

	double animationTime = std::fmod(timeInTicks, animDuration);

	if (!animStates.looped && animationTime < lastAnimationTime)
	{
		timeInTicks = lastAnimationTime = 0.0;
		animStates.ResetValues();
		ResetAnimationTime();
	}

	lastAnimationTime = animationTime;

	return animationTime;
}

double SolidToModelManager::GetCurrentAnimationTime()
{
	CheckIfInitialized();

	if (!animStates.paused)
	{
		currentAnimationTime = std::chrono::system_clock::now();
	}

	return GetAnimationTimeTicks(std::chrono::duration<double>(currentAnimationTime - startAnimationTime).count());
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
	assert(initialized && "SolidToModelManager is uninitialized");
}

