#include "ObjectSim.h"
#include "EverettExceptionInternal.h"

template<ObjectSim::LinkableFuncNames funcName, typename MemberFuncType, typename... ParamTypes>
void ObjectSim::ExecuteLinkedObjects(MemberFuncType memberFunc, bool executeLinkedObjs, ParamTypes&&... values)
{
	constexpr static auto funcNameID = std::to_underlying(funcName);

	if (visited) return;

	visited = true;

	// Caused by necessity of ViewValuesRelatedTo to accept l-value to guarantee that passed
	// value will outlive the call for the co routine and "this" is a pr-value
	// There must be another way of doing this
	ObjectSim* thisPtr = this;

	for (auto&& [isHardLink, linkedObject] : objectGraph.ViewValuesRelatedTo(thisPtr))
	{
		if (isHardLink || 
			(executeLinkedObjs && objectLinkingEnabled && linkedObject->objectLinkingForFuncTracker[funcNameID])
		)
		{
			(linkedObject->*memberFunc)(std::forward<ParamTypes>(values)...);
		}
	}

	visited = false;
}

ObjectSim::ObjectSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const float speed
)
	: pos(pos), scale(scale), speed(speed), visited(false), objectLinkingEnabled(true), 
	  orient({1.0f, 0.0f, 0.0f, 0.0f})
{
	rotationLimits = {
		{-fullRotation, -fullRotation, -fullRotation},
		{ fullRotation,  fullRotation,  fullRotation}
	};

	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}

	std::fill(objectLinkingForFuncTracker.begin(), objectLinkingForFuncTracker.end(), true);
}

ObjectSim::~ObjectSim()
{
	objectGraph.RemoveElement(this);
}

void ObjectSim::InitializeObjectGraph()
{
	objectGraph.EnableBidirectionality(false);
}

void ObjectSim::ResetObjectLinking()
{
	objectGraph.EraseAllRelations();
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
	res = res && SimSerializer::SetValueToLoadFrom(line, pos.GetValue(),                          1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyCompatibilityVect,                 1, 15);
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

const glm::vec3& ObjectSim::GetWorldAxisVector(Axis axis)
{
	switch (axis)
	{
	case Axis::X:
		return worldRight;
	case Axis::Y:
		return worldUp;
	case Axis::Z:
		return worldFront;
	default:
		std::unreachable();
	}
}

void ObjectSim::SetMovementSpeed(float speed, bool executeLinkedObjects)
{
	this->speed = speed;

	ExecuteLinkedObjects<LinkableFuncNames::SetMovementSpeed>(
		&ObjectSim::SetMovementSpeed, executeLinkedObjects, speed, true
	);
}

float ObjectSim::GetMovementSpeed()
{
	return speed;
}

void ObjectSim::InvertMovement(bool value, bool executeLinkedObjects)
{
	speed = (value ? (-1) : 1) * speed;

	ExecuteLinkedObjects<LinkableFuncNames::InvertMovement>(
		&ObjectSim::InvertMovement, executeLinkedObjects, value, true
	);
}

bool ObjectSim::IsMovementInverted()
{
	return speed < 0.0f;
}

void ObjectSim::SetPositionVector(IObjectSim& obj, bool executeLinkedObjects)
{
	SetPositionVector(obj.GetPositionVectorAddr(), executeLinkedObjects);
}

void ObjectSim::SetScaleVector(IObjectSim& obj, bool executeLinkedObjects)
{
	SetScaleVector(obj.GetScaleVectorAddr(), executeLinkedObjects);
}

void ObjectSim::SetOrientation(IObjectSim& obj, bool executeLinkedObjects)
{
	SetOrientation(obj.GetOrientationAddr(), executeLinkedObjects);
}

void ObjectSim::SetPositionVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	pos = vect;

	ExecuteLinkedObjects<LinkableFuncNames::SetPositionVector>(
		static_cast<void(ObjectSim::*)(const glm::vec3&, bool)>(&ObjectSim::SetPositionVector), executeLinkedObjects, 
		vect, true
	);
}

void ObjectSim::SetScaleVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	scale = vect;

	ExecuteLinkedObjects<LinkableFuncNames::SetScaleVector>(
		static_cast<void(ObjectSim::*)(const glm::vec3&, bool)>(&ObjectSim::SetScaleVector), executeLinkedObjects, 
		vect, true
	);
}

