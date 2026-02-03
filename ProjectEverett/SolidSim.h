#pragma once

#include "ObjectSim.h"

#include <cassert>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <functional>

#include "SolidToModelManager.h"

#include "interfaces/ISolidSim.h"

class SolidSim : public ObjectSim, virtual public ISolidSim
{
private:
	glm::mat4 model;
	SolidType type;
	SolidToModelManager STMM;

	void ResetModelMatrix();
	std::string CollectInfoToSaveFromSTMM();
	bool CollectInfoToLoadToSTMM(std::string_view& line);
protected:
	std::string GetSimInfoToSaveImpl();
public:
	SolidSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 1.0f
	);

	static std::string GetObjectTypeNameStr();

	std::string GetSimInfoToSave(const std::string& modelSolidName);
	bool SetSimInfoToLoad(std::string_view& line);

	glm::mat4& GetModelMatrixAddr() override;
	void ForceModelUpdate() override;
	void SetType(SolidType type) override;
	void SetPosition(ObjectSim::Direction dir, const glm::vec3& limitAxis, bool executeLinkedObjects = true) override;
	void Rotate(const Rotation& toRotate, bool executeLinkedObjects = true) override;

	void EnableAutoModelUpdates(bool value = true) override;
	
	// Solid to model access section
	// Mesh access; avalible through interface
	void SetBackwardsModelAccess(SolidToModelManager::FullModelInfo& model);
	std::vector<std::string> GetModelMeshNames() override;
	size_t GetMeshAmount() override;
	void SetAllMeshVisibility(bool value) override;
	void SetModelMeshVisibility(const std::string name, bool value) override;
	void SetModelMeshVisibility(size_t index, bool value) override;
	bool GetModelMeshVisibility(const std::string name) override;
	bool GetModelMeshVisibility(size_t index) override;

	// Mesh access; engine only
	float GetModelMeshShininess(const std::string& name);
	float GetModelMeshShininess(size_t index);

	// Animation access; avalible through interface
	std::vector<std::string> GetModelAnimationNames() override;
	size_t GetModelAnimationAmount() override;
	void SetModelAnimation(size_t index) override;
	void SetModelAnimation(const std::string& name) override;
	size_t GetModelAnimation() override;
	double GetModelAnimationSpeed() override;
	void SetModelAnimationSpeed(double speed) override;
	void PlayModelAnimation(bool loop = false) override;
	void PauseModelAnimation() override;
	void StopModelAnimation() override;
	bool IsModelAnimationPlaying() override;
	bool IsModelAnimationPaused() override;
	bool IsModelAnimationLooped() override;
	void SetModelAnimationPlaybackCallback(std::function<void(bool, bool, bool)> callback) override;

	// Animation access; engine only
	double GetModelCurrentAnimationTime();
	void AppendModelStartingBoneIndex(size_t startingBoneIndex);
	size_t GetModelCurrentStartingBoneIndex();
	
	static bool CheckForCollision(const SolidSim& solid1, const SolidSim& solid2);
	bool CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2) override;
};