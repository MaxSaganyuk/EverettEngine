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

std::string ObjectSim::GetObjectTypeNameStr()
{
	return "Object";
}

void ObjectSim::SetRenderDeltaTime(float deltaTime)
{
	renderDeltaTime = deltaTime;
}

std::string ObjectSim::GetSimInfoToSaveImpl()
{
	std::string res = "";

	res += SimSerializer::GetValueToSaveFrom(scale);
	res += SimSerializer::GetValueToSaveFrom(front);
	res += SimSerializer::GetValueToSaveFrom(up);
	res += SimSerializer::GetValueToSaveFrom(pos);
	res += SimSerializer::GetValueToSaveFrom(lastPos);
	res += SimSerializer::GetValueToSaveFrom(rotate);
	res += SimSerializer::GetValueToSaveFrom(lastBlocker);
	res += SimSerializer::GetValueToSaveFrom(speed);
	res += SimSerializer::GetValueToSaveFrom(ghostMode);
	res += SimSerializer::GetValueToSaveFrom(lastDir);
	res += SimSerializer::GetValueToSaveFrom(disabledDirs);
	res += SimSerializer::GetValueToSaveFrom(rotationLimits);
	res += SimSerializer::GetValueToSaveFrom(scriptFuncStorage.GetAddedScriptDLLs());

	return res;
}

bool ObjectSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = true;

	res = res && SimSerializer::SetValueToLoadFrom(line, scale,                                   1);
	res = res && SimSerializer::SetValueToLoadFrom(line, front,                                   1);
	res = res && SimSerializer::SetValueToLoadFrom(line, up,                                      1);
	res = res && SimSerializer::SetValueToLoadFrom(line, pos,                                     1);
	res = res && SimSerializer::SetValueToLoadFrom(line, lastPos,                                 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, rotate,                                  1);
	res = res && SimSerializer::SetValueToLoadFrom(line, lastBlocker,                             1);
	res = res && SimSerializer::SetValueToLoadFrom(line, speed,                                   1);
	res = res && SimSerializer::SetValueToLoadFrom(line, ghostMode,                               1);
	res = res && SimSerializer::SetValueToLoadFrom(line, lastDir,                                 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, disabledDirs,                            1);
	res = res && SimSerializer::SetValueToLoadFrom(line, rotationLimits,                          1);
	res = res && SimSerializer::SetValueToLoadFrom(line, scriptFuncStorage.tempScriptDllNameVect, 2);

	return res;
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

	float correctedSpeed = speed * renderDeltaTime;

	switch (dir)
	{
	case Direction::Forward:
		pos += correctedSpeed * front * limitAxis;
		break;
	case Direction::Backward:
		pos -= correctedSpeed * front * limitAxis;
		break;
	case Direction::Left:
		pos -= correctedSpeed * glm::normalize(glm::cross(front, up));
		break;
	case Direction::Right:
		pos += correctedSpeed * glm::normalize(glm::cross(front, up));
		break;
	case Direction::Up:
		pos += glm::abs(correctedSpeed * glm::normalize(front * up));
		break;
	case Direction::Down:
		pos -= glm::abs(correctedSpeed * glm::normalize(front * up));
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

void ObjectSim::AddScriptFunc(
	const std::string& dllPath, 
	const std::string& dllName, 
	ScriptFuncStorage::ScriptFuncWeakPtr& scriptFunc
)
{
	scriptFuncStorage.AddScriptFunc(dllPath, dllName, scriptFunc);
}

std::vector<std::pair<std::string, std::string>> ObjectSim::GetTempScriptDLLInfo()
{
	return scriptFuncStorage.GetTempScriptDllNameVect();
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