void ObjectSim::SetOrientation(const glm::quat& quat, bool executeLinkedObjects)
{
	orient = quat;

	ExecuteLinkedObjects<LinkableFuncNames::SetOrientation>(
		static_cast<void(ObjectSim::*)(const glm::quat&, bool)>(&ObjectSim::SetOrientation), executeLinkedObjects, 
		quat, true
	);
}

void ObjectSim::SetTransform(IObjectSim& obj, bool executeLinkedObjects)
{
	pos = obj.GetPositionVectorAddr();
	scale = obj.GetScaleVectorAddr();
	orient = obj.GetOrientationAddr();

	ExecuteLinkedObjects<LinkableFuncNames::SetTransform>(
		&ObjectSim::SetTransform, executeLinkedObjects, obj, true
	);
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

const glm::vec3& ObjectSim::GetPositionVectorAddr()
{
	return pos;
}

const glm::vec3& ObjectSim::GetScaleVectorAddr()
{
	return scale;
}

const glm::quat& ObjectSim::GetOrientationAddr()
{
	return orient;
}

void ObjectSim::DisableDirection(Direction dir, bool executeLinkedObjects)
{
	disabledDirs[dir] = true;

	ExecuteLinkedObjects<LinkableFuncNames::DisableDirection>(
		&ObjectSim::DisableDirection, executeLinkedObjects, dir, true
	);
}

void ObjectSim::EnableDirection(Direction dir, bool executeLinkedObjects)
{
	disabledDirs[dir] = false;

	ExecuteLinkedObjects<LinkableFuncNames::EnableDirection>(
		&ObjectSim::EnableDirection, executeLinkedObjects, dir, true
	);
}

void ObjectSim::EnableAllDirections(bool executeLinkedObjects)
{
	for (size_t i = 0; i < realDirectionAmount; ++i)
	{
		disabledDirs[static_cast<Direction>(i)] = false;
	}

	ExecuteLinkedObjects<LinkableFuncNames::EnableAllDirections>(
		&ObjectSim::EnableAllDirections, executeLinkedObjects, true
	);
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

bool ObjectSim::UpdateTransform()
{
	// Bitwise is intended, yes. Check all and if any changed return true
	return pos.Update() | scale.Update() | orient.Update();
}

void ObjectSim::SetLastPosition(bool executeLinkedObjects)
{
	pos.SetLastValue();

	ExecuteLinkedObjects<LinkableFuncNames::SetLastPosition>(
		&ObjectSim::SetLastPosition, executeLinkedObjects, true
	);
}

void ObjectSim::SetLastScale(bool executeLinkedObjects)
{
	scale.SetLastValue();

	ExecuteLinkedObjects<LinkableFuncNames::SetLastScale>(
		&ObjectSim::SetLastScale, executeLinkedObjects, true
	);
}

void ObjectSim::SetLastOrientation(bool executeLinkedObjects)
{
	orient.SetLastValue();

	ExecuteLinkedObjects<LinkableFuncNames::SetLastOrientation>(
		&ObjectSim::SetLastOrientation, executeLinkedObjects, true
	);
}

void ObjectSim::SetLastTransform(bool executeLinkedObjects)
{
	pos.SetLastValue();
	scale.SetLastValue();
	orient.SetLastValue();

	ExecuteLinkedObjects<LinkableFuncNames::SetLastTransform>(
		&ObjectSim::SetLastTransform, executeLinkedObjects, true
	);
}

void ObjectSim::MoveInDirection(Direction dir, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	if (disabledDirs[dir]) return;

	float correctedSpeed = speed * renderDeltaTime;

	switch (dir)
	{
	case Direction::Forward:
		pos += correctedSpeed * GetFrontVector() * limitAxis;
		break;
	case Direction::Backward:
		pos -= correctedSpeed * GetFrontVector() * limitAxis;
		break;
	case Direction::Left:
		pos -= correctedSpeed * glm::normalize(glm::cross(GetFrontVector(), GetUpVector()));
		break;
	case Direction::Right:
		pos += correctedSpeed * glm::normalize(glm::cross(GetFrontVector(), GetUpVector()));
		break;
	case Direction::Up:
		pos += glm::abs(correctedSpeed * glm::normalize(GetFrontVector() * GetUpVector()));
		break;
	case Direction::Down:
		pos -= glm::abs(correctedSpeed * glm::normalize(GetFrontVector() * GetUpVector()));
		break;
	default:
		std::unreachable();
	}

	ExecuteLinkedObjects<LinkableFuncNames::MoveInDirection>(
		&ObjectSim::MoveInDirection, executeLinkedObjects, dir, limitAxis, true
	);
}

void ObjectSim::MoveByAxis(const glm::vec3& axis, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	float correctedSpeed = speed * renderDeltaTime;

	pos += correctedSpeed * axis * limitAxis;

	ExecuteLinkedObjects<LinkableFuncNames::MoveByAxis>(
		static_cast<void(ObjectSim::*)(const glm::vec3&, const glm::vec3&, bool)>(&ObjectSim::MoveByAxis), 
		executeLinkedObjects, axis, limitAxis, executeLinkedObjects
	);
}

void ObjectSim::MoveByAxis(Axis axis, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	MoveByAxis(GetWorldAxisVector(axis), limitAxis, executeLinkedObjects);
}

void ObjectSim::LimitRotations(const Rotation& min, const Rotation& max, bool executeLinkedObjects)
{
	rotationLimits = { min, max };

	ExecuteLinkedObjects<LinkableFuncNames::LimitRotations>(
		&ObjectSim::LimitRotations, executeLinkedObjects, min, max, true
	);
}

void ObjectSim::RotateImpl(const Rotation& toRotate)
{
	orient += (glm::normalize(CalcOrientationFromRotation(toRotate) * orient.GetValue()) - orient.GetValue());
}

glm::quat ObjectSim::CalcOrientationFromRotation(const Rotation& toRotate)
{
	glm::quat pitch = glm::angleAxis(toRotate.GetPitch(), worldRight);
	glm::quat yaw   = glm::angleAxis(toRotate.GetYaw(),   worldUp);
	glm::quat roll  = glm::angleAxis(toRotate.GetRoll(),  worldFront);

	return pitch * yaw * roll;
}

void ObjectSim::Rotate(const Rotation& toRotate, bool executeLinkedObjects)
{
	if (!toRotate.Zeroed())
	{
		RotateImpl(toRotate);
	}

	ExecuteLinkedObjects<LinkableFuncNames::Rotate>(
		&ObjectSim::Rotate, executeLinkedObjects, toRotate, true
	);
}

void ObjectSim::RevolveAround(const Rotation& toRotate, IObjectSim& obj, bool executeLinkedObjects)
{
	RevolveAround(toRotate, obj.GetPositionVectorAddr(), executeLinkedObjects);
}

void ObjectSim::RevolveAround(const Rotation& toRotate, const glm::vec3& centerPos, bool executeLinkedObjects)
{
	glm::vec3 offset = CalcOrientationFromRotation(toRotate) * (pos.GetValue() - centerPos);

	pos += (centerPos + offset) - pos.GetValue();

	ExecuteLinkedObjects<LinkableFuncNames::RevolveAround>(
		static_cast<void(ObjectSim::*)(const Rotation&, const glm::vec3&, bool)>(&ObjectSim::RevolveAround), 
		executeLinkedObjects, toRotate, centerPos, true
	);
}

void ObjectSim::LookAt(IObjectSim& obj, bool executeLinkedObjects)
{
	LookAt(obj.GetPositionVectorAddr(), executeLinkedObjects);
}

void ObjectSim::LookAt(const glm::vec3& pointToLookAt, bool executeLinkedObjects)
{
	orient += glm::quatLookAt(glm::normalize(pos.GetValue() - pointToLookAt), worldUp) - orient.GetValue();

	ExecuteLinkedObjects<LinkableFuncNames::LookAt>(
		static_cast<void(ObjectSim::*)(const glm::vec3&, bool)>(&ObjectSim::LookAt), executeLinkedObjects, 
		pointToLookAt, true
	);
}

void ObjectSim::SetPositionChangeCallback(std::function<void()> callback)
{
	pos.SetValueUpdateCallback(std::move(callback));
}

void ObjectSim::SetScaleChangeCallback(std::function<void()> callback)
{
	scale.SetValueUpdateCallback(std::move(callback));
}

void ObjectSim::SetRotationChangeCallback(std::function<void()> callback)
{
	orient.SetValueUpdateCallback(std::move(callback));
}

void ObjectSim::HardLinkObject(IObjectSim& otherObject)
{
	objectGraph.SetupRelations(this, dynamic_cast<ObjectSim*>(&otherObject), stdEx::RelationType::LeftToRight, true);
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

void ObjectSim::EnableLinkedExecutionForFunc(LinkableFuncNames linkFuncName, bool value)
{
	objectLinkingForFuncTracker[std::to_underlying(linkFuncName)] = value;
}
