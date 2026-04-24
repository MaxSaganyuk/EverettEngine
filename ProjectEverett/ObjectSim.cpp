#include "ObjectSim.h"
#include "EverettExceptionInternal.h"

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

ObjectSim::ObjectSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const float speed
)
	: pos(pos), scale(scale), speed(speed), visited(false), objectLinkingEnabled(true), 
	  orient({1.0f, 0.0f, 0.0f, 0.0f}), accumPos({})
{
	rotationLimits = {
		{-fullRotation, -fullRotation, -fullRotation},
		{ fullRotation,  fullRotation,  fullRotation}
	};

	lastPos = pos;

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

std::string ObjectSim::GetThisObjectTypeNameStr()
{
	ThrowExceptionWMessage("Impossible call, ObjectSim cannot be full type");
}

void ObjectSim::SetRenderDeltaTime(float deltaTime)
{
	renderDeltaTime = deltaTime;
}

std::string ObjectSim::GetSimInfoToSaveImpl()
{
	std::string res = "";

	res += SimSerializer::GetValueToSaveFrom(scale.GetValue());
	res += SimSerializer::GetValueToSaveFrom(pos.GetValue());
	res += SimSerializer::GetValueToSaveFrom(lastPos);
	res += SimSerializer::GetValueToSaveFrom(speed);
	res += SimSerializer::GetValueToSaveFrom(disabledDirs);
	res += SimSerializer::GetValueToSaveFrom(rotationLimits);
	res += SimSerializer::GetValueToSaveFrom(objectLinkingEnabled);
	res += SimSerializer::GetValueToSaveFrom(orient.GetValue());

	return res;
}

bool ObjectSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = true;
	bool legacyCompatibilityBool{};
	glm::vec3 legacyCompatibilityVect;
	glm::vec3 legacyRotationVectGetter{};
	Direction legacyCompatibilityDir{};
	std::vector<std::pair<std::string, std::string>> legacyCompatVectOfPairs{};

	res = res && SimSerializer::SetValueToLoadFrom(line, scale.GetValue(),                        1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityVect,                 1, 7);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityVect,                 1, 7);
	res = res && SimSerializer::SetValueToLoadFrom(line, pos.GetValue(), 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, lastPos,                                 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyRotationVectGetter,                1, 7);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityBool,                 1, 10);
	res = res && SimSerializer::SetValueToLoadFrom(line, speed,                                   1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityBool,                 1, 8);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityDir,                  1, 10);
	res = res && SimSerializer::SetValueToLoadFrom(line, disabledDirs,                            1);
	res = res && SimSerializer::SetValueToLoadFrom(line, rotationLimits,                          1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatVectOfPairs,                 2, 11);
	res = res && SimSerializer::SetValueToLoadFrom(line, objectLinkingEnabled,                    5);
	res = res && SimSerializer::SetValueToLoadFrom(line, orient.GetValue(),                       6);

	Rotation legacyRotationVect = legacyRotationVectGetter;

	if (!legacyRotationVect.Zeroed())
	{
		RotateImpl(legacyRotationVect);
	}

	return res;
}

void ObjectSim::CheckRotationLimits()
{
	// TODO: rotation recalculation 
}

const glm::vec3& ObjectSim::GetWorldAxisVector(int axis)
{
	switch (axis)
	{
	case 0:
		return worldRight;
	case 1:
		return worldUp;
	case 2:
		return worldFront;
	default:
		ThrowExceptionWMessage("Axis can only be 0, 1 or 2");
	}
}

void ObjectSim::SetMovementSpeed(float speed, bool executeLinkedObjects)
{
	this->speed = speed;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::SetMovementSpeed, speed, true);
	}
}

float ObjectSim::GetMovementSpeed()
{
	return speed;
}

void ObjectSim::InvertMovement(bool value, bool executeLinkedObjects)
{
	speed = (value ? (-1) : 1) * speed;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::InvertMovement, value, true);
	}
}

bool ObjectSim::IsMovementInverted()
{
	return speed < 0.0f;
}

void ObjectSim::SetPositionVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	pos = vect;

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::SetPositionVector, vect, true);
	}
}

void ObjectSim::SetScaleVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	scale = vect;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::SetScaleVector, vect, true);
	}
}

void ObjectSim::SetOrientation(const glm::quat& quat, bool executeLinkedObjects)
{
	orient = quat;

	if (rotationChangeCallback)
	{
		rotationChangeCallback();
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::SetOrientation, quat, true);
	}
}

glm::vec3 ObjectSim::GetFrontVector()
{
	const glm::quat& oreintRef = orient;
	return oreintRef * worldFront;
}

