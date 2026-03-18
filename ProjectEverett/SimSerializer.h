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
#include "ConceptUtils.h"
#include "EverettException.h"

class SimSerializer
{
private:
	constexpr static size_t ConverterBufferSize = 1024;
	static inline char ConverterBuffer[ConverterBufferSize];

	template<typename>
	static bool FromString(const std::string_view str);
	template<typename>
	static std::string ToString(const bool value);

	template<OnlyFundamentalNotBool Type>
	static Type FromString(const std::string_view str);

	template<OnlyFundamentalNotBool Type>
	static std::string ToString(const Type value);

	#define ValidateVersionCheck(version, deprecated)                      \
	auto versionValidation = ValidateVersion(version, deprecated);         \
	if (versionValidation > VersionValidationState::NewerValid)            \
	{                                                                      \
		return versionValidation != VersionValidationState::UnsetCritical; \
	} 

	// OlderInvalid - required version should skip serialization of current line
    // Unset state should fail and stop serialization
	enum class VersionValidationState
	{
		ExactValid,
		NewerValid,
		OlderInvalid,
		DeprecatedInvalid,
		UnsetCritical
	};

	constexpr static inline int latestSerializerVersion = 8;
	static inline int usedVersion = -1;
	static VersionValidationState ValidateVersion(int requiredVersion, int deprecatedAt);
	static bool SetUsedVersion(int usedVersionToSet);

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

	static bool GetVersionFromLine(std::string_view& line);
	static std::string GetLatestVersionStr();

	static void GetObjectInfo(std::string_view& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo);
#ifdef _HAS_CXX20
	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(FundamentalType f);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(std::string_view& line, FundamentalType& f, int requiredVersion, int deprecatedAt = 0);

	template<OnlyEnums EnumType>
	static std::string GetValueToSaveFrom(EnumType e);

	template<OnlyEnums EnumType>
	static bool SetValueToLoadFrom(std::string_view& line, EnumType& e, int requiredVersion, int deprecatedAt = 0);

	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<FundamentalType>& vector, int requiredVersion, int deprecatedAt = 0
	);
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
	static bool SetValueToLoadFrom(std::string_view& line, std::string& str, int requiredVersion, int deprecatedAt = 0);

	// GLM
	template<OnlyGLMs GLMType>
	static std::string GetValueToSaveFrom(const GLMType& cont);

	template<OnlyGLMs GLMType>
	static bool SetValueToLoadFrom(std::string_view& line, GLMType& cont, int requiredVersion, int depricatedAt = 0);

	// Special cases
	static std::string GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static std::string GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static std::string GetValueToSaveFrom(const std::vector<std::string>& vectorStr);
	static std::string GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr);
	static std::string GetValueToSaveFrom(const std::chrono::system_clock::time_point timePoint);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::unordered_map<IObjectSim::Direction, bool>& disabledDirs, 
		int requiredVersion, int deprecatedAt = 0
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits, 
		int requiredVersion, int deprecatedAt = 0
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::string>& vectorStr, int requiredVersion, int deprecatedAt = 0
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::pair<std::string, std::string>>& vectorPairStr, 
		int requiredVersion, int deprecatedAt = 0
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::chrono::system_clock::time_point& timePoint, int requiredVersion, int deprecatedAt = 0
	);
};

#ifdef _HAS_CXX20

template<OnlyFundamentalNotBool Type>
Type SimSerializer::FromString(const std::string_view str)
{
	Type value{};

	auto [_, errorCode] = std::from_chars(str.data(), str.data() + str.size(), value);

	CheckAndThrowExceptionWMessage(
		static_cast<bool>(errorCode == std::errc()),
		"Failed to get from string during deserialization, error: " + std::to_string(static_cast<int>(errorCode))
	);

	return value;
}

template<OnlyFundamentalNotBool Type>
std::string SimSerializer::ToString(const Type value)
{
	auto [lineEndPtr, errorCode] = std::to_chars(ConverterBuffer, ConverterBuffer + ConverterBufferSize, value);

	CheckAndThrowExceptionWMessage(
		static_cast<bool>(errorCode == std::errc()),
		"Failed to get from value during serialization, error: " + std::to_string(static_cast<int>(errorCode))
	);

	return std::string(ConverterBuffer, lineEndPtr);
}

template<typename>
bool SimSerializer::FromString(const std::string_view str)
{
	return FromString<int>(str);
}

template<typename>
std::string SimSerializer::ToString(const bool value)
{
	return ToString(static_cast<int>(value));
}

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(ToString<FundamentalType>(f));
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, FundamentalType& f, int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string value;

	UnpackValue(line, value, false);

	f = FromString<FundamentalType>(value);

	return true;
}

template<OnlyEnums EnumType>
std::string SimSerializer::GetValueToSaveFrom(EnumType e)
{
	return PackValue(ToString(static_cast<int>(e)));
}

template<OnlyEnums EnumType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e, int requiredVersion, int deprecatedAt)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	int preEnumValue;
	std::string value;

	UnpackValue(line, value, false);

	preEnumValue = FromString<int>(value);
	e = static_cast<EnumType>(preEnumValue);

	return true;
}

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(const std::vector<FundamentalType>& vector)
{
	std::string res;

	for (const auto iter : vector)
	{
		res += ToString<FundamentalType>(iter) + ' ';
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, std::vector<FundamentalType>& vector, int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (i >= vector.size())
			{
				vector.push_back(FromString<FundamentalType>(value));
			}
			else
			{
				vector[i] = FromString<FundamentalType>(value);
			}
			++i;
			value.clear();
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == vector.size());
}

template<OnlyGLMs GLMType>
std::string SimSerializer::GetValueToSaveFrom(const GLMType& cont)
{
	std::string res = "";
	const typename GLMType::value_type* ptr = glm::value_ptr(cont);

	for (size_t i = 0; i < sizeof(GLMType) / sizeof(typename GLMType::value_type); ++i)
	{
		res += ToString(*(ptr + i)) + ' ';
	}
	res.pop_back();

	return PackValue(res);
}

template<OnlyGLMs GLMType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, GLMType& cont, int requiredVersion, int deprecatedAt)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	typename GLMType::value_type* ptr = glm::value_ptr(cont);
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			*(ptr + i++) = FromString<typename GLMType::value_type>(value);
			value.clear();
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == sizeof(GLMType) / sizeof(typename GLMType::value_type));
}

#else

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(std::to_string(f));
}

template<typename FundamentalType, typename std::enable_if_t<std::is_fundamental_v<FundamentalType>, bool>>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, FundamentalType& f, int requiredVersion)
{
	ValidateVersionCheck(requiredVersion)

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
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e, int requiredVersion)
{
	ValidateVersionCheck(requiredVersion)

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
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::vector<FundamentalType>& vector, int requiredVersion)
{
	ValidateVersionCheck(requiredVersion)

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

#undef VersionValidateCheck