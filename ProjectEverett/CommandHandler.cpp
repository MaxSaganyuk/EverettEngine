#include "CommandHandler.h"

#include <iostream>
#include <Windows.h>

CommandHandler::CommandHandler()
{
	commands.SetDefaultValue([](const std::string&) { std::cout << "This command does not exist\n"; });
	commands["commandList"] = [this](const std::string&) { GetCommandList(""); };
}

void CommandHandler::GetCommandList(const std::string&)
{
	std::cout << '\n';
	for (const auto& command : commands)
	{
		std::cout << command.first << '\n';
	}
}

void CommandHandler::AddCommandLambda(const std::string& commandName, std::function<void(const std::string&)> command)
{
	if (commands.find(commandName) == commands.end())
	{
		commands[commandName] = command;
		std::cout << "Added command " + commandName << '\n';
	}
	else
	{
		std::cerr << "Command " + commandName + " already exists\n";
	}
}

void CommandHandler::RunCommandLine()
{
	HWND activeHWND = GetActiveWindow();
	HWND consoleHWND = GetConsoleWindow();

	BringWindowToTop(consoleHWND);

	std::cout << "Enter command\n";

	std::string input;
	std::getline(std::cin, input);

	std::string command = input.substr(0, input.find(' '));
	std::string arg = input.substr(input.find(' ') + 1, std::string::npos);

	commands.at(command)(arg);

	BringWindowToTop(activeHWND);
}