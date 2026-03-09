#pragma once

#include <iostream>
#include <string>

namespace OutputUtils
{
	inline void LogOrError(bool success, const std::string& message)
	{
		(success ? std::cout : std::cerr) << message << ": " << (success ? "Success" : "Error") << '\n';
	}
}