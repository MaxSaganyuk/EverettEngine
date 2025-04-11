#pragma once

#include <string>

class CommonStrEdits
{
public:
	static std::string RemoveDigitsFromStringEnd(const std::string& str)
	{
		std::string resStr = str;

		for (auto iter = str.rbegin(); iter != str.rend(); ++iter)
		{
			if (!std::isdigit(*iter))
			{
				break;
			}

			resStr.pop_back();
		}

		return resStr;
	}
};