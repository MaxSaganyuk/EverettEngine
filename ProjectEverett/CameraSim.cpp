#include "CameraSim.h"

CameraSim::CameraSim(
	const int windowHeight,
	const int windowWidth,
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
	view = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(fov), static_cast<float>(windowHeight / windowWidth), 0.1f, 100.f);

	lastX = static_cast<float>(windowHeight);
	lastY = static_cast<float>(windowWidth);

	sensitivity = 0.05f;

	mode = Mode::Fly;

	SolidSim::LimitRotations(
		{ -fullRotation, -90.0f, -fullRotation },
		{  fullRotation,  90.0f,  fullRotation}
	);

	SolidSim::SetType(SolidType::Dynamic);
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
	glm::vec3& pos = SolidSim::GetPositionVectorAddr();
	glm::vec3& front = SolidSim::GetFrontVectorAddr();

	SolidSim::SetPosition(dir, { 1.0f, static_cast<float>(mode == Mode::Fly), 1.0f });

	view = glm::lookAt(pos, pos + front, GetUpVectorAddr());
	projection = glm::perspective(glm::radians(fov), static_cast<float>(windowHeight / windowWidth), 0.1f, 100.f);
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