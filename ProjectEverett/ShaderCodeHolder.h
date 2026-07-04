#pragma once

#include <array>
#include <string>

#include "external/IEverettEngine.h"

class ShaderCodeHolder
{
public:
	void SetShaderCodeForSection(IEverettEngine::ShaderCodeSection codeSect, std::string code)
	{
		customCode[std::to_underlying(codeSect)] = std::move(code);
	}

	const char* GetShaderCodeForSection(IEverettEngine::ShaderCodeSection codeSect, bool forceDefault = false)
	{
		auto codeSectID = std::to_underlying(codeSect);

		return !(forceDefault || customCode[codeSectID].empty()) ? customCode[codeSectID].c_str() : defaultCode[codeSectID];
	}

	constexpr const char* GetStringForCodeSection(IEverettEngine::ShaderCodeSection codeSect)
	{
		return enumStrs[std::to_underlying(codeSect)];
	}

	bool IsCustomCodeInSection(IEverettEngine::ShaderCodeSection codeSect)
	{
		return !customCode[std::to_underlying(codeSect)].empty();
	}

	void ClearCustomCode()
	{
		customCode.fill("");
	}

private:
	constexpr static auto CodeSectionAmount = std::to_underlying(IEverettEngine::ShaderCodeSection::_SIZE);

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