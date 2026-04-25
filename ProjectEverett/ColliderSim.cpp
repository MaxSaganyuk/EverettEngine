#include "ColliderSim.h"

#include <algorithm>
#include <numbers>

ColliderSim::ColliderSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	float speed
)
	: ObjectSim(pos, scale, speed), isCollided(false), isActive(true)
{
	AppendToSortedVectorOfColliders();
}

ColliderSim::~ColliderSim()
{
	DeleteFromSortedVectorOfColliders();
}

std::string ColliderSim::GetObjectTypeNameStr()
{
	return TypeName;
}

std::string ColliderSim::GetThisObjectTypeNameStr()
{
	return TypeName;
}

std::string ColliderSim::GetSimInfoToSaveImpl()
{
	std::string res = ObjectSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(isActive);

	return res;
}

std::string ColliderSim::GetSimInfoToSave(const std::string& colliderName)
{
	std::string info = GetObjectTypeNameStr() + "**" + colliderName + '*';

	info += GetSimInfoToSaveImpl();

	return info + '\n';
}

bool ColliderSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = ObjectSim::SetSimInfoToLoad(line);

	res = res && SimSerializer::SetValueToLoadFrom(line, isActive, 9);

	return res;
}

ColliderSim::CollisionSet ColliderSim::SetDifference(
	const CollisionSet& firstSet, const CollisionSet& secondSet
)
{
	CollisionSet resSet;

	std::set_difference(
		firstSet.begin(), firstSet.end(), secondSet.begin(), secondSet.end(), std::inserter(resSet, resSet.begin())
	);

	return resSet;
}

void ColliderSim::ExecuteBroadCollisionCheck()
{
	// For a rotated box, max possible maxMin is full diagonal length (45 degree rotation)
	constexpr float maxPossibleMaxMin = 0.5f * std::numbers::sqrt2_v<float>;

	size_t amountOfColliders = collidersByAxis.size();

	std::vector<bool> currentGeneralCollisonState(amountOfColliders, false);
	CollisionSet currentCollisionState;

	for (size_t i = 0; i < amountOfColliders; ++i)
	{
		for (size_t j = i + 1; j < amountOfColliders; ++j)
		{
			bool doPruneCheck = 
				!(collidersByAxis[j]->pos[axisToSortBy] - (collidersByAxis[j]->scale[axisToSortBy] * maxPossibleMaxMin) >
				  collidersByAxis[i]->pos[axisToSortBy] + (collidersByAxis[i]->scale[axisToSortBy] * maxPossibleMaxMin));

			if (doPruneCheck)
			{
				if (!ByPosSorter(collidersByAxis[i], collidersByAxis[j]))
				{
					std::swap(collidersByAxis[i], collidersByAxis[j]);
				}

				if (collidersByAxis[i]->isActive && ExecuteNarrowCollisionCheck(*collidersByAxis[i], *collidersByAxis[j]))
				{
					currentGeneralCollisonState[i] = true;
					currentGeneralCollisonState[j] = true;
					currentCollisionState.insert({ i, j });
				}
			}
			else break;
		}

		collidersByAxis[i]->isCollided = false;
		if (lastGeneralCollisionState[i] && !currentGeneralCollisonState[i])
		{
			ExecuteEndAnyCallbacksFor(*collidersByAxis[i]);
		}
	}

	if (collidersByAxis.size() && lastCollisionState.size() > currentCollisionState.size())
	{
		CollisionSet collidersToCallEndCallbacksSet = SetDifference(lastCollisionState, currentCollisionState);

		for (auto& [i, j] : collidersToCallEndCallbacksSet)
		{
			ExecuteEndBindedCallbacksFor(*collidersByAxis[i], *collidersByAxis[j]);
			ExecuteEndBindedCallbacksFor(*collidersByAxis[j], *collidersByAxis[i]);
		}
	}

	lastGeneralCollisionState = currentGeneralCollisonState;
	lastCollisionState = currentCollisionState;
}

void ColliderSim::AppendToSortedVectorOfColliders()
{
	collidersByAxis.emplace_back(this);
	lastGeneralCollisionState.emplace_back(false);
	ResortCollidersByPos();
}

