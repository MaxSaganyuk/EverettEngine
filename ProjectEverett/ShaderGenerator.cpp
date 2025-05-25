#include "ShaderGenerator.h"

#include <fstream>

std::vector<std::string> ShaderGenerator::fileTypes { "evert", "efrag" };
std::vector<std::pair<std::string, std::string>> ShaderGenerator::customKeywords{ {"#genDefine", "#define"} };

ShaderGenerator::~ShaderGenerator()
{
	DeinitializeFileObjects();
}

void ShaderGenerator::InitializeFileObjects(const std::string& path, const int flag)
{
	for (auto& fileType : fileTypes)
	{
		preSourceFiles.push_back(std::fstream(path + '.' + fileType, flag));
	}
}

void ShaderGenerator::DeinitializeFileObjects()
{
	preSourceFiles.clear();
}

void ShaderGenerator::LoadPreSources(const std::string& path)
{
	preSourcePath = path;
	InitializeFileObjects(preSourcePath, std::ios::in);
	InitializeLineMap();
	ProcessPreSources();
}

void ShaderGenerator::ProcessPreSources()
{
	std::string buffer;

	for (size_t fileIndex = 0; fileIndex < preSourceFiles.size(); ++fileIndex)
	{
		for (size_t lineIndex = 0; !preSourceFiles[fileIndex].eof(); ++lineIndex)
		{
			std::getline(preSourceFiles[fileIndex], buffer);

			if (buffer.size() > 0 && buffer[0] != '/') 
			{
				for (size_t customKeyWordIndex = 0; customKeyWordIndex < customKeywords.size(); ++customKeyWordIndex)
				{
					size_t pos = buffer.find(customKeywords[customKeyWordIndex].first);

					if (pos != std::string::npos)
					{
						std::string valueName = buffer.substr(buffer.find(' ') + 1, std::string::npos);
						std::string defaultSubstitute = "";

						if (valueName.find(' ') != std::string::npos)
						{
							defaultSubstitute = valueName.substr(valueName.find(' ') + 1, std::string::npos);
							valueName = valueName.substr(0, valueName.find(' '));
						}

						lineToSubstMap[fileIndex].emplace(
							lineIndex, LineToSubstInfo{ customKeyWordIndex, valueName, defaultSubstitute }
						);
					}
				}
			}
		}

		preSourceFiles[fileIndex].clear();
		preSourceFiles[fileIndex].seekg(0);
	}
}

void ShaderGenerator::InitializeLineMap()
{
	for (auto& fileType : fileTypes)
	{
		lineToSubstMap.push_back({});
	}
}

std::vector<std::string> ShaderGenerator::GetValuesToDefine()
{
	std::vector<std::string> res;

	for (auto& firstMap : lineToSubstMap)
	{
		for (auto& secondMap : firstMap)
		{
			res.push_back(secondMap.second.valueName);
		}
	}

	return res;
}

void ShaderGenerator::GenerateShaderFiles(const std::string& path)
{
	std::string buffer;

	for (size_t fileIndex = 0; fileIndex < preSourceFiles.size(); ++fileIndex)
	{
		std::string newShaderFileType = fileTypes[fileIndex];
		std::string newShaderFileName = path + '.' + newShaderFileType.erase(0, 1);
		std::fstream newShaderFile(newShaderFileName, std::ios::out);

		for (size_t lineIndex = 0; !preSourceFiles[fileIndex].eof(); ++lineIndex)
		{
			std::getline(preSourceFiles[fileIndex], buffer);

			if (lineToSubstMap[fileIndex].find(lineIndex) != lineToSubstMap[fileIndex].end())
			{
				// In case other special keywords will exist, this will be reworked
				LineToSubstInfo& substInfo = lineToSubstMap[fileIndex][lineIndex];
				buffer.clear();
				
				buffer +=
					customKeywords[substInfo.customKeywordIndex].second +
					' ' +
					substInfo.valueName + ' ' +
					substInfo.substitute;
			}

			newShaderFile << buffer << '\n';
		}
	}
}