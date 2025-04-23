#pragma once
#pragma warning(disable : 4250) // GCC and Clang treat automatic dominance resolve as a feature

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <functional>
#include <string>

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
		Down,
		Nowhere
	};

	struct Rotation : public glm::vec3
	{
		Rotation(const glm::vec3& axis = {}) : glm::vec3(axis) {}
		Rotation(float pitch, float yaw, float roll) : Rotation(glm::vec3{ pitch, yaw, roll }) {}

		float GetPitch() { return x; };
		float GetYaw() { return y; };
		float GetRoll() { return z; };

		Rotation& operator+=(const Rotation& toRotate)
		{
			x += toRotate.x;
			y += toRotate.y;
			z += toRotate.z;

			return *this;
		}
	};

	constexpr static float fullRotation = 360.0f;

	virtual void InvertMovement() = 0;
	virtual bool IsMovementInverted() = 0;

	virtual glm::vec3& GetFrontVectorAddr() = 0;
	virtual glm::vec3& GetPositionVectorAddr() = 0;
	virtual glm::vec3& GetUpVectorAddr() = 0;
	virtual glm::vec3& GetScaleVectorAddr() = 0;

	virtual void SetGhostMode(bool val) = 0;
	virtual bool IsGhostMode() const = 0;

	virtual void DisableDirection(Direction dir) = 0;
	virtual void EnableDirection(Direction dir) = 0;
	virtual void EnableAllDirections() = 0;
	virtual size_t GetAmountOfDisabledDirs() = 0;
	virtual Direction GetLastDirection() = 0;

	virtual void SetLastPosition() = 0;
	virtual void SetPosition(Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }) = 0;

	virtual void LimitRotations(const Rotation& min, const Rotation& max) = 0;
	virtual void Rotate(const Rotation& toRotate) = 0;

	virtual void ExecuteScriptFunc(const std::string& dllName = "") = 0;
	virtual void ExecuteAllScriptFuncs() = 0;
	virtual bool IsScriptFuncAdded(const std::string& dllName = "") = 0;
	virtual bool IsScriptFuncRunnable(const std::string& dllName = "") = 0;
};