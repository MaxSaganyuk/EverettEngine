#pragma once

#include <cassert>
#include <map>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class SolidSim
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

	struct Rotation
	{
		float yaw;
		float pitch;
		float roll;

		Rotation& operator+=(const Rotation& toRotate)
		{
			yaw   += toRotate.yaw;
			pitch += toRotate.pitch;
			roll  += toRotate.roll;

			return *this;
		}
	};

	enum class SolidType
	{
		Static, // Set if solid is unchanging or changes it's position, rotation or scale rarely
		Dynamic // UNIMPLEMENTED Set if solid changes it's position, rotation or scale constantly or often
	};


	constexpr static float fullRotation = 360.0f;

private:
	constexpr static size_t realDirectionAmount = 6;

	glm::mat4 model;
	glm::vec3 scale;
	glm::vec3 front;
	glm::vec3 pos;
	glm::vec3 lastPos;
	Rotation rotate;
	bool lastBlocker;
	float speed;

	Direction lastDir;

	SolidType type;

	std::map<Direction, bool> disabledDirs;
	std::pair<Rotation, Rotation> rotationLimits;

	void CheckRotationLimits();
	void ResetModelMatrix();
public:
	SolidSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
		const float speed = 0.006f,
		const SolidType type = SolidType::Static
	);

	void InvertMovement();
	bool IsMovementInverted();

	glm::mat4& GetModelMatrixAddr();
	glm::vec3& GetFrontVectorAddr();
	glm::vec3& GetPositionVectorAddr();
	glm::vec3& GetScaleVectorAddr();

	void DisableDirection(Direction dir);
	void EnableDirection(Direction dir);
	void EnableAllDirections();
	size_t GetAmountOfDisabledDirs();
	Direction GetLastDirection();

	void SetLastPosition();
	void SetPosition(Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f });

	void LimitRotations(const Rotation& min, const Rotation& max);
	void Rotate(const Rotation& toRotate);

	static bool CheckForCollision(const SolidSim& solid1, const SolidSim& solid2);
};