void ColliderSim::DeleteFromSortedVectorOfColliders()
{
	CleanBindedCollisionCallbacksFrom(this);
	std::erase(collidersByAxis, this);
	lastGeneralCollisionState.resize(lastGeneralCollisionState.size() - 1);
}

void ColliderSim::AddPersistentCollisionCallback(const CollisionCallbackOptions& collisionOpts)
{
	AddCollisionCallbackImpl({ collisionOpts, false, true });
}

void ColliderSim::AddCollisionCallback(const CollisionCallbackOptions& collisionOpts)
{
	AddCollisionCallbackImpl({ collisionOpts });
}

void ColliderSim::SetColliderActive(bool value)
{
	isActive = value;
}

void ColliderSim::ClearCollisionCallbacks()
{
	static auto persistanceCheck = [](CollisionCallback& colCallback) { return !colCallback.persistent; };

	std::erase_if(anyCollisionCallbacks, persistanceCheck);
	for (auto& [_, bindedCollisionCallbackVect] : bindedCollisionCallbacks)
	{
		std::erase_if(bindedCollisionCallbackVect, persistanceCheck);
	}
}

void ColliderSim::AddCollisionCallbackImpl(const CollisionCallback& colCallback)
{
	if (!colCallback.collisionStart && !colCallback.collisionStop) return;

	if (!colCallback.colliderToBindTo)
	{
		anyCollisionCallbacks.push_back(colCallback);
	}
	else
	{
		auto iter = bindedCollisionCallbacks.find(colCallback.colliderToBindTo);

		if (iter == bindedCollisionCallbacks.end())
		{
			bindedCollisionCallbacks.emplace(colCallback.colliderToBindTo, std::vector<CollisionCallback>{});
		}

		bindedCollisionCallbacks[colCallback.colliderToBindTo].push_back(colCallback);
	}
}

void ColliderSim::ResortCollidersByPos()
{
	std::sort(collidersByAxis.begin(), collidersByAxis.end(), ByPosSorter);
}

bool ColliderSim::ByPosSorter(ColliderSim* firstCollider, ColliderSim* secondCollider)
{
	return firstCollider->pos[axisToSortBy] - (firstCollider->scale[axisToSortBy] / 2) <
		secondCollider->pos[axisToSortBy] - (secondCollider->scale[axisToSortBy] / 2);
}

float ColliderSim::CalcProjectionRadius(ColliderSim& collider, const glm::vec3& axis)
{
	float projRad{};

	for (int i = 0; i <3; ++i)
	{
		projRad += 
			((collider.scale[i] / 2.0f) * 
			glm::abs(glm::dot(axis, collider.orient.GetValue() * GetWorldAxisVector(static_cast<Axis>(i)))));
	}

	return projRad;
}

glm::vec3 ColliderSim::GetCurrentAxis(ColliderSim& firstCollider, ColliderSim& secondCollider, int index)
{
	// 0-2 - a, 3-5 - b, 6-... - cross prod. each 0,1,2 - x y z
	const glm::vec3& currentWorldAxisVect = GetWorldAxisVector(static_cast<Axis>(index % 3));
	int setOfCalcs = index / 3;

	if (setOfCalcs == 0)
	{
		return firstCollider.orient.GetValue() * currentWorldAxisVect;
	}

	glm::vec3 secondVect = secondCollider.orient.GetValue() * currentWorldAxisVect;

	if (setOfCalcs == 1)
	{
		return secondVect;
	}
	else
	{
		return glm::cross(
			firstCollider.orient.GetValue() * GetWorldAxisVector(static_cast<Axis>(setOfCalcs - 2)), secondVect
		);
	}
}

bool ColliderSim::ExecuteAABBCheck(ColliderSim& firstCollider, ColliderSim& secondCollider)
{
	bool res = true;

	for (int i = 0; i <3; ++i)
	{
		res = ((firstCollider.pos[i] + firstCollider.scale[i] / 2) -
			   (secondCollider.pos[i] - secondCollider.scale[i] / 2)) > 0.0f &&
			  ((firstCollider.pos[i] - firstCollider.scale[i] / 2) -
			   (secondCollider.pos[i] + secondCollider.scale[i] / 2)) < 0.0f;

		if (!res) break;
	}

	return res;
}

