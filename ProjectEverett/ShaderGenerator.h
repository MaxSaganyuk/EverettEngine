#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <generator>

class ShaderGenerator
{
public:
	ShaderGenerator(const std::string& path);
	~ShaderGenerator();

	std::generator<std::string_view> GetValuesToDefine() const noexcept;

	template<typename Type>
	bool SetValueToDefine(const std::string& valueName, Type&& value);

	void GenerateShaderFiles(const std::string& path);
private:
	void LoadPreSources(const std::string& path);
	void InitializeFileObjects(const std::string& path, int flag);
	void DeinitializeFileObjects() noexcept;
	void InitializeLineMap();
	void ProcessPreSources();

	struct LineToSubstInfo
	{
		size_t customKeywordIndex{};
		std::string valueName;
		std::string substitute;
	};

	std::string preSourcePath;
	std::vector<std::fstream> preSourceFiles;
	std::vector<std::map<size_t, LineToSubstInfo>> lineToSubstMap;

	static std::vector<std::string> fileTypes;
	static std::vector<std::pair<std::string, std::string>> customKeywords;
};

template<typename Type>
bool ShaderGenerator::SetValueToDefine(const std::string& valueName, Type&& value)
{
	bool success = false;

	for (auto& collectionOfNames : lineToSubstMap)
	{
		for (auto& [_, info] : collectionOfNames)
		{
			if (info.valueName == valueName)
			{
				info.substitute = std::to_string(std::forward<Type>(value));

				success = true;
			}
		}
	}

	return success;
}