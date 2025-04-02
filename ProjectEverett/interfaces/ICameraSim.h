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

	virtual glm::mat4& GetViewMatrixAddr() = 0;
	virtual glm::mat4& GetProjectionMatrixAddr() = 0;

	virtual void SetPosition(Direction dir) = 0;
	virtual void Rotate(float xpos, float ypos) = 0;
	virtual void Zoom(float xpos, float ypos) = 0;
	virtual void SetMode(Mode mode) = 0;
};
