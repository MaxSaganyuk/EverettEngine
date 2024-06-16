#pragma once

#include "SolidSim.h"

class CameraSim : public SolidSim
{
public:
	enum class Mode
	{
		Fly,
		Walk
	};

private:
	int windowHeight;
	int windowWidth;
	glm::mat4 view;
	glm::mat4 projection;
	float fov;
	float sensitivity;

	float lastX;
	float lastY;

	Mode mode;
public:
	CameraSim(
		const int windowHeight,
		const int windowWidth,
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(0.35f, 0.35f, 0.35f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
		const float fov = 45.0f,
		const float speed = 0.006f
	);

	glm::mat4& GetViewMatrixAddr();
	glm::mat4& GetProjectionMatrixAddr();

	void SetPosition(Direction dir);
	void Rotate(float xpos, float ypos);
	void Zoom(float xpos, float ypos);
	void SetMode(Mode mode);
};