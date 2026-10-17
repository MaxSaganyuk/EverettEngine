#pragma once

#include "IObjectSim.h"

class ILightSim : virtual public IObjectSim
{
public:
	enum LightTypes
	{
		Direction,
		Point,
		Spot,
		_SIZE
	};

	struct Attenuation
	{
		float linear;
		float quadratic;
	};

	virtual LightTypes GetLightType() = 0;
	virtual std::string GetLightTypeStr() = 0;

	// Range is 0 to Pi / 2 radians
	virtual float GetInnerCutoff() = 0;
	virtual void SetInnerCutoff(float radians) = 0;
	virtual float GetOuterCutoff() = 0;
	virtual void SetOuterCutoff(float radians) = 0;

	virtual Attenuation GetAttenuation() = 0;
	virtual glm::vec3& GetAmbientLightColorVectorAddr() = 0;
	virtual glm::vec3& GetColorVectorAddr() = 0;
};
