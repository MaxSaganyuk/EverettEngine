#pragma once

#include "IObjectSim.h"

class ILightSim : virtual public IObjectSim
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
	virtual glm::vec3& GetAmbientLightColorVectorAddr() = 0;
	virtual glm::vec3& GetColorVectorAddr() = 0;
};
