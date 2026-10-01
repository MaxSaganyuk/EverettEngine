#pragma once

#include <map>

#include "glm/glm.hpp"

// Source: http://devernay.free.fr/cours/opengl/materials.html

namespace MaterialSim
{
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

	Material GetMaterial(MaterialID materialId);
};