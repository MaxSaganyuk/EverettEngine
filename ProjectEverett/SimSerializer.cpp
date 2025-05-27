#include "SimSerializer.h"

using namespace std::chrono;

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
	assert(evaluation && "SimSerializer failed to parse values");
	return evaluation;
}

SimSerializer::VersionValidationState SimSerializer::ValidateVersion(int requiredVersion)
{
	if (usedVersion != -1)
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
		assert(false && "Used version was not set");
		return VersionValidationState::UnsetCritical;
	}
}

bool SimSerializer::SetUsedVersion(int usedVersionToSet)
{
	bool isValidVersion = latestSerializerVersion <= usedVersionToSet;

	assert(isValidVersion && "Used version exceeds maximum supported one");
	if(isValidVersion)
	{
		usedVersion = usedVersionToSet;
	}

	return isValidVersion;
}

bool SimSerializer::GetVersionFromLine(std::string_view& line)
{
	std::string numberStr = std::string(line.substr(line.find("*") + 1));
	return SetUsedVersion(FundamentalConvert<int>::Convert(numberStr));
}

std::string SimSerializer::GetLatestVersionStr()
{
	return "Version*" + std::to_string(latestSerializerVersion) + '\n';
}

void SimSerializer::GetObjectInfo(std::string_view& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo)
{
	size_t objectInfoAmount = std::count(line.begin(), line.end(), '*');
	assert(objectInfoAmount <= ObjectInfoNames::_SIZE && "Invalid object info amount during world load");

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

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::string& str, int requiredVersion)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	UnpackValue(line, str, false);

	return true;
}

std::string SimSerializer::GetValueToSaveFrom(const glm::vec3& vec)
{
	std::string res = "";

	for (size_t i = 0; i < vec.length(); ++i)
	{
		res += std::to_string(vec[i]) + ' ';
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, glm::vec3& vec, int requiredVersion)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			vec[i++] = FundamentalConvert<float>::Convert(value);
			value = "";
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == vec.length());
}

std::string SimSerializer::GetValueToSaveFrom(const glm::mat4& mat)
{
	std::string res = "";

	for (size_t i = 0; i < mat.length(); ++i)
	{
		for (size_t j = 0; j < mat[i].length(); ++j)
		{
			res += std::to_string(mat[i][j]) + ' ';
		}
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, glm::mat4& mat, int requiredVersion)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			mat[i / mat.length()][i % mat[0].length()] = FundamentalConvert<float>::Convert(value);
			++i;
			value = "";
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == (mat.length() * mat[0].length()));
}

std::string SimSerializer::GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs)
{
	std::string res = "";

	for (auto& disabledDir : disabledDirs)
	{
		res += (std::to_string(static_cast<int>(disabledDir.first)) + ' ' + std::to_string(disabledDir.second)) + ' ';
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::unordered_map<IObjectSim::Direction, bool>& disabledDirs,
	int requiredVersion
)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
	IObjectSim::Direction currentDirection;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			if (!(i % 2))
			{
				currentDirection = static_cast<IObjectSim::Direction>(FundamentalConvert<int>::Convert(value));
			}
			else
			{
				disabledDirs.emplace(currentDirection, FundamentalConvert<int>::Convert(value));
			}

			++i;
			value = "";
			continue;
		}

		value += c;
	}

	return AssertAndReturn(!(i % 2));
}

std::string SimSerializer::GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits)
{
	std::string res = "";

	for (size_t i = 0; i < rotationLimits.first.length(); ++i)
	{
		res += std::to_string(rotationLimits.first[i]) + ' ';
	}
	for (size_t i = 0; i < rotationLimits.second.length(); ++i)
	{
		res += std::to_string(rotationLimits.second[i]) + ' ';
	}
	res.pop_back();

	return PackValue(res);
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits,
	int requiredVersion
)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
	IObjectSim::Rotation currentRotation;
	size_t i = 0;

	for (auto c : values)
	{
		if (c == ' ')
		{
			currentRotation[i % 3] = FundamentalConvert<float>::Convert(value);

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
			value = "";
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == 6);
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<std::string>& vectorStr)
{
	std::string res = "";

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

bool SimSerializer::SetValueToLoadFrom(std::string_view& line, std::vector<std::string>& vectorStr, int requiredVersion)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
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
			value = "";
			continue;
		}

		value += c;
	}

	return AssertAndReturn(i == vectorStr.size());
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<std::pair<std::string, std::string>>& vectorPairStr)
{
	std::string res = "";

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
	int requiredVersion
)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string values;

	UnpackValue(line, values);

	std::string value = "";
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

			value = "";
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
	return PackValue(std::to_string(timeCount));
}

bool SimSerializer::SetValueToLoadFrom(
	std::string_view& line, 
	std::chrono::system_clock::time_point& timePoint, 
	int requiredVersion
)
{
	auto versionValidation = ValidateVersion(requiredVersion);
	if (versionValidation > VersionValidationState::NewerValid)
	{
		return versionValidation == VersionValidationState::OlderInvalid;
	}

	std::string value;

	UnpackValue(line, value);

	long long timeCount = FundamentalConvert<long long>::Convert(value);
	timePoint = system_clock::time_point(milliseconds(timeCount));

	return true;
}