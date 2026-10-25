#include "SimSerializer.h"

namespace
{
	constexpr static std::array<size_t, 19> legacyVersionFlatMap{
		"May 26 2025"_ver, "May 27 2025"_ver, "Aug 15 2025"_ver, "Aug 16 2025"_ver, "Feb 03 2026"_ver, "Mar 14 2026"_ver,
		"Mar 15 2026"_ver, "Mar 16 2026"_ver, "Mar 25 2026"_ver, "Mar 29 2026"_ver, "Apr 06 2026"_ver, "Apr 11 2026"_ver, 
		"Jun 17 2026"_ver, "Jun 19 2026"_ver, "Jun 27 2026"_ver, "Sep 20 2026"_ver, "Sep 26 2026"_ver, "Oct 15 2026"_ver, 
		"Oct 17 2026"_ver
	};

	// __DATE__
	constexpr stdEx::FixedString latestSerializerVersionDate = "Oct 25 2026";
	constexpr size_t latestSerializerVersion = 
		DateUtilities::GetDaysPassedFromDateString(latestSerializerVersionDate.View());

	std::optional<size_t> usedVersion;
}

using namespace std::chrono;
using namespace _SimSerializer;
using namespace SimSerializer;

#define ValidateVersionCheck(version)                                  \
auto versionValidation = ValidateVersion(version);                     \
if (versionValidation > VersionValidationState::NewerValid)            \
{                                                                      \
	return versionValidation != VersionValidationState::UnsetCritical; \
} 

std::string _SimSerializer::PackValue(const std::string& value)
{
	return '{' + value + '}';
}

void _SimSerializer::UnpackValue(std::string_view& line, std::string& value, bool severalVals)
{
	size_t leftSidePos = line.find('{') + 1;
	value = line.substr(leftSidePos, line.find('}') - leftSidePos);
	
	if (!value.empty() && severalVals)
	{
		value += ' ';
	}

	UnpackValue(line);
}

void _SimSerializer::UnpackValue(std::string_view& line)
{
	line.remove_prefix(line.find('}') + 1);
}

bool _SimSerializer::AssertAndReturn(bool evaluation)
{
	CheckAndThrowExceptionWMessage(evaluation, "SimSerializer failed to parse values");
	return evaluation;
}

_SimSerializer::VersionValidationState _SimSerializer::ValidateVersion(size_t requiredVersion)
{
	if (usedVersion)
	{
		if (usedVersion == requiredVersion)
		{
			return VersionValidationState::ExactValid;
		}
		else if (usedVersion > requiredVersion)
		{
			return VersionValidationState::NewerValid;
		}
		else
		{
			return VersionValidationState::OlderInvalid;
		}
	}
	else
	{
		ThrowExceptionWMessage("Used version was not set");
		return VersionValidationState::UnsetCritical;
	}
}

size_t SimSerializer::GetUsedVersion()
{
	return *usedVersion;
}

size_t SimSerializer::GetLastVersion()
{
	return latestSerializerVersion;
}

bool SimSerializer::SetUsedVersion(size_t usedVersionToSet)
{
	bool isValidVersion = usedVersionToSet <= latestSerializerVersion;

	CheckAndThrowExceptionWMessage(isValidVersion, "Used version exceeds maximum supported one");
	if(isValidVersion)
	{
		usedVersion = usedVersionToSet;
	}

	return isValidVersion;
}

bool SimSerializer::GetVersionFromLine(std::string_view& line)
{
	std::string numberStr = std::string(line.substr(line.find("*") + 1));

	size_t version{};

	if (numberStr.size() == latestSerializerVersionDate.size)
	{
		version = DateUtilities::GetDaysPassedFromDateString(numberStr);
	}
	else
	{
		version = StringCast::FromString<size_t>(numberStr);

		CheckAndThrowExceptionWMessage((version > 0 && version < 20), "Invalid legacy version");

		version = legacyVersionFlatMap[version - 1];
	}

	return SetUsedVersion(version);
}

std::string SimSerializer::GetLatestVersionStr()
{
	return "Version*" + latestSerializerVersionDate.Unfix() + '\n';
}

void SimSerializer::GetObjectInfo(std::string_view& line, ObjectInfoArray& objectInfo)
{
	size_t objectInfoAmount = std::count(line.begin(), line.end(), '*');
	CheckAndThrowExceptionWMessage(
		static_cast<bool>(objectInfoAmount <= ObjectInfoNames::_SIZE), 
		"Invalid object info amount during world load"
	);

	for (size_t i = 0; i < objectInfoAmount; ++i)
	{
		objectInfo[i] = line.substr(0, line.find('*'));
		line.remove_prefix(line.find('*') + 1);
	}
}

