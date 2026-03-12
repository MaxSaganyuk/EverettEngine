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

	virtual void SetMode(Mode mode) = 0;
};
