#pragma once
#pragma warning(disable : 4250) // GCC and Clang treat automatic dominance resolve as a feature

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <functional>
#include <string>

#define Linkable // Linkable methods allow chained execution

class IObjectSim
{
public:
	enum class Direction
	{
		Forward,
		Backward,
		Left,
		Right,
		Up,
		Down
	};

	struct Rotation : public glm::vec3
	{
		Rotation(const glm::vec3& axis = {}) : glm::vec3(axis) {}
		Rotation(float pitch, float yaw, float roll) : Rotation(glm::vec3{ pitch, yaw, roll }) {}

		float GetPitch() const { return x; };
		float GetYaw() const   { return y; };
		float GetRoll() const  { return z; };

		bool Zeroed() const { return (x + y + z) == 0.0f; }

		Rotation& operator+=(const Rotation& toRotate)
		{
			x += toRotate.x;
			y += toRotate.y;
			z += toRotate.z;

			return *this;
		}
	};

	constexpr static float fullRotation = 2.0f * glm::pi<float>();

	constexpr static inline const glm::vec3 worldRight{ 1, 0, 0 };
	constexpr static inline const glm::vec3 worldUp   { 0, 1, 0 };
	constexpr static inline const glm::vec3 worldFront{ 0, 0, 1 };

	virtual std::string GetThisObjectTypeNameStr() = 0;

	Linkable virtual void InvertMovement(bool value = true, bool executeLinkedObjects = true) = 0;
	virtual bool IsMovementInverted() = 0;

	Linkable virtual void SetMovementSpeed(float speed, bool executeLinkedObjects = true) = 0;
	virtual float GetMovementSpeed() = 0;

	Linkable virtual void SetPositionVector(const glm::vec3& vect, bool executeLinkedObjects = true) = 0;
	Linkable virtual void SetScaleVector(const glm::vec3& vect, bool executeLinkedObjects = true) = 0;
	Linkable virtual void SetOrientation(const glm::quat& quat, bool executeLinkedObjects = true) = 0;

	virtual glm::vec3 GetUpVector() = 0;
	virtual glm::vec3 GetFrontVector() = 0;

	virtual glm::vec3& GetPositionVectorAddr() = 0;
	virtual glm::vec3& GetScaleVectorAddr() = 0;
	virtual glm::quat& GetOrientationAddr() = 0;

	Linkable virtual void DisableDirection(Direction dir, bool executeLinkedObjects = true) = 0;
	Linkable virtual void EnableDirection(Direction dir, bool executeLinkedObjects = true) = 0;
	Linkable virtual void EnableAllDirections(bool executeLinkedObjects = true) = 0;
	virtual size_t GetAmountOfDisabledDirs() = 0;

	Linkable virtual void SetLastPosition(bool executeLinkedObjects = true) = 0;
	Linkable virtual void MoveInDirection(
		Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) = 0;
	Linkable virtual void MoveByAxis(
		const glm::vec3& axis, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) = 0;

	Linkable virtual void LimitRotations(
		const Rotation& min, const Rotation& max, bool executeLinkedObjects = true
	) = 0;
	Linkable virtual void Rotate(const Rotation& toRotate, bool executeLinkedObjects = true) = 0;

	virtual bool IsScriptFuncAdded(const std::string& dllName = "") = 0;

	virtual void SetPositionChangeCallback(std::function<void()> callback) = 0;
	virtual void SetRotationChangeCallback(std::function<void()> callback) = 0;

	virtual void LinkObject(IObjectSim& objectToLink) = 0;
	virtual void UnlinkObject(IObjectSim& objectToUnlink) = 0;
	virtual void EnableObjectLinking(bool val = true) = 0; // By default enabled.
};