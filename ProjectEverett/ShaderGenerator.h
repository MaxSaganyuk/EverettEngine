#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <generator>

#include "ConceptUtils.h"

class ShaderGenerator
{
public:
	ShaderGenerator(const std::string& path);
	~ShaderGenerator();

	std::generator<std::string_view> GetValuesToDefine() const noexcept;

	template<OnlyFundamental Type>
	bool SetValueToDefine(const std::string& valueName, Type&& value);
	bool SetValueToSubst(const std::string& valueName, std::string code);

	void GenerateShaderFiles(const std::string& path);
private:
	void LoadPreSources(const std::string& path);
	void InitializeFileObjects(const std::string& path, int flag);
	void DeinitializeFileObjects() noexcept;
	void InitializeLineMap();
	void ProcessPreSources();

	enum class Keywords
	{
		Define, Subst
	};

	struct LineToSubstInfo
	{
		Keywords customKeywordIndex;
		std::string valueName;
		std::string substitute;
	};

	bool SetValueImpl(const std::string& valueName, std::string&& value);

	std::string preSourcePath;
	std::vector<std::fstream> preSourceFiles;
	std::vector<std::map<size_t, LineToSubstInfo>> lineToSubstMap;

	static std::vector<std::string> fileTypes;
	static std::vector<std::pair<std::string, std::string>> customKeywords;
};

template<OnlyFundamental Type>
bool ShaderGenerator::SetValueToDefine(const std::string& valueName, Type&& value)
{
	return SetValueImpl(valueName, std::to_string(std::forward<Type>(value)));
}
