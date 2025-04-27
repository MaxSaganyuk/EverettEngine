#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>

class ShaderGenerator
{
public:
	~ShaderGenerator();

	void LoadPreSources(const std::string& path);
	std::vector<std::string> GetValuesToDefine();

	template<typename Type>
	void SetValueToDefine(const std::string& valueName, Type& value);

	void GenerateShaderFiles(const std::string& path);
private:
	void InitializeFileObjects(const std::string& path, int flag);
	void DeinitializeFileObjects();
	void InitializeLineMap();
	void ProcessPreSources();

	struct LineToSubstInfo
	{
		size_t customKeywordIndex;
		std::string valueName;
		std::string substitute;
	};

	std::string preSourcePath;
	std::vector<std::fstream> preSourceFiles;
	std::vector<std::map<int, LineToSubstInfo>> lineToSubstMap;

	static std::vector<std::string> fileTypes;
	static std::vector<std::pair<std::string, std::string>> customKeywords;
};

template<typename Type>
void ShaderGenerator::SetValueToDefine(const std::string& valueName, Type& value)
{
	for (auto& collectionOfNames : lineToSubstMap)
	{
		for (auto& secondMap : collectionOfNames)
		{
			if (secondMap.second.valueName == valueName)
			{
				secondMap.second.substitute = std::to_string(value);
			}
		}
	}
}