#include "SolidSim.h"

void SolidSim::CheckRotationLimits()
{
#define CheckRotationFor(dir) checkRotation(rotate.dir, rotationLimits.first.dir, rotationLimits.second.dir)
	
	auto checkRotation = [](float& toCheck, float min, float max)
	{
		if (toCheck > fullRotation)
		{
			toCheck = 0.0f;
		}
		if (toCheck < -fullRotation)
		{
			toCheck = 0.0f;
		}

		if (toCheck > max)
		{
			toCheck = max;
		}
		else if (toCheck < min)
		{
			toCheck = min;
		}
	};

	CheckRotationFor(yaw);
	CheckRotationFor(pitch);
	CheckRotationFor(roll);

#undef CheckRotationFor
}

SolidSim::SolidSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed
)
	: front(front), pos(pos), scale(scale), speed(speed)
{
	rotate = { -90.0f, 0.0f, 0.0f };

	rotationLimits = {
		{-fullRotation, -fullRotation, -fullRotation},
		{fullRotation, fullRotation, fullRotation}
	};

	lastBlocker = false;

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}
}

void SolidSim::InvertMovement()
{
	speed = (-1) * speed;
}

bool SolidSim::IsMovementInverted()
{
	return speed < 0.0f;
}

glm::vec3& SolidSim::GetFrontVectorAddr()
{
	return front;
}

glm::vec3& SolidSim::GetPositionVectorAddr()
{
	return pos;
}

glm::vec3& SolidSim::GetScaleVectorAddr()
{
	return scale;
}

void SolidSim::DisableDirection(Direction dir)
{
	disabledDirs[dir] = true;
}

void SolidSim::EnableDirection(Direction dir)
{
	disabledDirs[dir] = false;
}

void SolidSim::EnableAllDirections()
{
	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}
}

size_t SolidSim::GetAmountOfDisabledDirs()
{
	size_t amount = 0;

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		amount += disabledDirs[static_cast<Direction>(i)];
	}

	return amount;
}

SolidSim::Direction SolidSim::GetLastDirection()
{
	return lastDir;
}

void SolidSim::SetLastPosition()
{
	pos = lastPos;
}

void SolidSim::SetPosition(Direction dir, const glm::vec3& limitAxis)
{
	if (disabledDirs[dir])
	{
		return;
	}

	if (!lastBlocker && dir != Direction::Nowhere)
	{
		lastBlocker = true;
		lastDir = dir;
		lastPos = pos;
	}
	else
	{
		lastBlocker = false;
	}

	switch (dir)
	{
	case Direction::Forward:
		pos += speed * front * limitAxis;
		break;
	case Direction::Backward:
		pos -= speed * front * limitAxis;
		break;
	case Direction::Left:
		pos -= speed * glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case Direction::Right:
		pos += speed * glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case Direction::Up:
		pos -= speed * glm::cross(front, glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case Direction::Down:
		pos += speed * glm::cross(front, glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case Direction::Nowhere:
		break;
	default:
		assert(false && "Undefined direction");
		return;
	}
}

void SolidSim::LimitRotations(const Rotation& min, const Rotation& max)
{
	rotationLimits = { min, max };
}

void SolidSim::Rotate(const Rotation& toRotate)
{
	rotate += toRotate;

	CheckRotationLimits();

	glm::vec3 direction;
	direction.x = cos(glm::radians(rotate.yaw)) * cos(glm::radians(rotate.pitch));
	direction.y = sin(glm::radians(rotate.pitch));
	direction.z = sin(glm::radians(rotate.yaw)) * cos(glm::radians(rotate.pitch));
	front = glm::normalize(direction);
}

bool SolidSim::CheckForCollision(const SolidSim& solid1, const SolidSim& solid2)
{
	bool res = true;

	for (int i = 0; i <3; ++i)
	{
		res = ((solid1.pos[i] + solid1.scale[i] / 2) - (solid2.pos[i] - solid2.scale[i] / 2)) > 0.0f &&
			  ((solid1.pos[i] - solid1.scale[i] / 2) - (solid2.pos[i] + solid2.scale[i] / 2)) < 0.0f;

		if (!res) break;
	}

	return res;
}