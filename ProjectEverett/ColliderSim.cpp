#include "ColliderSim.h"

#include <algorithm>

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

glm::quat& ColliderSim::GetOrientationAddr()
{
	static glm::quat placeholderOrient{};

	return placeholderOrient;
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
	size_t amountOfColliders = collidersByAxis.size();

	std::vector<bool> currentGeneralCollisonState(amountOfColliders, false);
	CollisionSet currentCollisionState;

	for (size_t i = 0; i < amountOfColliders; ++i)
	{
		for (size_t j = i + 1; j < amountOfColliders; ++j)
		{
			bool doPruneCheck = !(collidersByAxis[j]->pos[axisToSortBy] - (collidersByAxis[j]->scale[axisToSortBy] / 2) >
				                  collidersByAxis[i]->pos[axisToSortBy] + (collidersByAxis[i]->scale[axisToSortBy] / 2));

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
			else
			{
				break;
			}
		}

		collidersByAxis[i]->isCollided = false;
		if (lastGeneralCollisionState[i] && !currentGeneralCollisonState[i])
		{
			ExecuteEndAnyCallbacksFor(*collidersByAxis[i]);
		}
	}

	if (lastCollisionState.size() > currentCollisionState.size())
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
	ResortCollidersByXPos();
}

void ColliderSim::DeleteFromSortedVectorOfColliders()
{
	CleanBindedCollisionCallbacksFrom(this);
	std::erase(collidersByAxis, this);
	lastGeneralCollisionState.resize(lastGeneralCollisionState.size() - 1);
}

void ColliderSim::AddCollisionCallback(const CollisionCallbackOptions& collisionOpts)
{
	if (!collisionOpts.collisionStart && !collisionOpts.collisionStop) return;

	if (!collisionOpts.colliderToBindTo)
	{
		anyCollisionCallbacks.push_back({ collisionOpts });
	}
	else
	{
		auto iter = bindedCollisionCallbacks.find(collisionOpts.colliderToBindTo);

		if (iter == bindedCollisionCallbacks.end())
		{
			bindedCollisionCallbacks.emplace(collisionOpts.colliderToBindTo, std::vector<CollisionCallback>{});
		}

		bindedCollisionCallbacks[collisionOpts.colliderToBindTo].push_back({ collisionOpts });
	}
}

void ColliderSim::SetColliderActive(bool value)
{
	isActive = value;
}

void ColliderSim::ResortCollidersByXPos()
{
	std::sort(collidersByAxis.begin(), collidersByAxis.end(), ByPosSorter);
}

bool ColliderSim::ByPosSorter(ColliderSim* firstCollider, ColliderSim* secondCollider)
{
	return firstCollider->pos[axisToSortBy] - (firstCollider->scale[axisToSortBy] / 2) <
		secondCollider->pos[axisToSortBy] - (secondCollider->scale[axisToSortBy] / 2);
}

bool ColliderSim::ExecuteNarrowCollisionCheck(ColliderSim& firstCollider, ColliderSim& secondCollider)
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