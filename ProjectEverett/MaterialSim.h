#pragma once

#include <string>
#include <map>

#include "glm/glm.hpp"

// Source: http://devernay.free.fr/cours/opengl/materials.html

class MaterialSim
{
public:
	enum class MaterialID
	{
		EMERALD,
		JADE,
		OBSIDIAN,
		PEARL,
		RUBY,
		TURQUOISE,
		BRASS,
		BRONZE,
		CHROME,
		COPPER,
		GOLD,
		SILVER
	};

	struct Material
	{
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};

	static Material GetMaterial(MaterialID materialId)
	{
		return materialMap[materialId];
	}

private:
	static std::map<MaterialID, Material> materialMap;
};


std::map<MaterialSim::MaterialID, MaterialSim::Material> MaterialSim::materialMap
{
	{
		MaterialID::EMERALD,
		Material{
			glm::vec3(0.0215, 0.1745, 0.0215),
			glm::vec3(0.07568, 0.61424, 0.07568),
			glm::vec3(0.633, 0.727811, 0.633),
			0.6f * 128
		}
	},
	{
		MaterialID::JADE,
		Material{
			glm::vec3(0.135, 0.2225, 0.0215),
			glm::vec3(0.54, 0.89, 0.63),
			glm::vec3(0.316228, 0.316228, 0.316228),
			0.1f * 128
		}
	},
	{
		MaterialID::OBSIDIAN,
		Material{
			glm::vec3(0.05375, 0.05, 0.06625),
			glm::vec3(0.18275, 0.17, 0.22525),
			glm::vec3(0.332741, 0.328634, 0.346435),
			0.3f * 128
		}
	},
	{
		MaterialID::PEARL,
		Material{
			glm::vec3(0.25, 0.20725, 0.20725),
			glm::vec3(1, 0.829, 0.829),
			glm::vec3(0.296648, 0.296648, 0.296648),
			0.088f * 128
		}
	},
	{
		MaterialID::RUBY,
		Material{
			glm::vec3(0.1745, 0.01175, 0.01175),
			glm::vec3(0.61424, 0.04136, 0.04136),
			glm::vec3(0.727811, 0.626959, 0.626959),
			0.6f * 128
		}
	},
	{
		MaterialID::TURQUOISE,
		Material{
			glm::vec3(0.1, 0.18725, 0.1745),
			glm::vec3(0.396, 0.74151, 0.69102),
			glm::vec3(0.297254, 0.30829, 0.306678),
			0.1f * 128
		}
	},
	{
		MaterialID::BRASS,
		Material{
			glm::vec3(0.329412, 0.223529, 0.027451),
			glm::vec3(0.780392, 0.568627, 0.113725),
			glm::vec3(0.992157, 0.941176, 0.807843),
			0.21794872f * 128
		}
	},
	{
		MaterialID::BRONZE,
		Material{
			glm::vec3(0.2125, 0.1275, 0.054),
			glm::vec3(0.714, 0.4284, 0.18144),
			glm::vec3(0.393548, 0.271906, 0.166721),
			0.2f * 128
		}
	},
	{
		MaterialID::CHROME,
		Material{
			glm::vec3(0.25, 0.25, 0.25),
			glm::vec3(0.4, 0.4, 0.4),
			glm::vec3(0.774597, 0.774597, 0.774597),
			0.6f * 128
		}
	},
	{
		MaterialID::COPPER,
		Material{
			glm::vec3(0.19125, 0.0735, 0.0225),
			glm::vec3(0.7038, 0.27048, 0.0828),
			glm::vec3(0.256777, 0.137622, 0.086014),
			0.1f * 128
		}
	},
	{
		MaterialID::GOLD,
		Material{
			glm::vec3(0.24725, 0.1995, 0.0745),
			glm::vec3(0.75164, 0.60648, 0.22648),
			glm::vec3(0.628281, 0.555802, 0.366065),
			0.4f * 128
		}
	},
	{
		MaterialID::SILVER,
		Material{
			glm::vec3(0.19225, 0.19225, 0.19225),
			glm::vec3(0.50754, 0.50754, 0.508273),
			glm::vec3(0.508273, 0.508273, 0.508273),
			0.4f * 128
		}
	}
};