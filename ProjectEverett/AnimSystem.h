#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>
#include <string>
#include <unordered_map>

#include "TreeManager.h"

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

	void ProcessAnimations(ModelAnim& modelAnim, double currentTime, size_t animIndex, size_t startingBoneIndex);
	std::vector<glm::mat4>& GetFinalTransforms();
	void ResetFinalTransforms();
	void IncrementTotalBoneAmount(ModelAnim& modelAnim);
	size_t GetTotalBoneAmount();
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
	void CollectAllFinalTransforms(
		BoneTree::TreeManagerNode* boneTreeNode,
		size_t startingBoneIndex,
		std::vector<glm::mat4>& finalTransforms
	);

	std::vector<glm::mat4> finalTransforms;

	size_t totalBoneAmount = 0;
};