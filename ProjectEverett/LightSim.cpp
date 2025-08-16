#include "LightSim.h"
#include "EverettException.h"

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

LightSim::LightSim(
	LightTypes lightType,
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed,
	const float range
) : 
	lightType(lightType),
	lightRange(static_cast<int>(range)),
	color({0.5f, 0.5f, 0.5f}),
	ObjectSim(pos, scale, front, speed)
{
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
	res += SimSerializer::GetValueToSaveFrom(color);

	return res;
}

bool LightSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = ObjectSim::SetSimInfoToLoad(line);
	
	res = res && SimSerializer::SetValueToLoadFrom(line, lightRange, 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, color, 3);

	return res;
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

	ThrowExceptionWMessage("Unexpected Attenuation range value");
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

	ThrowExceptionWMessage("Nonexistent type");
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

	ThrowExceptionWMessage("Nonexistent type");
}

std::string LightSim::GetCurrentLightType()
{
	return GetTypeToName(lightType);
}

glm::vec3& LightSim::GetColorVectorAddr()
{
	return color;
}

