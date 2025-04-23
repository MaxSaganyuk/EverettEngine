#pragma once

#include <cassert>
#include <map>
#include <string>

#include "interfaces/ILightSim.h"
#include "SolidSim.h"

// Source: https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

class LightSim : public ObjectSim, public ILightSim
{
public:
	LightSim() = default;
	LightSim(
		LightTypes lightType,
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 0.0f,
		const float range = 60.0f
	);

	static std::vector<std::string> GetLightTypeNames();
	std::string GetCurrentLightType() override;

	static LightTypes GetTypeToName(const std::string& name);
	static std::string GetTypeToName(LightTypes lightType);

	int lightRange;

	static Attenuation GetAttenuation(int range);
	Attenuation GetAttenuation() override;
private:
	static std::map<int, Attenuation> attenuationVals;

	static std::vector<std::pair<LightTypes, std::string>> lightTypeToName;

	LightTypes lightType;
};