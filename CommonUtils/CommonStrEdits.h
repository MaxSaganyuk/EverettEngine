#pragma once

#include <string>

class CommonStrEdits
{
public:
	static std::string RemoveDigitsFromStringEnd(const std::string& str)
	{
		int placeholder;
		
		return RemoveDigitsFromStringEnd(str, placeholder);
	}

	static std::string RemoveDigitsFromStringEnd(const std::string& str, int& number)
	{
		std::string resStr = str;
		std::string digitStr = "";

		for (auto iter = str.rbegin(); iter != str.rend(); ++iter)
		{
			if (!std::isdigit(*iter))
			{
				number = std::stoi(digitStr.empty() ? "0" : digitStr);
				break;
			}

			digitStr = *iter + digitStr;
			resStr.pop_back();
		}

		return resStr;
	}
};