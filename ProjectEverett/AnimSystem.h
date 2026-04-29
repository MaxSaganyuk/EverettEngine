#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>
#include <string>
#include <unordered_map>

#include "TreeManager.h"

class SolidSim;

class AnimSystem
{
public:
	struct BoneInfo
	{
		int id = -1;
		glm::mat4 offsetMatrix = glm::mat4(1.0f);
		glm::mat4 localTransform = glm::mat4(1.0f);
		glm::mat4 globalTransform = glm::mat4(1.0f);
		glm::mat4 finalTransform = glm::mat4(1.0f);
	};

	struct AnimInfo
	{
		constexpr static double defaultTicksPerSecond = 25;

		std::string animName;
		double animDuration;
		double ticksPerSecond;

		AnimInfo(const std::string& animName, double animDuration, double ticksPerSecond)
			:
			animName(animName),
			animDuration(animDuration),
			ticksPerSecond(!ticksPerSecond ? defaultTicksPerSecond : ticksPerSecond) {}
	};

	struct AnimKeys
	{
		std::vector<std::pair<double, glm::vec3>> positionKeys;
		std::vector<std::pair<double, glm::quat>> rotationKeys;
		std::vector<std::pair<double, glm::vec3>> scalingKeys;

		bool KeysExist()
		{
			return !(positionKeys.empty() && rotationKeys.empty() && scalingKeys.empty());
		}
	};

	using BoneTree = TreeManager<std::string, BoneInfo>;
	using AnimKeyMap = std::unordered_map<std::string, std::vector<AnimKeys>>;
	using AnimInfoVect = std::vector<AnimInfo>;

	struct ModelAnim
	{
		size_t boneAmount;
		BoneTree boneTree;
		AnimKeyMap animKeyMap;
		AnimInfoVect animInfoVect;
		glm::mat4 globalInverseTransform = glm::mat4(1.0f);
	};

	void ProcessAnimations(ModelAnim& modelAnim, SolidSim& solid);
	std::vector<glm::mat4>& GetFinalTransforms();
	void IncrementTotalBoneAmount(ModelAnim& modelAnim);
	void DecrementTotalBoneAmount(SolidSim& solid);
	size_t GetTotalBoneAmount();
	const size_t& GetLastStartBoneIndexRef();
private:
	template<typename GLMType>
	void InterpolateKey(std::vector<std::pair<double, GLMType>>& keys, GLMType& res, double animTime);
	void InterpolateImpl(const glm::vec3& vec1, const glm::vec3& vec2, glm::vec3& resVec, float factor);
	void InterpolateImpl(const glm::quat& quat1, const glm::quat& quat2, glm::quat& resQuat, float factor);
	void ParseBoneTree(
		BoneTree::TreeManagerNode* currentBoneNode,
		double animationTimeTicks,
		size_t animIndex,
		glm::mat4& globalInverseTransform,
		AnimKeyMap& animKeyMap
	);

	using FinalTransformAssigner = void(AnimSystem::*)(BoneTree::TreeManagerNode*, size_t, int);

	void FinalTransformAssignIdentity(BoneTree::TreeManagerNode* boneTreeNode, size_t startBoneIndex, int currentID);
	void FinalTransformAssignData(BoneTree::TreeManagerNode* boneTreeNode, size_t startBoneIndex, int currentID);

	void ExecuteAssignFinalTransforms(BoneTree& boneTree, size_t startingBoneIndex, FinalTransformAssigner assigner);
	void AssignFinalTransforms(
		BoneTree::TreeManagerNode* boneTreeNode, size_t startingBoneIndex, FinalTransformAssigner assigner
	);
	void AppendToFinalTransforms(size_t amount);
	void CutAndGlueFinalTransforms(size_t startPoint, size_t amount);
	void CutAndRecalcStartBoneIndexes(size_t startBoneIndex, size_t boneAmount);

	std::vector<glm::mat4> finalTransforms;
	std::list<size_t> startBoneIndexes;
};