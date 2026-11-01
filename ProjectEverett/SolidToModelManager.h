#pragma once

#include <vector>
#include <string>

#include "ModelInfo.h"
#include "PlaybackManager.h"

#include "external/ColorManager.h"
#include "AutoReindexer.h"

class SolidToModelManager
{
public:
	SolidToModelManager();

	bool IsInitialized();

	void InitializeSTMM(ModelInfo& modelInfo, const std::string* modelName);

	const std::string& GetModelName();

	std::vector<std::string> GetMeshNames();
	size_t GetMeshAmount();
	void SetMeshVisibility(size_t index, bool value);
	void SetMeshVisibility(const std::string& name, bool value);
	bool GetMeshVisibility(size_t index);
	bool GetMeshVisibility(const std::string& name);
	float GetMeshShininess(size_t index);
	float GetMeshShininess(const std::string& name);
	void EnableMeshTextureRender(size_t index, bool value);
	bool IsMeshTextureRenderOn(size_t index);
	void EnableMeshTextureRender(const std::string& name, bool value);
	bool IsMeshTextureRenderOn(const std::string& name);

	glm::vec3 GetAutoScaleVector();

	void SetModelVisibility(bool value);
	bool GetModelVisibility();
	void SetModelDefaultColor(const glm::vec4& color);
	const glm::vec4& GetModelDefaultColor();
	void EnableModelTextureRender(bool value = true);
	bool IsModelTextureRenderOn();

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
	void SetAnimationPlaybackCallback(std::function<void(bool, bool, bool)>&& callback);

	double GetCurrentAnimationTime();
	void SetStartBoneIndexRef(AutoIndex startingBoneIndex);
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
	const std::string* modelName;
	double animationSpeed;
	double lastAnimationTime;
	size_t currentAnimationIndex;
	PlaybackManager animStates;

	class ModelAndMesh
	{
		bool model{};
		std::vector<bool> meshes;
		std::function<void()> triggerOnBoolChange;
	public:
		ModelAndMesh() = default;
		ModelAndMesh(bool value, size_t meshAmount, std::function<void()> triggerOnBoolChange = nullptr);

		ModelAndMesh(const ModelAndMesh&) = default;
		ModelAndMesh& operator=(const ModelAndMesh&) = default;
		ModelAndMesh(ModelAndMesh&&) = default;
		ModelAndMesh& operator=(ModelAndMesh&&) = default;

		void SetModel(bool value);
		bool GetModel() const;
		void SetMesh(size_t index, bool value);
		bool GetMesh(size_t index) const;

		void RecalcModelFromMeshes();
		std::vector<bool>& GetMeshVect() noexcept;
	};

	ModelAndMesh visibility;
	ModelAndMesh texRender;
	glm::vec4 modelDefaultColor = ColorManager::GetColorVec4(ColorManager::Colors::WHITE);

	AutoIndex startBoneIndexPtr;
	
	ModelInfo* modelInfoPtr;
	ModelInfo::FullModelInfo* fullModelInfoP;
};