#include "SolidToModelManager.h"

SolidToModelManager::SolidToModelManager() 
	: initialized(false) {}

void SolidToModelManager::InitializeSTMM(FullModelInfo& fullModelInfoRef)
{
	fullModelInfoP = &fullModelInfoRef;
	meshVisibility.resize(fullModelInfoP->first.meshes.size());
	std::fill(meshVisibility.begin(), meshVisibility.end(), true);
	currentAnimationIndex = 0;

	initialized = true;
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
	return fullModelInfoP->second.animInfoVect.size();
}

void SolidToModelManager::SetAnimation(size_t index)
{
	currentAnimationIndex = index;
}

void SolidToModelManager::SetAnimation(const std::string& name)
{
	currentAnimationIndex = GetIndexByName(name, GetAnimationNames());
}

size_t SolidToModelManager::GetAnimation()
{
	return currentAnimationIndex;
}

void SolidToModelManager::CheckIfInitialized()
{
	assert(initialized && "SolidToModelManager is uninitialized");
}