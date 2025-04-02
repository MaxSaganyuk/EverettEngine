#pragma once

#include "ISolidSim.h"
#include "ILightSim.h"

class ILightSim : virtual public ISolidSim
{
public:
	enum LightTypes
	{
		Direction,
		Point,
		Spot
	};

	struct Attenuation
	{
		float linear;
		float quadratic;
	};

	virtual std::string GetCurrentLightType() = 0;

	virtual Attenuation GetAttenuation() = 0;
};
