#pragma once

#include <string>

#include "EverettException.h"

// Engine interface is for external use, will only expose necessary calls to the engine from scripting
class IEverettEngine
{
public:
	virtual void RequestWorldLoad(const char* path) = 0;
};