void SimSerializer::SkipDeprecatedValue(std::string_view& line, size_t requiredVersion, size_t deprecatedAt)
{
	if (usedVersion >= requiredVersion && usedVersion < deprecatedAt)
	{
		UnpackValue(line);
	}
}

std::string SimSerializer::GetValueToSaveFrom(const std::string& str)
{
	return PackValue(str);
}

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::string& str, size_t requiredVersion)
{
	ValidateVersionCheck(requiredVersion)

	UnpackValue(line, str, false);

	return true;
}

std::string SimSerializer::GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs)
{
	ParserTracker<int> pt;
	std::string res;

	for (auto& disabledDir : disabledDirs)
	{
		pt.ToString(static_cast<int>(disabledDir.first), res);
		pt.ToString(static_cast<int>(disabledDir.second), res);
	}
	pt.FinalizeToString(res);

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::unordered_map<IObjectSim::Direction, bool>& disabledDirs,
	size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string values;

	UnpackValue(line, values);

	ParserTracker<int> pt;
	std::string value;
	IObjectSim::Direction currentDirection;
	size_t i = 0;

	for (auto value : pt.FromString(values))
	{
		if (!(i % 2))
		{
			currentDirection = static_cast<IObjectSim::Direction>(value);
		}
		else
		{
			disabledDirs.emplace(currentDirection, value);
		}

		++i;
	}

	return AssertAndReturn(!(i % 2));
}

std::string SimSerializer::GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits)
{
	ParserTracker<IObjectSim::Rotation::value_type> pt;
	std::string res;

	for (size_t i = 0; i < rotationLimits.first.length(); ++i)
	{
		pt.ToString(rotationLimits.first[i], res);
	}
	for (size_t i = 0; i < rotationLimits.second.length(); ++i)
	{
		pt.ToString(rotationLimits.second[i], res);
	}
	pt.FinalizeToString(res);

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits,
	size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string values;

	UnpackValue(line, values);

	ParserTracker<IObjectSim::Rotation::value_type> pt;
	std::string value;
	IObjectSim::Rotation currentRotation;
	size_t i = 0;

	for (auto value : pt.FromString(values))
	{
		currentRotation[i % 3] = value;

		if (!((i + 1) % 3))
		{
			if (i < 4)
			{
				rotationLimits.first = currentRotation;
			}
			else
			{
				rotationLimits.second = currentRotation;
			}
		}

		++i;
	}

	return AssertAndReturn(i == 6);
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<std::string>& vectorStr)
{
	std::string res;

	for (auto& str : vectorStr)
	{
		res += (str + ' ');
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, std::vector<std::string>& vectorStr, size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (i >= vectorStr.size())
			{
				vectorStr.push_back(value);
			}
			else
			{
				vectorStr[i] = value;
			}
			++i;
			value.clear();
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == vectorStr.size());
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr)
{
	std::string res;

	for (auto& pair : vectorPairStr)
	{
		res += (pair.first + ' ' + pair.second + ' ');
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::vector<std::pair<std::string, std::string>>& vectorPairStr, 
	size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	std::string firstValue;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (i % 2 == 0)
			{
				firstValue = value;
			}
			else
			{
				vectorPairStr.push_back({ firstValue, value });
			}

			value.clear();
			++i;
			continue;
		}

		value += c;
	}

	return AssertAndReturn(!(i % 2));
}

std::string SimSerializer::GetValueToSaveFrom(const std::chrono::system_clock::time_point timePoint)
{
	long long timeCount = duration_cast<milliseconds>(timePoint.time_since_epoch()).count();
	return PackValue(StringCast::ToString(timeCount));
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::chrono::system_clock::time_point& timePoint, 
	size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string value;

	UnpackValue(line, value);

	long long timeCount = StringCast::FromString<long long>(value);
	timePoint = system_clock::time_point(milliseconds(timeCount));

	return true;
}

std::string SimSerializer::GetValueToSaveFrom(std::generator<EverettStructs::BasicFileInfo>&& vectOfFileInfo)
{
	std::string res;

	for (auto&& fileInfo : vectOfFileInfo)
	{
		res += (fileInfo.path + ' ' + fileInfo.name + ' ' + fileInfo.hash + ' ');
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, std::vector<EverettStructs::BasicFileInfo>& vectOfFileInfo,
	size_t requiredVersion
)
{
	ValidateVersionCheck(requiredVersion)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	std::string firstValue;
	std::string secondValue;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (i % 3 == 0)
			{
				firstValue = value;
			}
			else if (i % 3 == 1)
			{
				secondValue = value;
			}
			else
			{
				vectOfFileInfo.push_back({ firstValue, secondValue, value });
			}

			value.clear();
			++i;
			continue;
		}

		value += c;
	}

	return AssertAndReturn(!(i % 3));
}