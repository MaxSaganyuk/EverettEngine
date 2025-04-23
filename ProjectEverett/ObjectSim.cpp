#include "ObjectSim.h"

ObjectSim::ObjectSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed
)
	: front(front), pos(pos), scale(scale), speed(speed)
{
	rotate = { 0.0f, 0.0f, 0.0f };

	rotationLimits = {
		{0.0f, 0.0f, 0.0f},
		{fullRotation, fullRotation, fullRotation}
	};

	up = glm::vec3(0.0f, 1.0f, 0.0f);

	lastPos = pos;

	lastBlocker = false;
	ghostMode = false;

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}
}

void ObjectSim::CheckRotationLimits()
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

	for (int i = 0; i < 3; ++i)
	{
		CheckRotation(rotate[i], rotationLimits.first[i], rotationLimits.second[i]);
	}
}


void ObjectSim::SetGhostMode(bool val)
{
	ghostMode = val;
}

bool ObjectSim::IsGhostMode() const
{
	return ghostMode;
}

void ObjectSim::InvertMovement()
{
	speed = (-1) * speed;
}

bool ObjectSim::IsMovementInverted()
{
	return speed < 0.0f;
}

glm::vec3& ObjectSim::GetFrontVectorAddr()
{
	return front;
}

glm::vec3& ObjectSim::GetPositionVectorAddr()
{
	return pos;
}

glm::vec3& ObjectSim::GetUpVectorAddr()
{
	return up;
}

glm::vec3& ObjectSim::GetScaleVectorAddr()
{
	return scale;
}

void ObjectSim::DisableDirection(Direction dir)
{
	disabledDirs[dir] = true;
}

void ObjectSim::EnableDirection(Direction dir)
{
	disabledDirs[dir] = false;
}

void ObjectSim::EnableAllDirections()
{
	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}
}

size_t ObjectSim::GetAmountOfDisabledDirs()
{
	size_t amount = 0;

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		amount += disabledDirs[static_cast<Direction>(i)];
	}

	return amount;
}

ObjectSim::Direction ObjectSim::GetLastDirection()
{
	return lastDir;
}

void ObjectSim::SetLastPosition()
{
	pos = lastPos;
}

void ObjectSim::SetPosition(Direction dir, const glm::vec3& limitAxis)
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
		pos -= speed * glm::normalize(glm::cross(front, up));
		break;
	case Direction::Right:
		pos += speed * glm::normalize(glm::cross(front, up));
		break;
	case Direction::Up:
		pos += glm::abs(speed * glm::normalize(front * up));
		break;
	case Direction::Down:
		pos -= glm::abs(speed * glm::normalize(front * up));
		break;
	case Direction::Nowhere:
		return;
	default:
		assert(false && "Undefined direction");
		return;
	}
}

void ObjectSim::LimitRotations(const Rotation& min, const Rotation& max)
{
	rotationLimits = { min, max };
}

void ObjectSim::Rotate(const Rotation& toRotate)
{
	CheckRotationLimits();

	glm::vec3 direction;
	direction.x = cos(glm::radians(rotate.GetPitch())) * cos(glm::radians(rotate.GetYaw()));
	direction.y = sin(glm::radians(rotate.GetYaw()));
	direction.z = sin(glm::radians(rotate.GetPitch())) * cos(glm::radians(rotate.GetYaw()));
	front = glm::normalize(direction);
}

void ObjectSim::AddScriptFunc(const std::string& dllName, ScriptFuncStorage::ScriptFuncWeakPtr& scriptFunc)
{
	scriptFuncStorage.AddScriptFunc(dllName, scriptFunc);
}

void ObjectSim::ExecuteScriptFunc(const std::string& dllName)
{
	scriptFuncStorage.ExecuteScriptFunc(this, dllName);
}

void ObjectSim::ExecuteAllScriptFuncs()
{
	scriptFuncStorage.ExecuteAllScriptFuncs(this);
}

bool ObjectSim::IsScriptFuncAdded(const std::string& dllName)
{
	return scriptFuncStorage.IsScriptFuncAdded(dllName);
}

bool ObjectSim::IsScriptFuncRunnable(const std::string& dllName)
{
	return scriptFuncStorage.IsScriptFuncRunnable(dllName);
}