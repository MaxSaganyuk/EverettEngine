#include "SimSerializer.h"

using namespace std::chrono;

#define ValidateVersionCheck(version, deprecated)                      \
auto versionValidation = ValidateVersion(version, deprecated);         \
if (versionValidation > VersionValidationState::NewerValid)            \
{                                                                      \
	return versionValidation != VersionValidationState::UnsetCritical; \
} 

std::string SimSerializer::PackValue(const std::string& value)
{
	return '{' + value + '}';
}

void SimSerializer::UnpackValue(std::string_view& line, std::string& value, bool severalVals)
{
	value = line.substr(line.find('{') + 1, line.find('}') - 1);
	
	if (!value.empty() && severalVals)
	{
		value += ' ';
	}

	line.remove_prefix(line.find('}') + 1);
}

bool SimSerializer::AssertAndReturn(bool evaluation)
{
	CheckAndThrowExceptionWMessage(evaluation, "SimSerializer failed to parse values");
	return evaluation;
}

SimSerializer::VersionValidationState SimSerializer::ValidateVersion(int requiredVersion, int deprecatedAt)
{
	if (usedVersion != -1)
	{
		if (deprecatedAt && usedVersion >= deprecatedAt)
		{
			return VersionValidationState::DeprecatedInvalid;
		}

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

bool SimSerializer::SetUsedVersion(int usedVersionToSet)
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
	return SetUsedVersion(StringCast::FromString<int>(numberStr));
}

std::string SimSerializer::GetLatestVersionStr()
{
	return "Version*" + StringCast::ToString(latestSerializerVersion) + '\n';
}

void SimSerializer::GetObjectInfo(std::string_view& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo)
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

std::string SimSerializer::GetValueToSaveFrom(const std::string& str)
{
	return PackValue(str);
}

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::string& str, int requiredVersion, int deprecatedAt)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	UnpackValue(line, str, false);

	return true;
}

std::string SimSerializer::GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs)
{
	std::string res = "";

	for (auto& disabledDir : disabledDirs)
	{
		res += (StringCast::ToString(static_cast<int>(disabledDir.first)) + 
			' ' + 
			StringCast::ToString(static_cast<int>(disabledDir.second))) + 
			' ';
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::unordered_map<IObjectSim::Direction, bool>& disabledDirs,
	int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	IObjectSim::Direction currentDirection;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (!(i % 2))
			{
				currentDirection = static_cast<IObjectSim::Direction>(StringCast::FromString<int>(value));
			}
			else
			{
				disabledDirs.emplace(currentDirection, StringCast::FromString<int>(value));
			}

			++i;
			value.clear();
			continue;
		}

		value += c;
	}

	return AssertAndReturn(!(i % 2));
}

std::string SimSerializer::GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits)
{
	std::string res;

	for (size_t i = 0; i < rotationLimits.first.length(); ++i)
	{
		res += StringCast::ToString(rotationLimits.first[i]) + ' ';
	}
	for (size_t i = 0; i < rotationLimits.second.length(); ++i)
	{
		res += StringCast::ToString(rotationLimits.second[i]) + ' ';
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits,
	int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string values;

	UnpackValue(line, values);

	std::string value;
	IObjectSim::Rotation currentRotation;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			currentRotation[i % 3] = StringCast::FromString<float>(value);

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
			value.clear();
			continue;
		}

		value += c;
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
	std::string_view& line, std::vector<std::string>& vectorStr, int requiredVersion, int deprecatedAt
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
	int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

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
	int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

	std::string value;

	UnpackValue(line, value);

	long long timeCount = StringCast::FromString<long long>(value);
	timePoint = system_clock::time_point(milliseconds(timeCount));

	return true;
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<EverettStructs::BasicFileInfo>& vectOfFileInfo)
{
	std::string res;

	for (auto& fileInfo : vectOfFileInfo)
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
	int requiredVersion, int deprecatedAt
)
{
	ValidateVersionCheck(requiredVersion, deprecatedAt)

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