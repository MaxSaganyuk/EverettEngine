#pragma once

#include "external/IObjectSim.h"

#include "stdEx/utilityEx.h"

#include <unordered_map>

#include "SimSerializer.h"
#include "ValueObserver.h"

class ObjectSim : virtual public IObjectSim
{
private:
	template<LinkableFuncNames funcName, typename MemberFuncType, typename... ParamTypes>
	void ExecuteLinkedObjects(MemberFuncType memberFunc, ParamTypes&&... values);

	void RotateImpl(const Rotation& toRotate);
	glm::quat CalcOrientationFromRotation(const Rotation& toRotate);
protected:
	static inline float renderDeltaTime = 1.0f;

	std::string GetSimInfoToSaveImpl();
	bool SetSimInfoToLoad(std::string_view& line);
	void CheckRotationLimits();
	
	constexpr static size_t realDirectionAmount = 6;

	ValueObserver<glm::vec3> pos;
	ValueObserver<glm::vec3> scale;
	ValueObserver<glm::quat> orient;

	float speed;

	std::unordered_map<Direction, bool> disabledDirs;
	std::pair<Rotation, Rotation> rotationLimits;

	static inline stdEx::RelationGraph<ObjectSim*> objectGraph;
	bool visited; // Utility bool to prevent infinite loops of linked object traversal
	bool objectLinkingEnabled;
	std::array<bool, std::to_underlying(LinkableFuncNames::_SIZE)> objectLinkingForFuncTracker;
public:
	ObjectSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const float speed = 1.0f
	);
	~ObjectSim();

	std::string GetThisObjectTypeNameStr() override;

	static void InitializeObjectGraph();
	static void ResetObjectLinking();
	static void SetRenderDeltaTime(float deltaTime);
	static const glm::vec3& GetWorldAxisVector(Axis axis);

	// True if values updated
	virtual bool UpdateTransform();

	void InvertMovement(bool value = true, bool executeLinkedObjects = true) override;
	bool IsMovementInverted() override;

	void SetPositionVector(const glm::vec3& vect, bool executeLinkedObjects = true) override;
	void SetScaleVector(const glm::vec3& vect, bool executeLinkedObjects = true) override;
	void SetOrientation(const glm::quat& quat, bool executeLinkedObjects = true) override;
		
	glm::vec3 GetFrontVector() override;
	glm::vec3 GetUpVector() override;

	const glm::vec3& GetPositionVectorAddr() override;
	const glm::vec3& GetScaleVectorAddr() override;
	const glm::quat& GetOrientationAddr() override;

	void SetMovementSpeed(float speed, bool executeLinkedObjects = true) override;
	float GetMovementSpeed() override;

	void DisableDirection(Direction dir, bool executeLinkedObjects = true) override;
	void EnableDirection(Direction dir, bool executeLinkedObjects = true) override;
	void EnableAllDirections(bool executeLinkedObjects = true) override;
	size_t GetAmountOfDisabledDirs() override;

	void SetLastPosition(bool executeLinkedObjects = true) override;
	void SetLastScale(bool executeLinkedObjects = true) override;
	void SetLastOrientation(bool executeLinkedObjects = true) override;
	void MoveInDirection(
		Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) override;
	void MoveByAxis(
		const glm::vec3& axis, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) override;
	void MoveByAxis(
		Axis axis, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) override;

	void LimitRotations(const Rotation& min, const Rotation& max, bool executeLinkedObjects = true) override;
	void Rotate(const Rotation& toRotate, bool executeLinkedObjects = true) override;
	void RevolveAround(const Rotation& toRotate, const glm::vec3& centerPos, bool executeLinkedObjects = true) override;
	void LookAt(const glm::vec3& pointToLookAt, bool executeLinkedObjects = true) override;

	// Callback setter
	void SetPositionChangeCallback(std::function<void()> callback);
	void SetScaleChangeCallback(std::function<void()> callback);
	void SetRotationChangeCallback(std::function<void()> callback);

	void HardLinkObject(IObjectSim& otherObject);
	void LinkObject(IObjectSim& otherObject) override;
	void UnlinkObject(IObjectSim& objectToUnlink) override;
	void EnableObjectLinking(bool val = true) override;
	void EnableLinkedExecutionForFunc(LinkableFuncNames linkFuncName, bool value = true) override;
};