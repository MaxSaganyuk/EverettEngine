#include "CameraSim.h"

CameraSim::CameraSim(
	const int windowWidth,
	const int windowHeight,
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float fov,
	const float speed
)
	: SolidSim(pos, scale, front, speed),
	  windowHeight(windowHeight), 
	  windowWidth(windowWidth), 
	  fov(fov)
{
	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

	view = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(fov), aspect, 0.1f,  100.f);

	lastX = static_cast<float>(windowHeight);
	lastY = static_cast<float>(windowWidth);

	sensitivity = 0.1f;

	mode = Mode::Fly;

	SolidSim::LimitRotations(
		{ -fullRotation, -90.0f, -fullRotation },
		{  fullRotation,  90.0f,  fullRotation}
	);

	SolidSim::SetType(SolidType::Dynamic);
}

std::string CameraSim::GetObjectTypeNameStr()
{
	return "Camera";
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

void CameraSim::SetPosition(Direction dir)
{
	glm::vec3& pos = ObjectSim::GetPositionVectorAddr();
	glm::vec3& front = ObjectSim::GetFrontVectorAddr();

	SolidSim::SetPosition(dir, { 1.0f, static_cast<float>(mode == Mode::Fly), 1.0f });

	view = glm::lookAt(pos, pos + front, GetUpVectorAddr());
}

void CameraSim::Rotate(float xpos, float ypos)
{
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	SolidSim::Rotate({ xoffset, yoffset, 0.0f });
}

void CameraSim::Zoom(float xpos, float ypos)
{
	if (fov - ypos > 1.0f && fov - ypos < 45.0f)
	{
		fov -= ypos;
	}
}

void CameraSim::SetMode(Mode mode)
{
	this->mode = mode;
}