#include "SimSerializer.h"

void SimSerializer::GetObjectInfo(std::string& line, std::array<std::string, ObjectInfoNames::_SIZE>& objectInfo)
{
	size_t objectInfoAmount = std::count(line.begin(), line.end(), '*');
	assert(objectInfoAmount <= ObjectInfoNames::_SIZE && "Invalid object info amount during world load");

	for (size_t i = 0; i < objectInfoAmount; ++i)
	{
		objectInfo[i] = line.substr(0, line.find('*'));
		line.erase(0, line.find('*') + 1);
	}
}

std::string SimSerializer::GetValueToSaveFrom(const std::string& str)
{
	return '{' + str + '}';
}

void SimSerializer::SetValueToLoadFrom(std::string& line, std::string& str)
{
	str = line.substr(line.find('{') + 1, line.find('}') - 1);
	line.erase(line.find('{'), line.find('}') + 1);
}

std::string SimSerializer::GetValueToSaveFrom(const glm::vec3& vec)
{
	std::string res = "{";

	for (size_t i = 0; i < vec.length(); ++i)
	{
		res += std::to_string(vec[i]) + ' ';
	}
	res.pop_back();

	res += '}';

	return res;
}

void SimSerializer::SetValueToLoadFrom(std::string& line, glm::vec3& vec)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
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

	line.erase(line.find('{'), line.find('}') + 1);
}

std::string SimSerializer::GetValueToSaveFrom(const glm::mat4& mat)
{
	std::string res = "{";

	for (size_t i = 0; i < mat.length(); ++i)
	{
		for (size_t j = 0; j < mat[i].length(); ++j)
		{
			res += std::to_string(mat[i][j]) + ' ';
		}
	}
	res.pop_back();

	res += '}';

	return res;
}

void SimSerializer::SetValueToLoadFrom(std::string& line, glm::mat4& mat)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
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

	line.erase(line.find('{'), line.find('}') + 1);
}

std::string SimSerializer::GetValueToSaveFrom(const std::unordered_map<IObjectSim::Direction, bool>& disabledDirs)
{
	std::string res = "{";

	for (auto& disabledDir : disabledDirs)
	{
		res += (std::to_string(static_cast<int>(disabledDir.first)) + ' ' + std::to_string(disabledDir.second)) + ' ';
	}
	res.pop_back();

	res += '}';

	return res;
}

void SimSerializer::SetValueToLoadFrom(
	std::string& line, 
	std::unordered_map<IObjectSim::Direction, bool>& disabledDirs
)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
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

	line.erase(line.find('{'), line.find('}') + 1);
}

std::string SimSerializer::GetValueToSaveFrom(const std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits)
{
	std::string res = "{";

	for (size_t i = 0; i < rotationLimits.first.length(); ++i)
	{
		res += std::to_string(rotationLimits.first[i]) + ' ';
	}
	for (size_t i = 0; i < rotationLimits.second.length(); ++i)
	{
		res += std::to_string(rotationLimits.second[i]) + ' ';
	}
	res.pop_back();

	res += '}';

	return res;
}

void SimSerializer::SetValueToLoadFrom(
	std::string& line, 
	std::pair<IObjectSim::Rotation, IObjectSim::Rotation>& rotationLimits
)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
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

	line.erase(line.find('{'), line.find('}') + 1);
}

std::string SimSerializer::GetValueToSaveFrom(const std::vector<std::string>& vectorStr)
{
	std::string res = "{";

	for (auto& str : vectorStr)
	{
		res += (str + ' ');
	}
	if (res.size() > 1)
	{
		res.pop_back();
	}

	res += '}';

	return res;
}

void SimSerializer::SetValueToLoadFrom(std::string& line, std::vector<std::string>& vectorStr)
{
	std::string values = line.substr(line.find('{') + 1, line.find('}') - 1);
	std::string value = "";

	for (auto c : values)
	{
		if (c == ' ')
		{
			vectorStr.push_back(value);
			value = "";
			continue;
		}

		value += c;
	}
}