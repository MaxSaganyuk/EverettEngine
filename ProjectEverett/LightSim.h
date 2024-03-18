#pragma once

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
		for (auto& attenuationVal : attenuationVals)
		{
			if (range < attenuationVal.first)
			{
				return attenuationVal.second;
			}
		}
	}
private:
	static std::map<int, Attenuation> attenuationVals;
};

std::map<int, LightSim::Attenuation> LightSim::attenuationVals
{
	{7,    {0.7,    1.8}      },
	{13,   {0.35,   0.44}     },
	{20,   {0.22,   0.2}      },
	{32,   {0.14,   0.07}     },
	{50,   {0.09,   0.032}    },
	{65,   {0.07,   0.017}    },
	{100,  {0.045,  0.0075}   },
	{160,  {0.027,  0.0028}   },
	{200,  {0.022,  0.0019}   },
	{325,  {0.014,  0.0007}   },
	{600,  {0.007,  0.0002}   },
	{3250, {0.0014, 0.000007} }
};