#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "interfaces/IObjectSim.h"

class SimSerializer
{
private:
	template<typename Type>
	struct FundamentalConvert;

	#define FundamentalConvertSet(Type, Func)         \
	template<>                                        \
	struct FundamentalConvert<Type>                   \
	{                                                 \
		static Type Convert(const std::string& value) \
		{                                             \
			return std::##Func(value);                \
		}                                             \
	};                                                    

	FundamentalConvertSet(bool, stoi)
	FundamentalConvertSet(int, stoi)
	FundamentalConvertSet(long, stol)
	FundamentalConvertSet(long long, stoll)
	FundamentalConvertSet(unsigned long, stoul)
	FundamentalConvertSet(unsigned long long, stoull)
	FundamentalConvertSet(float, stof)
	FundamentalConvertSet(double, stod)
	FundamentalConvertSet(long double, stold)

public:
	enum ObjectInfoNames
	{
		ObjectType,
		SubtypeName,
		ObjectName,
		Path,
		_SIZE
	};

	static void GetObjectInfo(std::string& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static std::string GetValueToSaveFrom(FundamentalType f);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static void SetValueToLoadFrom(std::string& line, FundamentalType& f);

	template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool> = false>
	static std::string GetValueToSaveFrom(EnumType e);

	template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool> = false>
	static void SetValueToLoadFrom(std::string& line, EnumType& e);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static void SetValueToLoadFrom(std::string& line, std::vector<FundamentalType>& vector);

	// String
	static std::string GetValueToSaveFrom(const std::string& str);
	static void SetValueToLoadFrom(std::string& line, std::string& str);

	// GLM
	static std::string GetValueToSaveFrom(const glm::vec3& vec);
	static void SetValueToLoadFrom(std::string& line, glm::vec3& vec);
	static std::string GetValueToSaveFrom(const glm::mat4& mat);
	static void SetValueToLoadFrom(std::string& line, glm::mat4& mat);

	// Special cases
	static std::string GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static std::string GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static std::string GetValueToSaveFrom(const std::vector<std::string>& vectorStr);
	static void SetValueToLoadFrom(std::string& line, std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static void SetValueToLoadFrom(std::string& line, std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static void SetValueToLoadFrom(std::string& line, std::vector<std::string>& vectorStr);
};

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return '{' + std::to_string(f) + '}';
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
void SimSerializer::SetValueToLoadFrom(std::string& line, FundamentalType& f)
{
	std::string value = line.substr(line.find('{') + 1, line.find('}') - 1);

	f = FundamentalConvert<FundamentalType>::Convert(value);

	line.erase(line.find('{'), line.find('}') + 1);
}

template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(EnumType e)
{
	return '{' + std::to_string(static_cast<int>(e)) + '}';
}

template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool>>
void SimSerializer::SetValueToLoadFrom(std::string& line, EnumType& e)
{
	int preEnumValue;
	std::string value = line.substr(line.find('{') + 1, line.find('}') - 1);

	preEnumValue = FundamentalConvert<int>::Convert(value);
	e = static_cast<EnumType>(preEnumValue);

	line.erase(line.find('{'), line.find('}') + 1);
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(const std::vector<FundamentalType>& vector)
{
	std::string res = "{";

	for (const auto iter : vector)
	{
		res += std::to_string(iter) + ' ';
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	res += '}';

	return res;
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
void SimSerializer::SetValueToLoadFrom(std::string& line, std::vector<FundamentalType>& vector)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
	std::string value = "";
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (i >= vector.size())
			{
				vector.push_back(FundamentalConvert<FundamentalType>::Convert(value));
			}
			else
			{
				vector[i] = FundamentalConvert<FundamentalType>::Convert(value);
			}
			++i;
			value = "";
			continue;
		}

		value += c;
	}

	line.erase(line.find('{'), line.find('}') + 1);
}