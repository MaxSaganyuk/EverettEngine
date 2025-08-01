#include "AnimSystem.h"
#include "LGL.h"

void AnimSystem::InterpolateImpl(const glm::vec3& vec1, const glm::vec3& vec2, glm::vec3& resVec, float factor)
{
	resVec = glm::mix(vec1, vec2, factor);
}

void AnimSystem::InterpolateImpl(const glm::quat& quat1, const glm::quat& quat2, glm::quat& resQuat, float factor)
{
	resQuat = glm::slerp(quat1, quat2, factor);
}

template<typename GLMType>
void AnimSystem::InterpolateKey(std::vector<std::pair<double, GLMType>>& keys, GLMType& res, double animTime)
{
	size_t keyAmount = keys.size();

	if (keyAmount)
	{
		for (size_t i = 0; i < keyAmount - 1; ++i)
		{
			if (animTime < keys[i + 1].first)
			{
				double t1 = keys[i].first;
				double t2 = keys[i + 1].first;
				float factor = static_cast<float>((animTime - t1) / (t2 - t1));

				InterpolateImpl(keys[i].second, keys[i + 1].second, res, factor);

				return;
			}
		}

		res = keys.back().second;
	}
}

void AnimSystem::ParseBoneTree(
	BoneTree::TreeManagerNode* currentBoneNode,
	double animationTimeTicks,
	size_t animIndex,
	glm::mat4& globalInverseTransform,
	AnimKeyMap& animKeyMap
)
{
	BoneInfo& currentBone = currentBoneNode->GetValue();
	auto& nameKeyPair = animKeyMap[currentBoneNode->GetKey()];

	glm::mat4 nodeTransformation = currentBone.localTransform;

	if (!nameKeyPair.empty() && nameKeyPair[animIndex].KeysExist())
	{
		glm::vec3 interpolPos(1.0, 1.0, 1.0);
		glm::quat interpolRot(1.0, 1.0, 1.0, 1.0);
		glm::vec3 interpolSca(1.0, 1.0, 1.0);

		InterpolateKey(nameKeyPair[animIndex].positionKeys, interpolPos, animationTimeTicks);
		InterpolateKey(nameKeyPair[animIndex].rotationKeys, interpolRot, animationTimeTicks);
		InterpolateKey(nameKeyPair[animIndex].scalingKeys, interpolSca, animationTimeTicks);

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), interpolPos);
		glm::mat4 rotation = glm::mat4_cast(interpolRot);
		glm::mat4 scaling = glm::scale(glm::mat4(1.0f), interpolSca);

		nodeTransformation = translation * rotation * scaling;
	}

	BoneTree::TreeManagerNode* parentBoneNode = currentBoneNode->GetParentNode();
	glm::mat4 parentTransformMatrix = parentBoneNode ? parentBoneNode->GetValue().globalTransform : glm::mat4(1.0f);

	currentBone.globalTransform = parentTransformMatrix * nodeTransformation;

	if (currentBone.id != -1)
	{
		currentBone.finalTransform = globalInverseTransform * currentBone.globalTransform * currentBone.offsetMatrix;
	}

	for (auto& [childNodeName, childNode] : currentBoneNode->GetChildNodes())
	{
		ParseBoneTree(childNode, animationTimeTicks, animIndex, globalInverseTransform, animKeyMap);
	}
}

void AnimSystem::CollectAllFinalTransforms(
	BoneTree::TreeManagerNode* boneTreeNode,
	size_t startingBoneIndex,
	std::vector<glm::mat4>& finalTransforms
)
{
	int currentID = boneTreeNode->GetValue().id;

	if (currentID != -1)
	{
		finalTransforms[startingBoneIndex + currentID] = boneTreeNode->GetValue().finalTransform;
	}

	for (auto& [childNodeName, childNode] : boneTreeNode->GetChildNodes())
	{
		CollectAllFinalTransforms(childNode, startingBoneIndex, finalTransforms);
	}
}

void AnimSystem::ProcessAnimations(ModelAnim& modelAnim, double animationTimeTicks, size_t animIndex, size_t startingBoneIndex)
{
	if (!modelAnim.animInfoVect.empty())
	{
		for (auto& [childNodeName, childNode] : modelAnim.boneTree.GetChildNodes())
		{
			ParseBoneTree(childNode, animationTimeTicks, animIndex, modelAnim.globalInverseTransform, modelAnim.animKeyMap);
		}

		for (auto& [childNodeName, childNode] : modelAnim.boneTree.GetChildNodes())
		{
			CollectAllFinalTransforms(childNode, startingBoneIndex, finalTransforms);
		}
	}
}

std::vector<glm::mat4>& AnimSystem::GetFinalTransforms()
{
	return finalTransforms;
}

void AnimSystem::ResetFinalTransforms()
{
	finalTransforms = std::vector<glm::mat4>(GetTotalBoneAmount(), glm::mat4(1.0f));
}

void AnimSystem::IncrementTotalBoneAmount(ModelAnim& modelAnim)
{
	totalBoneAmount += modelAnim.boneAmount;
}

size_t AnimSystem::GetTotalBoneAmount()
{
	return totalBoneAmount;
}
