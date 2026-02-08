#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "LGLStructs.h"
#include "AnimSystem.h"

#include "PlaybackManager.h"
#include "FullModelInfoType.h"

class SolidToModelManager
{
public:
	SolidToModelManager();

	bool IsInitialized();

	void InitializeSTMM(std::weak_ptr<FullModelInfo> fullModelInfoRef);

	std::vector<std::string> GetMeshNames();
	size_t GetMeshAmount();
	void SetAllMeshVisibility(bool value);
	void SetMeshVisibility(size_t intex, bool value);
	void SetMeshVisibility(const std::string& name, bool value);
	bool GetMeshVisibility(size_t intex);
	bool GetMeshVisibility(const std::string& name);
	float GetMeshShininess(size_t index);
	float GetMeshShininess(const std::string& name);

	std::vector<std::string> GetAnimationNames();
	size_t GetAnimationAmount();
	void SetAnimation(size_t index);
	void SetAnimation(const std::string& name);
	size_t GetAnimation();
	double GetAnimationSpeed();
	void SetAnimationSpeed(double speed);
	void PlayAnimation(bool loop = false);
	void PauseAnimation();
	void StopAnimation();
	bool IsAnimationPlaying();
	bool IsAnimationPaused();
	bool IsAnimationLooped();
	void SetAnimationPlaybackCallback(std::function<void(bool, bool, bool)> callback);

	double GetCurrentAnimationTime();
	void AppendStartingBoneIndex(size_t startingBoneIndex);
	size_t GetCurrentStartingBoneIndex();
private:
	friend class SolidSim;

	void CheckIfInitialized();

	double GetAnimationTimeTicks(double currentTime);
	void ResetAnimationTime();

	template<typename Type>
	size_t GetIndexByName(const std::string& name, const std::vector<Type>& cont);

	bool initialized;

	double animationSpeed;
	double lastAnimationTime;
	size_t currentAnimationIndex;
	std::vector<size_t> startingBoneIndexes;
	PlaybackManager animStates;

	std::vector<bool> meshVisibility;
	
	std::weak_ptr<FullModelInfo> fullModelInfoP;
};