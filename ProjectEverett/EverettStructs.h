#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <unordered_set>
#include <string>

namespace EverettStructs
{
	struct LightParams
	{
		float constant;
		float linear;
		float quadratic;

		glm::vec3 direction;
		float cutOff;
		float outerCutOff;
	};

	struct AssetPaths
	{
		std::unordered_set<std::string> worldPaths;
		std::unordered_set<std::string> modelPaths;
		std::unordered_set<std::string> soundPaths;
		std::unordered_set<std::string> scriptPaths;

		const std::optional<std::reference_wrapper<const std::unordered_set<std::string>>> operator[](size_t index) const
		{
			switch (index)
			{
			case 0:
				return worldPaths;
			case 1:
				return modelPaths;
			case 2:
				return soundPaths;
			case 3:
				return scriptPaths;
			default:
				return std::nullopt;
			}
		}

		constexpr static size_t GetAssetTypeAmount()
		{
			return sizeof(AssetPaths) / sizeof(std::unordered_set<std::string>);
		}

		static const std::string GetAssetNameIndex(size_t index)
		{
			switch (index)
			{
			case 0:
				return "Worlds";
			case 1:
				return "Models";
			case 2:
				return "Sounds";
			case 3:
				return "Scripts";
			default:
				return "";
			}
		}
	};
}