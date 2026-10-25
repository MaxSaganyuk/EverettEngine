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
#include "DateUtilities.h"

template<stdEx::FixedString str>
consteval size_t operator""_ver()
{
	static_assert(str.GetBufferSize() == 12, "Invalid date format");

	return DateUtilities::GetDaysPassedFromDateString(str.View());
}

namespace _SimSerializer
{
#define ValidateVersionCheck(version)                                      \
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
				res += '[' + StringCast::ToString(counter).Unfix() + "] ";
			}

			counter = 0;
		}

	public:
		void ToString(Type value, std::string& res)
		{
			if (!counter || valueBuffer != value)
			{
				TryFormRepeatValue(res);

				res += StringCast::ToString<Type>(value).Unfix() + ' ';
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

	VersionValidationState ValidateVersion(size_t requiredVersion);

	template<OnlyFundamental Type>
	void FromValue(Type value, std::string& res);

	std::string PackValue(const std::string& value);
	void UnpackValue(std::string_view& line, std::string& value, bool severalVals = true);
	void UnpackValue(std::string_view& line);

	bool AssertAndReturn(bool evaluation);
}

namespace SimSerializer
{
	using namespace _SimSerializer;

	enum ObjectInfoNames
	{
		ObjectType,
		SubtypeName,
		ObjectName,
		Path,
		_SIZE
	};

	class ObjectInfoArray : public std::array<std::string, ObjectInfoNames::_SIZE>{};

	size_t GetUsedVersion();
	size_t GetLastVersion();
	bool SetUsedVersion(size_t usedVersionToSet);

	bool GetVersionFromLine(std::string_view& line);
	std::string GetLatestVersionStr();

	void GetObjectInfo(std::string_view& line, ObjectInfoArray& objectInfo);

	void SkipValuesInLines(std::string_view& line, size_t amountToSkip, size_t versionToSkip);

	template<OnlyFundamental FundamentalType>
	std::string GetValueToSaveFrom(FundamentalType f);

	void SkipDeprecatedValue(std::string_view& line, size_t requiredVersion, size_t deprecatedAt);

	template<OnlyFundamental FundamentalType>
	bool SetValueToLoadFrom(std::string_view& line, FundamentalType& f, size_t requiredVersion);

	template<OnlyEnums EnumType>
	std::string GetValueToSaveFrom(EnumType e);

	template<OnlyEnums EnumType>
	bool SetValueToLoadFrom(std::string_view& line, EnumType& e, size_t requiredVersion);

	template<OnlyFundamental FundamentalType>
	std::string GetValueToSaveFrom(const std::vector<FundamentalType>& vector);

	template<OnlyFundamental FundamentalType>
	bool SetValueToLoadFrom(
		std::string_view& line, std::vector<FundamentalType>& vector, size_t requiredVersion
	);

	// String
	std::string GetValueToSaveFrom(const std::string& str);
	bool SetValueToLoadFrom(std::string_view& line, std::string& str, size_t requiredVersion);

	// GLM
	template<OnlyGLMs GLMType>
	std::string GetValueToSaveFrom(const GLMType& cont);

	template<OnlyGLMs GLMType>
	bool SetValueToLoadFrom(std::string_view& line, GLMType& cont, size_t requiredVersion);

	// Special cases
	std::string GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs);
	std::string GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits);
	std::string GetValueToSaveFrom(const std::vector<std::string>& vectorStr);
	std::string GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr);
	std::string GetValueToSaveFrom(const std::chrono::system_clock::time_point timePoint);
	std::string GetValueToSaveFrom(std::generator<EverettStructs::BasicFileInfo>&& vectOfFileInfo);
	bool SetValueToLoadFrom(
		std::string_view& line, std::unordered_map<IObjectSim::Direction, bool>& disabledDirs, 
		size_t requiredVersion
	);
	bool SetValueToLoadFrom(
		std::string_view& line, std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits, 
		size_t requiredVersion
	);
	bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::string>& vectorStr, size_t requiredVersion
	);
	bool SetValueToLoadFrom(
		std::string_view& line, std::vector<std::pair<std::string, std::string>>& vectorPairStr, 
		size_t requiredVersion
	);
	bool SetValueToLoadFrom(
		std::string_view& line, std::chrono::system_clock::time_point& timePoint, size_t requiredVersion
	);
	bool SetValueToLoadFrom(
		std::string_view& line, std::vector<EverettStructs::BasicFileInfo>& vectOfFileInfo, 
		size_t requiredVersion
	);
};

template<OnlyFundamental FundamentalType>
std::string SimSerializer::GetValueToSaveFrom(FundamentalType f)
{
	return PackValue(StringCast::ToString<FundamentalType>(f));
}

template<OnlyFundamental FundamentalType>
bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, FundamentalType& f, size_t requiredVersion
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
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, EnumType& e, size_t requiredVersion)
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
	std::string_view& line, std::vector<FundamentalType>& vector, size_t requiredVersion
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
bool SimSerializer::SetValueToLoadFrom(std::string_view& line, GLMType& cont, size_t requiredVersion)
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

#undef VersionValidateCheck