#pragma once

#include "ObjectSim.h"

#include <cassert>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <functional>

#include "interfaces/ISolidSim.h"

class SolidSim : public ObjectSim, public ISolidSim
{
private:
	glm::mat4 model;
	SolidType type;

	void ResetModelMatrix(const Rotation& toRotate = {});
public:
	SolidSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 0.006f
	);

	glm::mat4& GetModelMatrixAddr() override;
	void ForceModelUpdate() override;
	void SetType(SolidType type) override;
	void SetPosition(ObjectSim::Direction dir, const glm::vec3& limitAxis) override;
	void Rotate(const Rotation& toRotate) override;
	
	static bool CheckForCollision(const SolidSim& solid1, const SolidSim& solid2);
	bool CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2) override;
};