bool ColliderSim::ExecuteOBBCheck(ColliderSim& firstCollider, ColliderSim& secondCollider)
{
	constexpr int FaceAmount = 3 * 2;
	constexpr int CrossAmount = 3 * 3;
	constexpr int OBBCalcAmount = FaceAmount + CrossAmount;
	constexpr float error = 1e-5f;

	for (int i = 0; i < OBBCalcAmount; ++i)
	{
		glm::vec3 currentAxis = glm::normalize(GetCurrentAxis(firstCollider, secondCollider, i));

		float rA = CalcProjectionRadius(firstCollider, currentAxis);
		float rB = CalcProjectionRadius(secondCollider, currentAxis);

		float distance = glm::abs(glm::dot(secondCollider.pos.GetValue() - firstCollider.pos.GetValue(), currentAxis));

		if (distance > rA + rB + error) return false;
	}

	return true;
}

bool ColliderSim::ExecuteNarrowCollisionCheck(ColliderSim& firstCollider, ColliderSim& secondCollider)
{
	constexpr glm::quat identityQuat = glm::identity<glm::quat>();

	// Use simpler AABB check for unrotated colliders, otherwise go with OBB
	// It's expected to execute OBB for any collider that was rotated at all, even if re-rotated back to default orientation
	bool res = firstCollider.orient.GetValue() == identityQuat && secondCollider.orient.GetValue() == identityQuat ?
		ExecuteAABBCheck(firstCollider, secondCollider) : ExecuteOBBCheck(firstCollider, secondCollider);

	if (res)
	{
		ExecuteStartCallbacksFor(firstCollider, secondCollider);
		ExecuteStartCallbacksFor(secondCollider, firstCollider);
	}

	return res;
}

void ColliderSim::ExecuteStartCallbacksFor(ColliderSim& colliderToExe, ColliderSim& bindedCollider)
{
	if (!colliderToExe.isCollided)
	{
		for (auto& anyCollisionCallback : colliderToExe.anyCollisionCallbacks)
		{
			if (anyCollisionCallback.collisionStart &&
				(anyCollisionCallback.holdable || !anyCollisionCallback.started))
			{
				anyCollisionCallback.started = true;
				anyCollisionCallback.collisionStart();
			}
		}
	}

	auto iter = colliderToExe.bindedCollisionCallbacks.find(&bindedCollider);

	if (iter != colliderToExe.bindedCollisionCallbacks.end())
	{
		for (auto& bindedCollisionCallback : iter->second)
		{
			if (bindedCollisionCallback.collisionStart && 
			    (bindedCollisionCallback.holdable || !bindedCollisionCallback.started))
			{
				bindedCollisionCallback.started = true;
				bindedCollisionCallback.collisionStart();
			}
		}
	}

	colliderToExe.isCollided = true;
}

void ColliderSim::ExecuteEndAnyCallbacksFor(ColliderSim& colliderToExe)
{
	for (auto& anyCollisionCallback : colliderToExe.anyCollisionCallbacks)
	{
		anyCollisionCallback.started = false;

		if (anyCollisionCallback.collisionStop)
		{
			anyCollisionCallback.collisionStop();
		}
	}
}

void ColliderSim::ExecuteEndBindedCallbacksFor(ColliderSim& firstCollider, ColliderSim& secondCollider)
{
	if (firstCollider.bindedCollisionCallbacks.contains(&secondCollider))
	{
		for (auto& bindedCollisionCallback : firstCollider.bindedCollisionCallbacks[&secondCollider])
		{
			bindedCollisionCallback.started = false;

			if (bindedCollisionCallback.collisionStop)
			{
				bindedCollisionCallback.collisionStop();
			}
		}
	}
}

void ColliderSim::CleanBindedCollisionCallbacksFrom(ColliderSim* colliderToRemove)
{
	for (auto collider : collidersByAxis)
	{
		collider->bindedCollisionCallbacks.erase(colliderToRemove);
	}
}