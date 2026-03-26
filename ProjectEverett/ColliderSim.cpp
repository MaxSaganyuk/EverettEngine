#include "ColliderSim.h"

#include <algorithm>

ColliderSim::ColliderSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	float speed
)
	: ObjectSim(pos, scale, speed), isCollided(false), isActive(true) {}

std::string ColliderSim::GetObjectTypeNameStr()
{
	return "Collider";
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

	CollisionSet currentCollisionState;
	std::fill(generalCollisionState.begin(), generalCollisionState.end(), false);

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

				if (ExecuteNarrowCollisionCheck(*collidersByAxis[i], *collidersByAxis[j]))
				{
					generalCollisionState[i] = true;
					generalCollisionState[j] = true;
					currentCollisionState.insert({ i, j });
				}
			}
			else
			{
				break;
			}
		}

		if (!generalCollisionState[i] && collidersByAxis[i]->isCollided)
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

	lastCollisionState = currentCollisionState;
}

void ColliderSim::AppendToSortedVectorOfColliders()
{
	collidersByAxis.emplace_back(this);
	generalCollisionState.emplace_back(false);
	ResortCollidersByXPos();
}

void ColliderSim::DeleteFromSortedVectorOfColliders()
{
	CleanBindedCollisionCallbacksFrom(this);
	std::erase(collidersByAxis, this);
	generalCollisionState.resize(generalCollisionState.size() - 1);
}

void ColliderSim::AddAnyCollisionCallback(std::function<void()> collisionStart, std::function<void()> collisionStop)
{
	anyCollisionCallbacks.push_back({ collisionStart, collisionStop });
}

void ColliderSim::AddBindedCollisionCallback(
	IColliderSim& otherCollider, std::function<void()> collisionStart, std::function<void()> collisionStop
)
{
	auto iter = bindedCollisionCallbacks.find(&otherCollider);

	if (iter == bindedCollisionCallbacks.end())
	{
		bindedCollisionCallbacks.emplace(&otherCollider, std::vector<CollisionCallback>{});
	}

	bindedCollisionCallbacks[&otherCollider].push_back({ collisionStart, collisionStop });
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
	colliderToExe.isCollided = true;

	auto iter = colliderToExe.bindedCollisionCallbacks.find(&bindedCollider);

	if (iter != colliderToExe.bindedCollisionCallbacks.end())
	{
		for (auto& bindedCollisionCallback : iter->second)
		{
			bindedCollisionCallback.started = true;
			bindedCollisionCallback.callbackStart();
		}
	}

	for (auto& anyCollisionCallbackStruct : colliderToExe.anyCollisionCallbacks)
	{
		anyCollisionCallbackStruct.callbackStart();
	}
}

void ColliderSim::ExecuteEndAnyCallbacksFor(ColliderSim& colliderToExe)
{
	colliderToExe.isCollided = false;

	for (auto& anyCollisionCallbackStruct : colliderToExe.anyCollisionCallbacks)
	{
		anyCollisionCallbackStruct.callbackStop();
	}
}

void ColliderSim::ExecuteEndBindedCallbacksFor(ColliderSim& firstCollider, ColliderSim& secondCollider)
{
	if (firstCollider.bindedCollisionCallbacks.contains(&secondCollider))
	{
		for (auto& bindedCollisionCallback : firstCollider.bindedCollisionCallbacks[&secondCollider])
		{
			bindedCollisionCallback.started = false;

			if (bindedCollisionCallback.callbackStop)
			{
				bindedCollisionCallback.callbackStop();
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