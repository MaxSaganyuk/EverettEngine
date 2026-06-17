#pragma once

#include "IObjectSim.h"

class ICameraSim : virtual public IObjectSim
{
public:
	enum class Mode
	{
		Fly,
		Walk
	};

	virtual void SetMode(Mode mode) = 0;
	virtual void Zoom(float valueDelta) = 0;
	virtual float GetFOV() = 0;
	virtual void SetFOV(float fov) = 0;
	virtual float& GetSensitivityAddr() = 0;
};
