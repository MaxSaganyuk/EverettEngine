#include "ObjectSim.h"
#include "EverettException.h"

template<typename MemberFuncType, typename... ParamTypes>
void ObjectSim::ExecuteLinkedObjects(MemberFuncType memberFunc, ParamTypes&&... values)
{
	if (visited)
	{
		return;
	}
	visited = true;

	for (ObjectSim* linkedObject : objectGraph.GetValuesRelatedTo(this))
	{
		(linkedObject->*memberFunc)(std::forward<ParamTypes>(values)...);
	}

	visited = false;
}

void ObjectSim::UpdateFrontVector()
{
	CheckRotationLimits();

	glm::vec3 direction;
	const Rotation& roateRef = rotate;
	direction.x = cos(roateRef.GetPitch()) * sin(roateRef.GetYaw());
	direction.y = sin(roateRef.GetPitch());
	direction.z = cos(roateRef.GetPitch()) * cos(roateRef.GetYaw());
	front = glm::normalize(direction);
}

ObjectSim::ObjectSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed
)
	: front(front), pos(pos), scale(scale), speed(speed), rotate({}), visited(false)
{
	rotationLimits = {
		{-fullRotation, -fullRotation, -fullRotation},
		{ fullRotation,  fullRotation,  fullRotation}
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

ObjectSim::~ObjectSim()
{
	objectGraph.RemoveElement(this);
}

void ObjectSim::InitializeObjectGraph()
{
	objectGraph.EnableBidirectionality(false);
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

void ObjectSim::SetMovementSpeed(float speed)
{
	this->speed = speed;

	ExecuteLinkedObjects(&ObjectSim::SetMovementSpeed, speed);
}

float ObjectSim::GetMovementSpeed()
{
	return speed;
}

void ObjectSim::InvertMovement(bool value)
{
	speed = (value ? (-1) : 1) * speed;

	ExecuteLinkedObjects(&ObjectSim::InvertMovement, value);
}

bool ObjectSim::IsMovementInverted()
{
	return speed < 0.0f;
}

void ObjectSim::SetPositionVector(const glm::vec3& vect)
{
	pos = vect;

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}

	ExecuteLinkedObjects(&ObjectSim::SetPositionVector, vect);
}

void ObjectSim::SetScaleVector(const glm::vec3& vect)
{
	scale = vect;

	ExecuteLinkedObjects(&ObjectSim::SetScaleVector, vect);
}

void ObjectSim::SetRotationVector(const Rotation& vect)
{
	rotate = vect;

	UpdateFrontVector();

	if (rotationChangeCallback)
	{
		rotationChangeCallback();
	}

	ExecuteLinkedObjects(&ObjectSim::SetRotationVector, vect);
}

const glm::vec3& ObjectSim::GetFrontVectorAddr()
{
	return front;
}

const glm::vec3& ObjectSim::GetUpVectorAddr()
{
	return up;
}

glm::vec3& ObjectSim::GetPositionVectorAddr()
{
	return pos;
}

glm::vec3& ObjectSim::GetScaleVectorAddr()
{
	return scale;
}

void ObjectSim::DisableDirection(Direction dir)
{
	disabledDirs[dir] = true;

	ExecuteLinkedObjects(&ObjectSim::DisableDirection, dir);
}

void ObjectSim::EnableDirection(Direction dir)
{
	disabledDirs[dir] = false;

	ExecuteLinkedObjects(&ObjectSim::EnableDirection, dir);
}

void ObjectSim::EnableAllDirections()
{
	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}

	ExecuteLinkedObjects(&ObjectSim::EnableAllDirections);
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

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}

	ExecuteLinkedObjects(&ObjectSim::SetLastPosition);
}

void ObjectSim::SetPosition(Direction dir, const glm::vec3& limitAxis)
{
	if (disabledDirs[dir])
	{
		return;
	}

	if (!lastBlocker)
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
	default:
		ThrowExceptionWMessage("Undefined direction");
		return;
	}

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}

	ExecuteLinkedObjects(&ObjectSim::SetPosition, dir, limitAxis);
}

void ObjectSim::LimitRotations(const Rotation& min, const Rotation& max)
{
	rotationLimits = { min, max };

	ExecuteLinkedObjects(&ObjectSim::LimitRotations, min, max);
}

void ObjectSim::Rotate(const Rotation& toRotate)
{
	UpdateFrontVector();

	if (rotationChangeCallback)
	{
		rotationChangeCallback();
	}

	ExecuteLinkedObjects(&ObjectSim::Rotate, toRotate);
}

void ObjectSim::AddScriptFunc(
	const std::string& dllPath, 
	const std::string& dllName, 
	ScriptFuncStorage::ScriptFuncWeakPtr& scriptFunc
)
{
	scriptFuncStorage.AddScriptFunc(dllPath, dllName, scriptFunc);
}

void ObjectSim::ClearScriptFuncMap()
{
	scriptFuncStorage.ClearScriptFuncMap();
}

std::vector<std::pair<std::string, std::string>> ObjectSim::GetTempScriptDLLInfo()
{
	return scriptFuncStorage.GetTempScriptDllNameVect();
}

void ObjectSim::SetPositionChangeCallback(std::function<void()> callback)
{
	positionChangeCallback = callback;
}

void ObjectSim::SetRotationChangeCallback(std::function<void()> callback)
{
	rotationChangeCallback = callback;
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

void ObjectSim::LinkObject(IObjectSim& otherObject)
{
	objectGraph.AddElements(this, dynamic_cast<ObjectSim*>(&otherObject), stdEx::RelationType::LeftToRight);
}