#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "ObjectSim.h"
#include "interfaces/IColliderSim.h"

class ColliderSim : public ObjectSim, public IColliderSim
{
public:
	ColliderSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		float speed = 1.0f
	);

	static std::string GetObjectTypeNameStr();

	static void ExecuteBroadCollisionCheck();

	void AppendToSortedVectorOfColliders();
	void DeleteFromSortedVectorOfColliders();

	void AddAnyCollisionCallback(
		std::function<void()> collisionStart, std::function<void()> collisionStop = nullptr
	) override;
	void AddBindedCollisionCallback(
		IColliderSim& otherCollider, std::function<void()> collisionStart, std::function<void()> collisionStop = nullptr
	) override;
	void SetColliderActive(bool value = true) override;

	std::string GetSimInfoToSave(const std::string& colliderName);
	bool SetSimInfoToLoad(std::string_view& line);

private:
	enum Axis
	{
		X, Y, Z
	};

	struct CollisionCallback
	{
		std::function<void()> callbackStart;
		std::function<void()> callbackStop;
		bool started{};
	};

	using CollisionSet = std::set<std::pair<size_t, size_t>>;

	static void ResortCollidersByXPos();
	static bool ByPosSorter(ColliderSim* firstCollider, ColliderSim* secondCollider);

	static bool ExecuteNarrowCollisionCheck(ColliderSim& firstCollider, ColliderSim& secondCollider);
	static void ExecuteStartCallbacksFor(ColliderSim& colliderToExe, ColliderSim& bindedCollider);
	static void ExecuteEndBindedCallbacksFor(ColliderSim& firstCollider, ColliderSim& secondCollider);
	static void ExecuteEndAnyCallbacksFor(ColliderSim& colliderToExe);

	static CollisionSet SetDifference(
		const CollisionSet& firstSet, const CollisionSet& secondSet
	);

	static void CleanBindedCollisionCallbacksFrom(ColliderSim* colliderToRemove);
 
	bool isActive;

	bool isCollided;
	std::vector<CollisionCallback> anyCollisionCallbacks;
	std::unordered_map<IColliderSim*, std::vector<CollisionCallback>> bindedCollisionCallbacks;

	static inline std::vector<bool> generalCollisionState;
	static inline CollisionSet lastCollisionState;

	constexpr static Axis axisToSortBy = Axis::X;
	static inline std::vector<ColliderSim*> collidersByAxis;
};