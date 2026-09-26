#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <chrono>
#include <generator>

#include "glm/gtc/type_ptr.hpp"

#include "external/IObjectSim.h"
#include "StringCast.h"
#include "EverettStructs.h"

class SimSerializer
{
private:
	#define ValidateVersionCheck(version)                                  \
	auto versionValidation = ValidateVersion(version);                     \
	if (versionValidation > VersionValidationState::NewerValid)            \
	{                                                                      \
		return versionValidation != VersionValidationState::UnsetCritical; \
	} 

	template<OnlyFundamental Type>
	class ParserTracker
	{
		Type valueBuffer{};
		size_t counter{};

		void TryFormRepeatValue(std::string& res)
		{
			if (counter > 1)
			{
				res += '[' + StringCast::ToString(counter) + "] ";
			}

			counter = 0;
		}

	public:
		void ToString(Type value, std::string& res)
		{
			if (!counter || valueBuffer != value)
			{
				TryFormRepeatValue(res);

				res += StringCast::ToString<Type>(value) + ' ';
				valueBuffer = value;
			}

			++counter;
		}

		void FinalizeToString(std::string& res)
		{
			TryFormRepeatValue(res);

			if (res.size() > 1)
			{
				res.pop_back();
			}
		}

		std::generator<Type> FromString(std::string_view values)
		{
			std::string value;

			for (auto c : values)
			{
				if (counter)
				{
					for (size_t i = 0; i < counter; ++i)
					{
						co_yield valueBuffer;
					}

					counter = 0;
				}
				else
				{
					switch (c)
					{
					case '[':
						break;
					case ']':
						counter = StringCast::FromString<size_t>(value) - 1;
						value.clear();
						break;
					case ' ':
						valueBuffer = StringCast::FromString<Type>(value);
						co_yield valueBuffer;
						value.clear();
						break;
					default:
						value += c;
					}
				}
			}
		}
	};

	// OlderInvalid - required version should skip serialization of current line
    // Unset state should fail and stop serialization
	enum class VersionValidationState
	{
		ExactValid,
		NewerValid,
		OlderInvalid,
		UnsetCritical
	};

	constexpr static inline int latestSerializerVersion = 17;
	static inline int usedVersion = -1;
	static VersionValidationState ValidateVersion(int requiredVersion);
	static bool SetUsedVersion(int usedVersionToSet);

	template<OnlyFundamental Type>
	static void FromValue(Type value, std::string& res);

	static std::string PackValue(const std::string& value);
	static void UnpackValue(std::string_view& line, std::string& value, bool severalVals = true);
	static void UnpackValue(std::string_view& line);

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

	static int GetUsedVersion();

	static bool GetVersionFromLine(std::string_view& line);
	static std::string GetLatestVersionStr();

	static void GetObjectInfo(std::string_view& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo);

	static void SkipValuesInLines(std::string_view& line, size_t amountToSkip, int versionToSkip);
#ifdef _HAS_CXX20
	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(FundamentalType f);

	static void SkipDeprecatedValue(std::string_view& line, int requiredVersion, int deprecatedAt);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(std::string_view& line, FundamentalType& f, int requiredVersion);

	template<OnlyEnums EnumType>
	static std::string GetValueToSaveFrom(EnumType e);

	template<OnlyEnums EnumType>
	static bool SetValueToLoadFrom(std::string_view& line, EnumType& e, int requiredVersion);

	template<OnlyFundamental FundamentalType>
	static std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<OnlyFundamental FundamentalType>
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<FundamentalType>& vector, int requiredVersion
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
	static bool SetValueToLoadFrom(std::string_view& line, std::string& str, int requiredVersion);

	// GLM
	template<OnlyGLMs GLMType>
	static std::string GetValueToSaveFrom(const GLMType& cont);

	template<OnlyGLMs GLMType>
	static bool SetValueToLoadFrom(std::string_view& line, GLMType& cont, int requiredVersion);

	// Special cases
	static std::string GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	static std::string GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	static std::string GetValueToSaveFrom(const std::vector<std::string>& vectorStr);
	static std::string GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr);
	static std::string GetValueToSaveFrom(const std::chrono::system_clock::time_point timePoint);
	static std::string GetValueToSaveFrom(std::generator<EverettStructs::BasicFileInfo>&& vectOfFileInfo);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::unordered_map<IObjectSim::Direction, bool>& disabledDirs, 
		int requiredVersion
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits, 
		int requiredVersion
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::string>& vectorStr, int requiredVersion
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::pair<std::string, std::string>>& vectorPairStr, 
		int requiredVersion
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::chrono::system_clock::time_point& timePoint, int requiredVersion
	);
	static bool SetValueToLoadFrom(
		std::string_view& line, std::vector<EverettStructs::BasicFileInfo>& vectOfFileInfo, 
		int requiredVersion
	);
};

#ifdef _HAS_CXX20

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(StringCast::ToString<FundamentalType>(f));
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, FundamentalType& f, int requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string value;

	UnpackValue(line, value, false);

	f = StringCast::FromString<FundamentalType>(value);

	return true;
}

template<OnlyEnums EnumType>
std::string SimSerializer::GetValueToSaveFrom(EnumType e)
{
	return PackValue(StringCast::ToString(static_cast<int>(e)));
}

template<OnlyEnums EnumType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e, int requiredVersion)
{
	ValidateVersionCheck(requiredVersion)

	int preEnumValue;
	std::string value;

	UnpackValue(line, value, false);

	preEnumValue = StringCast::FromString<int>(value);
	e = static_cast<EnumType>(preEnumValue);

	return true;
}

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(const std::vector<FundamentalType>& vector)
{
	ParserTracker<FundamentalType> pt;
	std::string res;

	for (const auto iter : vector)
	{
		pt.ToString(iter, res);
	}
	pt.FinalizeToString(res);

	return PackValue(res);
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, std::vector<FundamentalType>& vector, int requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	ParserTracker<FundamentalType> pt;
	std::string values;

	UnpackValue(line, values);

	size_t i{};

	for (auto value : pt.FromString(values))
	{
		if (i >= vector.size())
		{
			vector.push_back(value);
		}
		else
		{
			vector[i] = value;
		}

		++i;
	}

	return AssertAndReturn(i == vector.size());
}

template<OnlyGLMs GLMType>
std::string SimSerializer::GetValueToSaveFrom(const GLMType& cont)
{
	using ValueType = typename GLMType::value_type;

	ParserTracker<ValueType> pt;
	std::string res;
	const ValueType* ptr = glm::value_ptr(cont);

	for (size_t i = 0; i < sizeof(GLMType) / sizeof(ValueType); ++i)
	{
		pt.ToString(ptr[i], res);
	}
	pt.FinalizeToString(res);

	return PackValue(res);
}

template<OnlyGLMs GLMType>
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, GLMType& cont, int requiredVersion)
{
	using ValueType = typename GLMType::value_type;

	ValidateVersionCheck(requiredVersion)

	ParserTracker<ValueType> pt;
	std::string values;

	UnpackValue(line, values);

	ValueType* ptr = glm::value_ptr(cont);
	size_t i = 0;

	for (auto value : pt.FromString(values))
	{
		ptr[i++] = value;
	}

	return AssertAndReturn(i == sizeof(GLMType) / sizeof(ValueType));
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