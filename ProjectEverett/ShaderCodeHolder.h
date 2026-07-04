#pragma once

#include <array>
#include <string>

class ShaderCodeHolder
{
public:
	enum class ShaderCodeSection
	{
		AmbientLight,
		DirectionLight,
		PointLight,
		SpotLight,
		_SIZE
	};

	void SetShaderCodeForSection(ShaderCodeSection codeSect, std::string code)
	{
		customCode[std::to_underlying(codeSect)] = std::move(code);
	}

	const char* GetShaderCodeForSection(ShaderCodeSection codeSect)
	{
		auto codeSectID = std::to_underlying(codeSect);

		return !customCode[codeSectID].empty() ? customCode[codeSectID].c_str() : defaultCode[codeSectID];
	}

	constexpr const char* GetStringForCodeSection(ShaderCodeSection codeSect)
	{
		return enumStrs[std::to_underlying(codeSect)];
	}

	void ClearCustomCode()
	{
		customCode.fill("");
	}

private:
	constexpr static auto CodeSectionAmount = std::to_underlying(ShaderCodeSection::_SIZE);

	static inline std::array<std::string, CodeSectionAmount> customCode{};
	
	constexpr static std::array<const char*, CodeSectionAmount> enumStrs{
		"AmbientLight",
		"DirectionLight",
		"PointLight",
		"SpotLight"
	};

	constexpr static std::array<const char*, CodeSectionAmount> defaultCode{
		#include "shaderDetails/AmbientLight"
		,
		#include "shaderDetails/DirectionLight"
		,
		#include "shaderDetails/PointLight"
		,
		#include "shaderDetails/SpotLight"
	};
};