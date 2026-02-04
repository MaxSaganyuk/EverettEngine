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
	glm::vec3 cameraAxisLimit; // Depends on the mode, no need to save

	std::string GetSimInfoToSaveImpl();

	void UpdateViewMatrix();
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

	void SetAspect(const int windowWidth, const int windowHeight);

	glm::mat4& GetViewMatrixAddr();
	glm::mat4& GetProjectionMatrixAddr();

	void RotateByMousePos(float xpos, float ypos);
	void Zoom(float xpos, float ypos);

	void MoveInDirection(
		Direction dir, const glm::vec3& axisToLimit = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) override;
	void MoveByAxis(
		const glm::vec3& axis, const glm::vec3& axisToLimit = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) override;
	void SetMode(Mode mode) override;
};