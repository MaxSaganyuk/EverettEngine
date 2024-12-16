#pragma once

#include <cassert>
#include <map>
#include <string>

#include "SolidSim.h"

// Source: https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

class LightSim : public SolidSim
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

	LightSim(
		LightTypes lightType,
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 0.0f,
		const float range = 60.0f
	);

	static std::vector<std::string> GetLightTypeNames();
	static std::map<LightTypes, std::string>& GetLightTypeNameMap();
	std::string GetCurrentLightType();

	int lightRange;

	static Attenuation GetAttenuation(int range);
	Attenuation GetAttenuation();
private:
	static std::map<int, Attenuation> attenuationVals;
	static std::map<LightTypes, std::string> lightTypeNames;

	LightTypes lightType;
};