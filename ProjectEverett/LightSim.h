#pragma once

#include <cassert>
#include <map>

// Source: https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

class LightSim
{
public:
	struct Attenuation
	{
		float linear;
		float quadratic;
	};

	static Attenuation GetAttenuation(int range)
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
private:
	static std::map<int, Attenuation> attenuationVals;
};

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