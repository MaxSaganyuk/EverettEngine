#include "LightSim.h"
#include "EverettExceptionInternal.h"

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
	const float speed,
	const float range
) : 
	lightType(lightType),
	lightRange(static_cast<int>(range)),
	color(defaultLightColor),
	ObjectSim(pos, scale, speed)
{
	++amountOfLightsByType[lightType];
	++amountOfActiveLightsByType[lightType];
}

LightSim::~LightSim()
{
	--amountOfLightsByType[lightType];

	if (enabled)
	{
		--amountOfActiveLightsByType[lightType];
	}
}

std::string LightSim::GetThisObjectTypeNameStr()
{
	return TypeName;
}

std::string LightSim::GetObjectTypeNameStr()
{
	return TypeName;
}

void LightSim::SetEnable(bool value, bool executeLinkedObjects)
{
	if (value != enabled)
	{
		amountOfActiveLightsByType[lightType] += (value ? 1 : -1);
	}

	ObjectSim::SetEnable(value, executeLinkedObjects);
}

std::string LightSim::GetSimInfoToSave(const std::string& lightName)
{
	std::string info = GetObjectTypeNameStr() + '*' + lightName + '*';

	info += GetSimInfoToSaveImpl();

	return info + '\n';
}

void LightSim::CheckForSpotLightType()
{
	if (lightType != LightTypes::Spot)
	{
		std::cerr << "Retrieved inner cutoff value from non spot light\n";
	}
}

void LightSim::AdjustCutoffRange(float& radians)
{
	if (radians < 0.0f)
	{
		radians = 0.0f;
	}
	else if (radians > glm::pi<float>() / 2)
	{
		radians = glm::pi<float>() / 2;
	}
}

std::string LightSim::GetSimInfoToSaveImpl()
{
	std::string res = ObjectSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(lightRange);
	res += SimSerializer::GetValueToSaveFrom(color);
	res += SimSerializer::GetValueToSaveFrom(innerCutoffRad);
	res += SimSerializer::GetValueToSaveFrom(outerCutoffRad);

	return res;
}

bool LightSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = ObjectSim::SetSimInfoToLoad(line);
	
	res = res && SimSerializer::SetValueToLoadFrom(line, lightRange, "May 26 2025"_ver);
	res = res && SimSerializer::SetValueToLoadFrom(line, color, "Aug 15 2025"_ver);
	res = res && SimSerializer::SetValueToLoadFrom(line, innerCutoffRad, "Sep 20 2026"_ver);
	res = res && SimSerializer::SetValueToLoadFrom(line, outerCutoffRad, "Sep 20 2026"_ver);
	SimSerializer::SkipDeprecatedValue(line, "Oct 15 2026"_ver, "Oct 17 2026"_ver);

	if (!enabled)
	{
		--amountOfActiveLightsByType[lightType];
	}

	return res;
}

LightSim::LightTypes LightSim::GetLightType()
{
	return lightType;
}

size_t LightSim::GetAmountOfLightsByType(LightTypes lightType, bool activeOnly)
{
	return activeOnly ? amountOfActiveLightsByType[lightType] : amountOfLightsByType[lightType];
}

float LightSim::GetInnerCutoff()
{
	CheckForSpotLightType();

	return innerCutoffRad;
}

void LightSim::SetInnerCutoff(float radians)
{
	CheckForSpotLightType();
	AdjustCutoffRange(radians);

	innerCutoffRad = radians;
}

float LightSim::GetOuterCutoff()
{
	CheckForSpotLightType();

	return outerCutoffRad;
}

void LightSim::SetOuterCutoff(float radians)
{
	CheckForSpotLightType();
	AdjustCutoffRange(radians);

	if (radians < innerCutoffRad)
	{
		radians = innerCutoffRad;
	}

	outerCutoffRad = radians;
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

std::generator<std::string_view> LightSim::GetLightTypeNames()
{
	for (auto& [_, lightTypeName] : lightTypeToName)
	{
		co_yield lightTypeName;
	}
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

std::string LightSim::GetLightTypeStr()
{
	return GetTypeToName(lightType);
}

glm::vec3& LightSim::SGetAmbientLightColorVectorAddr()
{
	return ambientLightColor;
}

glm::vec3& LightSim::GetAmbientLightColorVectorAddr()
{
	return ambientLightColor;
}

glm::vec3& LightSim::GetColorVectorAddr()
{
	return color;
}

