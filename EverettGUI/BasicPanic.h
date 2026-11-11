#pragma once

#include <iostream>

#include "EverettEngine.h"

// C string is used instead of std::string to reduce amount of issues possible on panic state
inline void Panic(const char* errorMessage, EverettEngine& engine)
{
	std::cerr << errorMessage << '\n';
	engine.CreateLogReport();
	throw std::runtime_error{ errorMessage };
}