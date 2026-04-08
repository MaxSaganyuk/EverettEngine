#include "CameraSim.h"

CameraSim::CameraSim(
	const int windowWidth,
	const int windowHeight,
	const glm::vec3& pos,
	const glm::vec3& scale,
	const float fov,
	const float speed
)
	: SolidSim(pos, scale, speed),
	  windowHeight(windowHeight), 
	  windowWidth(windowWidth), 
	  fov(fov)
{

	view = glm::mat4(1.0f);
	SetAspect(windowWidth, windowHeight);

	lastX = static_cast<float>(windowHeight);
	lastY = static_cast<float>(windowWidth);

	sensitivity = 0.001f;

	SetMode(Mode::Fly);

	SolidSim::LimitRotations(
		{ -fullRotation, -fullRotation / 4, -fullRotation},
		{  fullRotation,  fullRotation / 4,  fullRotation}
	);

	SolidSim::SetType(SolidType::Dynamic);
}

std::string CameraSim::GetThisObjectTypeNameStr()
{
	return TypeName;
}

std::string CameraSim::GetObjectTypeNameStr()
{
	return TypeName;
}

void CameraSim::SetAspect()
{
	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

	projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.f);
}

void CameraSim::SetAspect(const int windowWidth, const int windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	SetAspect();
}

std::string CameraSim::GetSimInfoToSaveImpl()
{
	std::string res = SolidSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(view);
	res += SimSerializer::GetValueToSaveFrom(projection);
	res += SimSerializer::GetValueToSaveFrom(fov);
	res += SimSerializer::GetValueToSaveFrom(sensitivity);
	res += SimSerializer::GetValueToSaveFrom(mode);

	return res;
}

std::string CameraSim::GetSimInfoToSave(const std::string&)
{
	std::string info = GetObjectTypeNameStr() + '*';

	info += GetSimInfoToSaveImpl();

	return info + '\n';
}

bool CameraSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = SolidSim::SetSimInfoToLoad(line);
	
	res = res && SimSerializer::SetValueToLoadFrom(line, view,        1);
	res = res && SimSerializer::SetValueToLoadFrom(line, projection,  1);
	res = res && SimSerializer::SetValueToLoadFrom(line, fov,         1);
	res = res && SimSerializer::SetValueToLoadFrom(line, sensitivity, 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, mode,        1);

	return res;
}

glm::mat4& CameraSim::GetViewMatrixAddr()
{
	return view;
}

glm::mat4& CameraSim::GetProjectionMatrixAddr()
{
	return projection;
}

void CameraSim::UpdateViewMatrix()
{
	const glm::vec3& pos = GetPositionVectorAddr();

	view = glm::lookAt(pos, pos + GetFrontVector(), GetUpVector());
}

void CameraSim::ForceModelUpdate()
{
	SolidSim::ForceModelUpdate();
	UpdateViewMatrix();
}

void CameraSim::MoveInDirection(Direction dir, const glm::vec3& axisToLimit, bool executeLinkedObjects)
{
	SolidSim::MoveInDirection(dir, cameraAxisLimit, executeLinkedObjects);
	UpdateViewMatrix();
}

void CameraSim::MoveByAxis(const glm::vec3& axis, const glm::vec3& axisToLimit, bool executeLinkedObjects)
{
	SolidSim::MoveByAxis(axis, cameraAxisLimit, executeLinkedObjects);
	UpdateViewMatrix();
}

void CameraSim::RotateByMousePos(float xpos, float ypos)
{
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	glm::quat& orientRef = orient;

	glm::quat yaw = glm::angleAxis(-xoffset, worldUp);
	orientRef = yaw * orientRef;
	glm::vec3 right = orientRef * worldRight;
	glm::quat pitch = glm::angleAxis(-yoffset, right);
	glm::quat orientTest = pitch * orientRef;

	if (glm::abs(glm::dot(orientTest * worldFront, worldUp)) < 0.99f)
	{
		orientRef = orientTest;
	}

	orient = glm::normalize(orientRef);
	SolidSim::Rotate({});
	UpdateViewMatrix();
}

void CameraSim::Zoom(float xpos, float ypos)
{
	if (fov - ypos > 1.0f && fov - ypos < 45.0f)
	{
		fov -= ypos;
	}

	SetAspect();
}

void CameraSim::SetMode(Mode mode)
{
	this->mode = mode;
	cameraAxisLimit = { 1.0f, static_cast<float>(mode == Mode::Fly), 1.0f };
}