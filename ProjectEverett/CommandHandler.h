#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "stdEx/mapEx.h"

class CommandHandler
{
	stdEx::map<std::string, std::function<void(const std::string&)>> commands;
	void GetCommandList(const std::string&);
public:
	CommandHandler();
	void RunCommandLine();
	void AddCommandLambda(const std::string& commandName, std::function<void(const std::string&)> command);
};