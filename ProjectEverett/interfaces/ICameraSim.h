#pragma once

#include "ISolidSim.h"

class ICameraSim : virtual public ISolidSim
{
public:
	enum class Mode
	{
		Fly,
		Walk
	};

	Linkable virtual void MoveInDirection(
		Direction dir, const glm::vec3& axisToLimit = { 1.0f, 1.0f, 1.0f }, bool executeLinkedObjects = true
	) = 0;
	Linkable virtual void MoveByAxis(
		const glm::vec3& axis, const glm::vec3& axisToLimit = { 1.0f, 1.0f, 1.0f }, bool exeuteLinkedObject = true
	) = 0;
	virtual void SetMode(Mode mode) = 0;
};
