#pragma once

#include <array>
#include <stdexcept>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// MSVC does not want to build this utility as a class of static funcs and objects, namespace is used
namespace ColorManager
{
	struct RGBVal
	{
		std::uint8_t r{};
		std::uint8_t g{};
		std::uint8_t b{};

		constexpr RGBVal() = default;
		constexpr RGBVal(std::uint8_t r, std::uint8_t g, std::uint8_t b)
			: r(r), g(g), b(b) {}

		constexpr operator glm::vec3() const
		{
			return { r / 255.0f, g / 255.0f, b / 255.0f };
		}

		constexpr operator glm::vec4() const
		{
			return { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f };
		}

		constexpr bool operator==(const RGBVal&) const = default;
	};

	enum class Colors
	{
		RED, BLUE, GREEN, YELLOW, ORANGE, GREY, BLACK, WHITE, PINK, CYAN, PURPLE, BROWN, LIME, MAGENTA, NAVY, GOLD, _SIZE
	};

	namespace _ColorManager
	{
		constexpr inline size_t InitialColorAmount = std::to_underlying(Colors::_SIZE);
		constexpr inline size_t CustomColorAmount = 100;

		consteval auto InitializeColors()
		{
			std::array<glm::vec3, InitialColorAmount> colors{};

			colors[std::to_underlying(Colors::RED)]     = RGBVal{ 255, 0, 0   };
			colors[std::to_underlying(Colors::BLUE)]    = RGBVal{ 0,   0, 255 };
			colors[std::to_underlying(Colors::GREEN)]   = RGBVal{ 0, 255, 0   };
			colors[std::to_underlying(Colors::YELLOW)]  = RGBVal{ 255, 255, 0 };
			colors[std::to_underlying(Colors::ORANGE)]  = RGBVal{ 255, 165, 0 };
			colors[std::to_underlying(Colors::GREY)]    = RGBVal{ 128, 128, 128 };
			colors[std::to_underlying(Colors::BLACK)]   = RGBVal{ 0,   0,   0 };
			colors[std::to_underlying(Colors::WHITE)]   = RGBVal{ 255, 255, 255 };
			colors[std::to_underlying(Colors::PINK)]    = RGBVal{ 255, 192, 203 };
			colors[std::to_underlying(Colors::CYAN)]    = RGBVal{ 0, 255, 255 };
			colors[std::to_underlying(Colors::PURPLE)]  = RGBVal{ 128,   0, 128 };
			colors[std::to_underlying(Colors::BROWN)]   = RGBVal{ 139,  69,  19 };
			colors[std::to_underlying(Colors::LIME)]    = RGBVal{ 50, 205,  50 };
			colors[std::to_underlying(Colors::MAGENTA)] = RGBVal{ 255,   0, 255 };
			colors[std::to_underlying(Colors::NAVY)]    = RGBVal{ 0,   0, 128 };
			colors[std::to_underlying(Colors::GOLD)]    = RGBVal{ 255, 215,   0 };

			return colors;
		}

		constexpr inline auto initialColors = InitializeColors();
		constinit inline std::array<glm::vec3, CustomColorAmount> customColors{};
	}

	constexpr const glm::vec3& GetColorVec3(Colors color)
	{
		return _ColorManager::initialColors[std::to_underlying(color)];
	}

	constexpr glm::vec4 GetColorVec4(Colors color)
	{
		return glm::vec4{ GetColorVec3(color), 1.0f };
	}

	inline const glm::vec3& GetColorVec3(size_t index)
	{
		return _ColorManager::customColors[index];
	}

	inline glm::vec4 GetColorVec4(size_t index)
	{
		return glm::vec4{ GetColorVec3(index), 1.0f };
	}

	inline void SetCustomColor(size_t index, const RGBVal& RGBVal)
	{
		if (index >= _ColorManager::CustomColorAmount)
		{
			throw std::out_of_range("Out of range of max CustomColorAmount");
		}

		_ColorManager::customColors[index] = RGBVal;
	}

	inline void SetCustomColor(size_t index, const glm::vec3& vec)
	{
		if (index >= _ColorManager::CustomColorAmount)
		{
			throw std::out_of_range("Out of range of max CustomColorAmount");
		}

		_ColorManager::customColors[index] = vec;
	}
};