glm::vec3 ObjectSim::GetUpVector()
{
	const glm::quat& oreintRef = orient;
	return oreintRef * worldUp;
}

glm::vec3& ObjectSim::GetPositionVectorAddr()
{
	return pos;
}

glm::vec3& ObjectSim::GetScaleVectorAddr()
{
	return scale;
}

glm::quat& ObjectSim::GetOrientationAddr()
{
	return orient;
}

void ObjectSim::DisableDirection(Direction dir, bool executeLinkedObjects)
{
	disabledDirs[dir] = true;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::DisableDirection, dir, true);
	}
}

void ObjectSim::EnableDirection(Direction dir, bool executeLinkedObjects)
{
	disabledDirs[dir] = false;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::EnableDirection, dir, true);
	}
}

void ObjectSim::EnableAllDirections(bool executeLinkedObjects)
{
	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::EnableAllDirections, true);
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

void ObjectSim::UpdatePosition()
{
	lastPos = pos;
	pos += accumPos;
	accumPos = {};

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}
}

void ObjectSim::SetLastPosition(bool executeLinkedObjects)
{
	pos = lastPos;

	if (positionChangeCallback)
	{
		positionChangeCallback();
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::SetLastPosition, true);
	}
}

void ObjectSim::MoveInDirection(Direction dir, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	if (disabledDirs[dir])
	{
		return;
	}

	float correctedSpeed = speed * renderDeltaTime;

	switch (dir)
	{
	case Direction::Forward:
		accumPos += correctedSpeed * GetFrontVector() * limitAxis;
		break;
	case Direction::Backward:
		accumPos -= correctedSpeed * GetFrontVector() * limitAxis;
		break;
	case Direction::Left:
		accumPos -= correctedSpeed * glm::normalize(glm::cross(GetFrontVector(), GetUpVector()));
		break;
	case Direction::Right:
		accumPos += correctedSpeed * glm::normalize(glm::cross(GetFrontVector(), GetUpVector()));
		break;
	case Direction::Up:
		accumPos += glm::abs(correctedSpeed * glm::normalize(GetFrontVector() * GetUpVector()));
		break;
	case Direction::Down:
		accumPos -= glm::abs(correctedSpeed * glm::normalize(GetFrontVector() * GetUpVector()));
		break;
	default:
		ThrowExceptionWMessage("Undefined direction");
		return;
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::MoveInDirection, dir, limitAxis, true);
	}
}

void ObjectSim::MoveByAxis(const glm::vec3& axis, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	float correctedSpeed = speed * renderDeltaTime;

	accumPos += correctedSpeed * axis * limitAxis;

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::MoveByAxis, axis, limitAxis, executeLinkedObjects);
	}
}

void ObjectSim::LimitRotations(const Rotation& min, const Rotation& max, bool executeLinkedObjects)
{
	rotationLimits = { min, max };

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::LimitRotations, min, max, true);
	}
}

void ObjectSim::RotateImpl(const Rotation& toRotate)
{
	glm::quat pitch = glm::angleAxis(toRotate.GetPitch(), worldRight);
	glm::quat yaw   = glm::angleAxis(toRotate.GetYaw(),   worldUp);
	glm::quat roll  = glm::angleAxis(toRotate.GetRoll(),  worldFront);

	glm::quat& orientRef = orient;
	orient = glm::normalize(pitch * yaw * roll * orientRef);
}

void ObjectSim::Rotate(const Rotation& toRotate, bool executeLinkedObjects)
{
	if (!toRotate.Zeroed())
	{
		RotateImpl(toRotate);
	}

	if (rotationChangeCallback)
	{
		rotationChangeCallback();
	}

	if (objectLinkingEnabled && executeLinkedObjects)
	{
		ExecuteLinkedObjects(&ObjectSim::Rotate, toRotate, true);
	}
}

void ObjectSim::SetPositionChangeCallback(std::function<void()> callback)
{
	positionChangeCallback = callback;
}

void ObjectSim::SetRotationChangeCallback(std::function<void()> callback)
{
	rotationChangeCallback = callback;
}

void ObjectSim::LinkObject(IObjectSim& otherObject)
{
	objectGraph.SetupRelations(this, dynamic_cast<ObjectSim*>(&otherObject), stdEx::RelationType::LeftToRight);
}

void ObjectSim::UnlinkObject(IObjectSim& objectToUnlink)
{
	objectGraph.SetupRelations(this, dynamic_cast<ObjectSim*>(&objectToUnlink), stdEx::RelationType::NoRelation);
}

void ObjectSim::EnableObjectLinking(bool val)
{
	objectLinkingEnabled = val;
}