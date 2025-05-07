#include "LightSim.h"

std::map<int, LightSim::Attenuation> LightSim::attenuationVals
{
	{7,    {0.7f,    1.8f}      },
	{13,   {0.35f,   0.44f}     },
	{20,   {0.22f,   0.2f}      },
	{32,   {0.14f,   0.07f}     },
	{50,   {0.09f,   0.032f}    },
	{65,   {0.07f,   0.017f}    },
	{100,  {0.045f,  0.0075f}   },
	{160,  {0.027f,  0.0028f}   },
	{200,  {0.022f,  0.0019f}   },
	{325,  {0.014f,  0.0007f}   },
	{600,  {0.007f,  0.0002f}   },
	{3250, {0.0014f, 0.000007f} }
};

std::vector<std::pair<LightSim::LightTypes, std::string>> LightSim::lightTypeToName
{
	{ LightSim::LightTypes::Direction, "Direction" },
	{ LightSim::LightTypes::Point,     "Point"     },
	{ LightSim::LightTypes::Spot,      "Spot"      }
};

LightSim::LightSim(
	LightTypes lightType,
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed,
	const float range
) : ObjectSim(pos, scale, front, speed)
{
	this->lightType = lightType;
	this->lightRange = range;
}

std::string LightSim::GetObjectTypeNameStr()
{
	return "Light";
}

std::string LightSim::GetSimInfoToSave(const std::string& lightName)
{
	std::string info = GetObjectTypeNameStr() + '*' + lightName + '*';

	info += GetSimInfoToSaveImpl();

	return info + '\n';
}

std::string LightSim::GetSimInfoToSaveImpl()
{
	std::string res = ObjectSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(lightRange);

	return res;
}

void LightSim::SetSimInfoToLoad(std::string& line)
{
	ObjectSim::SetSimInfoToLoad(line);
	SimSerializer::SetValueToLoadFrom(line, lightRange);
}


LightSim::Attenuation LightSim::GetAttenuation(int range)
{
	for (const auto& attenuationVal : attenuationVals)
	{
		if (range < attenuationVal.first)
		{
			return attenuationVal.second;
		}
	}

	assert(false && "Unexpected Attenuation range value\n");
	return { 0.0f, 0.0f };
}

LightSim::Attenuation LightSim::GetAttenuation()
{
	return GetAttenuation(lightRange);
}

std::vector<std::string> LightSim::GetLightTypeNames()
{
	std::vector<std::string> lightTypeNamesVect;
	
	for(auto& lightType : lightTypeToName)
	{
		lightTypeNamesVect.push_back(lightType.second);
	}
	
	return lightTypeNamesVect;
}

LightSim::LightTypes LightSim::GetTypeToName(const std::string& name)
{
	for (auto& nameType : lightTypeToName)
	{
		if (nameType.second == name)
		{
			return nameType.first;
		}
	}

	assert(false && "Nonexistent type");
}

std::string LightSim::GetTypeToName(LightSim::LightTypes lightType)
{
	for (auto& nameType : lightTypeToName)
	{
		if (nameType.first == lightType)
		{
			return nameType.second;
		}
	}

	assert(false && "Nonexistent name");
}

std::string LightSim::GetCurrentLightType()
{
	return GetTypeToName(lightType);
}

