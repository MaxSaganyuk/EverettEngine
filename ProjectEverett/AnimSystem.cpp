#include "AnimSystem.h"
#include "SolidSim.h"

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

void AnimSystem::FinalTransformAssignIdentity(
	BoneTree::TreeManagerNode* boneTreeNode, size_t startBoneIndex, int currentID
)
{
	finalTransforms[startBoneIndex + currentID] = glm::mat4(1.0f);
}

void AnimSystem::FinalTransformAssignData(BoneTree::TreeManagerNode* boneTreeNode, size_t startBoneIndex, int currentID)
{
	finalTransforms[startBoneIndex + currentID] = boneTreeNode->GetValue().finalTransform;
}

void AnimSystem::AssignFinalTransforms(
	BoneTree::TreeManagerNode* boneTreeNode, size_t startingBoneIndex, FinalTransformAssigner assigner
)
{
	int currentID = boneTreeNode->GetValue().id;

	if (currentID != -1)
	{
		(this->*assigner)(boneTreeNode, startingBoneIndex, currentID);
	}

	for (auto& [childNodeName, childNode] : boneTreeNode->GetChildNodes())
	{
		AssignFinalTransforms(childNode, startingBoneIndex, assigner);
	}
}

void AnimSystem::ProcessAnimations(ModelAnim& modelAnim, SolidSim& solid)
{
	if (!modelAnim.animInfoVect.empty())
	{
		// On Play - parse and assign data, on pause - ignore all, on stop - assign identity mats only once (singular reset)
		bool isPlaying = solid.IsModelAnimationPlaying();
		bool isPaused = solid.IsModelAnimationPaused();

		if (!isPaused || (!isPlaying && solid.IsModelAnimationResetRequired()))
		{
			if (!isPaused)
			{
				for (auto& [childNodeName, childNode] : modelAnim.boneTree.GetChildNodes())
				{
					ParseBoneTree(
						childNode, solid.GetModelCurrentAnimationTime(), solid.GetModelAnimation(),
						modelAnim.globalInverseTransform, modelAnim.animKeyMap
					);
				}
			}

			// This avoids rechecking for required form of assignment (either 1.0f or real data) on every call
			// (No "reset ? glm::mat4(1.0f) : boneTreeNode->GetValue().finalTransform;")
			// And avoids creating a duplicate funcs for recursive logic
			ExecuteAssignFinalTransforms(
				modelAnim.boneTree, solid.GetModelCurrentStartingBoneIndex(),
				isPlaying ? &AnimSystem::FinalTransformAssignData : &AnimSystem::FinalTransformAssignIdentity
			);
		}
	}
}

void AnimSystem::ExecuteAssignFinalTransforms(
	BoneTree& boneTree, size_t startingBoneIndex, FinalTransformAssigner assigner
)
{
	for (auto& [childNodeName, childNode] : boneTree.GetChildNodes())
	{
		AssignFinalTransforms(childNode, startingBoneIndex, assigner);
	}
}

std::vector<glm::mat4>& AnimSystem::GetFinalTransforms()
{
	return finalTransforms;
}

void AnimSystem::AppendToFinalTransforms(size_t amount)
{
	finalTransforms.insert(finalTransforms.end(), amount, glm::mat4(1.0f));
}

void AnimSystem::CutAndGlueFinalTransforms(size_t startPoint, size_t amount)
{
	std::vector<glm::mat4> finalTransformSubstitute;

	finalTransformSubstitute.insert(
		finalTransformSubstitute.end(), finalTransforms.begin(), finalTransforms.begin() + startPoint
	);
	finalTransformSubstitute.insert(
		finalTransformSubstitute.end(), finalTransforms.begin() + startPoint + amount, finalTransforms.end()
	);

	finalTransforms = std::move(finalTransformSubstitute);
}

void AnimSystem::CutAndRecalcStartBoneIndexes(size_t startBoneIndex, size_t boneAmount)
{
	bool indexFound = false;

	for (auto iter = startBoneIndexes.begin(); iter != startBoneIndexes.end();)
	{
		if (indexFound)
		{
			(*iter) -= boneAmount;
		}

		if (!indexFound && *iter == startBoneIndex)
		{
			iter = startBoneIndexes.erase(iter);
			indexFound = true;
		}
		else
		{
			++iter;
		}
	}
}

void AnimSystem::IncrementTotalBoneAmount(ModelAnim& modelAnim)
{
	startBoneIndexes.push_back(finalTransforms.size());

	AppendToFinalTransforms(modelAnim.boneAmount);
}

void AnimSystem::DecrementTotalBoneAmount(SolidSim& solid)
{
	size_t currentSolidStartBoneIndex = solid.GetModelCurrentStartingBoneIndex();
	size_t modelBoneAmount = solid.GetModelBoneAmount();

	CutAndGlueFinalTransforms(currentSolidStartBoneIndex, modelBoneAmount);
	CutAndRecalcStartBoneIndexes(currentSolidStartBoneIndex, modelBoneAmount);
}

size_t AnimSystem::GetTotalBoneAmount()
{
	return finalTransforms.size();
}

const size_t& AnimSystem::GetLastStartBoneIndexRef()
{
	return startBoneIndexes.back();
}
