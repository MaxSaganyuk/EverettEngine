#include "SolidSim.h"

void SolidSim::CheckRotationLimits()
{
	auto CheckRotation = [](float& toCheck, float min, float max)
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

	for (int i = 0; i <3; ++i)
	{
		CheckRotation(rotate[i], rotationLimits.first[i], rotationLimits.second[i]);
	}
}

void SolidSim::ResetModelMatrix(const Rotation& toRotate)
{
	rotate += toRotate;

	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, scale);
	// gimbal lock will happen, will rewrite to use quaternions later
	for (int i = 0; i <3; ++i)
	{
		model = glm::rotate(model, rotate[i], { i == 0, i == 1, i == 2 });
	}
}

SolidSim::SolidSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed
)
	: front(front), pos(pos), scale(scale), speed(speed)
{
	ResetModelMatrix();

	rotate = { 0.0f, 0.0f, 0.0f };

	rotationLimits = {
		{0.0f, 0.0f, 0.0f},
		{fullRotation, fullRotation, fullRotation}
	};

	lastPos = pos;

	lastBlocker = false;
	ghostMode = false;

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}

	type = SolidType::Static;
}

void SolidSim::SetGhostMode(bool val)
{
	ghostMode = val;
}

bool SolidSim::IsGhostMode() const
{
	return ghostMode;
}

void SolidSim::SetType(SolidType type)
{
	this->type = type;
}

void SolidSim::InvertMovement()
{
	speed = (-1) * speed;
}

bool SolidSim::IsMovementInverted()
{
	return speed < 0.0f;
}

glm::mat4& SolidSim::GetModelMatrixAddr()
{
	return model;
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
		pos -= speed * glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		break;
	case Direction::Right:
		pos += speed * glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		break;
	case Direction::Up:
		pos += glm::abs(speed * glm::normalize(front * glm::vec3(0.0f, 1.0f, 0.0f)));
		break;
	case Direction::Down:
		pos -= glm::abs(speed * glm::normalize(front * glm::vec3(0.0f, 1.0f, 0.0f)));
		break;
	case Direction::Nowhere:
		return;
	default:
		assert(false && "Undefined direction");
		return;
	}

	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix();
	}
	else 
	{
		model[3].x = pos.x;
		model[3].y = pos.y;
		model[3].z = pos.z;
	}
}

void SolidSim::LimitRotations(const Rotation& min, const Rotation& max)
{
	rotationLimits = { min, max };
}

void SolidSim::Rotate(const Rotation& toRotate)
{
	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix(toRotate);
	}
	else 
	{
		for (int i = 0; i <3; ++i)
		{
			if (toRotate[i])
			{
				model = glm::rotate(model, toRotate[i], { i == 0, i == 1, i == 2 });
				rotate[i] += toRotate[i];
			}
		}
	}

	CheckRotationLimits();

	glm::vec3 direction;
	direction.x = cos(glm::radians(rotate.GetPitch())) * cos(glm::radians(rotate.GetYaw()));
	direction.y = sin(glm::radians(rotate.GetYaw()));
	direction.z = sin(glm::radians(rotate.GetPitch())) * cos(glm::radians(rotate.GetYaw()));
	front = glm::normalize(direction);
}

bool SolidSim::CheckForCollision(const SolidSim& solid1, const SolidSim& solid2)
{
	if (solid1.IsGhostMode() || solid2.IsGhostMode())
	{
		return false;
	}

	bool res = true;

	for (int i = 0; i <3; ++i)
	{
		res = ((solid1.pos[i] + solid1.scale[i] / 2) - (solid2.pos[i] - solid2.scale[i] / 2)) > 0.0f &&
			  ((solid1.pos[i] - solid1.scale[i] / 2) - (solid2.pos[i] + solid2.scale[i] / 2)) < 0.0f;

		if (!res) break;
	}

	return res;
}