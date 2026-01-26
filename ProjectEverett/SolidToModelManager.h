#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "LGLStructs.h"
#include "AnimSystem.h"

#include "PlaybackManager.h"

class SolidToModelManager
{
public:
	using FullModelInfo = std::pair<std::weak_ptr<LGLStructs::ModelInfo>, std::weak_ptr<AnimSystem::ModelAnim>>;

	SolidToModelManager();

	bool IsInitialized();

	void InitializeSTMM(FullModelInfo& fullModelInfoRef);

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
	
	FullModelInfo* fullModelInfoP;
};