#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "ObjectSim.h"
#include "external/IColliderSim.h"

class ColliderSim : public ObjectSim, public IColliderSim
{
public:
	ColliderSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		float speed = 1.0f
	);
	~ColliderSim();

	std::string GetThisObjectTypeNameStr() override;
	static std::string GetObjectTypeNameStr();

	static void ExecuteBroadCollisionCheck();

	void AppendToSortedVectorOfColliders();
	void DeleteFromSortedVectorOfColliders();

	// Persistent collision callbacks are engine only to keep gizmo color change after clean, for example.
	void AddPersistentCollisionCallback(const CollisionCallbackOptions& collisionOpts);
	
	void AddCollisionCallback(const CollisionCallbackOptions& collisionOpts) override;
	void SetColliderActive(bool value = true) override;
	void ClearCollisionCallbacks() override;

	std::string GetSimInfoToSave(const std::string& colliderName);
	bool SetSimInfoToLoad(std::string_view& line);

	// TODO: Implement math for narrow collision check for rotated colliders, for now
	// rotations for colliders are not allowed
	glm::quat& GetOrientationAddr() override;
	void SetOrientation(const glm::quat& quat, bool executeLinkedObjects = true) override {};
	void Rotate(const Rotation& toRotate, bool executeLinkedObjects = true) override {};
	//
private:
	constexpr static char TypeName[] = "Collider";

	enum Axis
	{
		X, Y, Z
	};

	struct CollisionCallback : public CollisionCallbackOptions
	{
		bool started{};
		bool persistent{};
	};

	using CollisionSet = std::set<std::pair<size_t, size_t>>;

	void AddCollisionCallbackImpl(const CollisionCallback& colCallback);

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

	std::string GetSimInfoToSaveImpl();
 
	bool isActive;

	bool isCollided;
	std::vector<CollisionCallback> anyCollisionCallbacks;
	std::unordered_map<IColliderSim*, std::vector<CollisionCallback>> bindedCollisionCallbacks;

	static inline std::vector<bool> lastGeneralCollisionState;
	static inline CollisionSet lastCollisionState;

	constexpr static Axis axisToSortBy = Axis::X;
	static inline std::vector<ColliderSim*> collidersByAxis;
};