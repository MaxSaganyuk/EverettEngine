#pragma once

#include <vector>
#include <string>
#include <chrono>

#include "ModelInfo.h"
#include "PlaybackManager.h"

class SolidToModelManager
{
public:
	SolidToModelManager();

	bool IsInitialized();

	void InitializeSTMM(ModelInfo::FullModelInfo& fullModelInfoRef, const std::string& modelName);

	std::string GetModelName();

	std::vector<std::string> GetMeshNames();
	size_t GetMeshAmount();
	void SetAllMeshVisibility(bool value);
	void SetMeshVisibility(size_t intex, bool value);
	void SetMeshVisibility(const std::string& name, bool value);
	bool GetMeshVisibility(size_t intex);
	bool GetMeshVisibility(const std::string& name);
	float GetMeshShininess(size_t index);
	float GetMeshShininess(const std::string& name);

	glm::vec3 GetAutoScaleVector();

	void SetModelDefaultColor(const glm::vec4& color);
	glm::vec4 GetModelDefaultColor();

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
	void SetStartBoneIndexRef(const size_t& startingBoneIndex);
	size_t GetCurrentStartingBoneIndex();
	size_t GetModelBoneAmount();
	bool IsAnimationResetRequired();
private:
	friend class SolidSim;

	void CheckIfInitialized();

	double GetAnimationTimeTicks(double currentTime);
	void ResetAnimationTime();

	template<typename Type>
	size_t GetIndexByName(const std::string& name, const std::vector<Type>& cont);

	bool initialized;

	bool resetAnim{};
	std::string modelName;
	double animationSpeed;
	double lastAnimationTime;
	size_t currentAnimationIndex;
	PlaybackManager animStates;

	std::vector<bool> meshVisibility;
	glm::vec4 modelDefaultColor;

	const size_t* startBoneIndexPtr;
	
	ModelInfo::FullModelInfo* fullModelInfoP;
};