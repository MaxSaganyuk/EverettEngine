#pragma once

#include "SolidSim.h"
#include "interfaces/ICameraSim.h"

class CameraSim : public SolidSim, public ICameraSim
{
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

	std::string GetSimInfoToSaveImpl();
public:
	CameraSim(
		const int windowWidth,
		const int windowHeight,
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(0.35f, 0.35f, 0.35f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float fov = 45.0f,
		const float speed = 1.0f
	);

	static std::string GetObjectTypeNameStr();

	std::string GetSimInfoToSave(const std::string&);
	bool SetSimInfoToLoad(std::string_view& line);

	glm::mat4& GetViewMatrixAddr() override;
	glm::mat4& GetProjectionMatrixAddr() override;

	void SetPosition(Direction dir) override;
	void Rotate(float xpos, float ypos) override;
	void Zoom(float xpos, float ypos) override;
	void SetMode(Mode mode) override;
};