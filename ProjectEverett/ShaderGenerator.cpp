#include "ShaderGenerator.h"

#include <fstream>

std::vector<std::string> ShaderGenerator::fileTypes { "evert", "efrag" };
std::vector<std::pair<std::string, std::string>> ShaderGenerator::customKeywords{ 
	{"#genDefine", "#define"}, {"#genSubst", ""}
};

ShaderGenerator::ShaderGenerator(const std::string& path)
{
	LoadPreSources(path);
}

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

void ShaderGenerator::DeinitializeFileObjects() noexcept
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
					if (buffer.find(customKeywords[customKeyWordIndex].first) != std::string::npos)
					{
						std::string_view bufferView{ buffer };

						std::string_view valueName = bufferView.substr(bufferView.find(' ') + 1, std::string::npos);
						std::string_view defaultSubstitute;

						if (valueName.find(' ') != std::string::npos)
						{
							defaultSubstitute = valueName.substr(valueName.find(' ') + 1, std::string::npos);
							valueName = valueName.substr(0, valueName.find(' '));
						}

						lineToSubstMap[fileIndex].try_emplace(
							lineIndex, static_cast<Keywords>(customKeyWordIndex), 
							std::string(valueName), std::string(defaultSubstitute)
						);
					}
				}
			}
		}

		preSourceFiles[fileIndex].clear();
		preSourceFiles[fileIndex].seekg(0);
	}
}

bool ShaderGenerator::SetValueImpl(const std::string& valueName, std::string&& value)
{
	bool success = false;

	for (auto& collectionOfNames : lineToSubstMap)
	{
		for (auto& [_, info] : collectionOfNames)
		{
			if (info.valueName == valueName)
			{
				info.substitute = value;

				success = true;
			}
		}
	}

	return success;
}

void ShaderGenerator::InitializeLineMap()
{
	lineToSubstMap.resize(fileTypes.size());
}

std::generator<std::string_view> ShaderGenerator::GetValuesToDefine() const noexcept
{
	for (auto& firstMap : lineToSubstMap)
	{
		for (auto& [_, info] : firstMap)
		{
			co_yield info.valueName;
		}
	}
}

bool ShaderGenerator::SetValueToSubst(const std::string& valueName, std::string code)
{
	return SetValueImpl(valueName, std::move(code));
}

void ShaderGenerator::GenerateShaderFiles(const std::string& path)
{
	std::string buffer;

	for (size_t fileIndex = 0; fileIndex < preSourceFiles.size(); ++fileIndex)
	{
		std::string_view newShaderFileType{ fileTypes[fileIndex] };
		std::string newShaderFileName = path + '.' + newShaderFileType.substr(1, std::string_view::npos).data();
		std::fstream newShaderFile(newShaderFileName, std::ios::out);

		for (size_t lineIndex = 0; !preSourceFiles[fileIndex].eof(); ++lineIndex)
		{
			std::getline(preSourceFiles[fileIndex], buffer);

			if (auto iter = lineToSubstMap[fileIndex].find(lineIndex); iter != lineToSubstMap[fileIndex].end())
			{
				buffer.clear();

				auto& substInfo = iter->second;

				switch (substInfo.customKeywordIndex)
				{
				case Keywords::Define:
					buffer = customKeywords[0].second + ' ' + substInfo.valueName + ' ' + substInfo.substitute;
					break;
				case Keywords::Subst:
					buffer = substInfo.substitute;
					break;
				default:
					std::unreachable();
				}
			}

			newShaderFile << buffer << '\n';
		}
	}
}