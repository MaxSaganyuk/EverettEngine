#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <chrono>
#include <concepts>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "interfaces/IObjectSim.h"

template<typename FundamentalType>
concept OnlyFundamental = std::is_fundamental_v<FundamentalType>;

template<typename EnumType>
concept OnlyEnums = std::is_enum_v<EnumType>;

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

	constexpr static char serializerVersion[] = "1.1";

	static std::string PackValue(const std::string& value);
	static void UnpackValue(std::string_view& line, std::string& value, bool severalVals = true);

	static bool AssertAndReturn(bool evaluation);
public:
	enum ObjectInfoNames
	{
		ObjectType,
		SubtypeName,
		ObjectName,
		Path,
		_SIZE
	};

	static std::string GetSerializerVersion();
	static bool CheckSerializerVersion(const std::string_view& versionStr);

	static void GetObjectInfo(std::string_view& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo);
#ifdef _HAS_CXX20
	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(FundamentalType f);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(std::string_view& line, FundamentalType& f);

	template<OnlyEnums EnumType>
	static std::string GetValueToSaveFrom(EnumType e);

	template<OnlyEnums EnumType>
	static bool SetValueToLoadFrom(std::string_view& line, EnumType& e);

	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(std::string_view& line, std::vector<FundamentalType>& vector);
#else
	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static std::string GetValueToSaveFrom(FundamentalType f);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static bool SetValueToLoadFrom(std::string_view& line, FundamentalType& f);

	template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool> = false>
	static std::string GetValueToSaveFrom(EnumType e);

	template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool> = false>
	static bool SetValueToLoadFrom(std::string_view& line, EnumType& e);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool> = false>
	static bool SetValueToLoadFrom(std::string_view& line, std::vector<FundamentalType>& vector);
#endif
	// String
	static std::string GetValueToSaveFrom(const std::string& str);
	static bool SetValueToLoadFrom(std::string_view& line, std::string& str);

	// GLM
	static std::string GetValueToSaveFrom(const glm::vec3& vec);
	static std::string GetValueToSaveFrom(const glm::mat4& mat);
	static bool SetValueToLoadFrom(std::string_view& line, glm::vec3& vec);
	static bool SetValueToLoadFrom(std::string_view& line, glm::mat4& mat);

	// Special cases
	static std::string GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static std::string GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static std::string GetValueToSaveFrom(const std::vector<std::string>& vectorStr);
	static std::string GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr);
	static std::string GetValueToSaveFrom(const std::chrono::system_clock::time_point timePoint);
	static bool SetValueToLoadFrom(std::string_view& line, std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static bool SetValueToLoadFrom(std::string_view& line, std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static bool SetValueToLoadFrom(std::string_view& line, std::vector<std::string>& vectorStr);
	static bool SetValueToLoadFrom(std::string_view& line, std::vector<std::pair<std::string, std::string>>& vectorPairStr);
	static bool SetValueToLoadFrom(std::string_view& line, std::chrono::system_clock::time_point& timePoint);
};

#ifdef _HAS_CXX20
template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(std::to_string(f));
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, FundamentalType& f)
{
	std::string value;

	UnpackValue(line, value, false);

	f = FundamentalConvert<FundamentalType>::Convert(value);

	return true;
}

template<OnlyEnums EnumType>
std::string SimSerializer::GetValueToSaveFrom(EnumType e)
{
	return PackValue(std::to_string(static_cast<int>(e)));
}

template<OnlyEnums EnumType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e)
{
	int preEnumValue;
	std::string value;

	UnpackValue(line, value, false);

	preEnumValue = FundamentalConvert<int>::Convert(value);
	e = static_cast<EnumType>(preEnumValue);

	return true;
}

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(const std::vector<FundamentalType>& vector)
{
	std::string res = "";

	for (const auto iter : vector)
	{
		res += std::to_string(iter) + ' ';
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::vector<FundamentalType>& vector)
{
	std::string values;

	UnpackValue(line, values);

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

	return AssertAndReturn(i == vector.size());
}
#else

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(std::to_string(f));
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, FundamentalType& f)
{
	std::string value;

	UnpackValue(line, value, false);

	f = FundamentalConvert<FundamentalType>::Convert(value);

	return true;
}

template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(EnumType e)
{
	return PackValue(std::to_string(static_cast<int>(e)));
}

template<typename EnumType, typename std::enable_if_t<std::is_enum_v<EnumType>, bool>>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e)
{
	int preEnumValue;
	std::string value;

	UnpackValue(line, value, false);

	preEnumValue = FundamentalConvert<int>::Convert(value);
	e = static_cast<EnumType>(preEnumValue);

	return true;
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(const std::vector<FundamentalType>& vector)
{
	std::string res = "";

	for (const auto iter : vector)
	{
		res += std::to_string(iter) + ' ';
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::vector<FundamentalType>& vector)
{
	std::string values;

	UnpackValue(line, values);

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

	return AssertAndReturn(i == vector.size());
}
#endif