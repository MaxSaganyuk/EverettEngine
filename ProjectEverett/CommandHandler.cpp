#include "CommandHandler.h"

#include <iostream>
#include <Windows.h>

#define WrapInALambda(func) [this](const std::string& arg) { func(arg); };

void CommandHandler::SpawnSolid(const std::string& arg)
{
	glm::vec3 posToPlace = camera.GetPositionVectorAddr() + camera.GetFrontVectorAddr();
	try
	{
		std::vector<SolidSim>& exactSolids = solids.at(arg);
		exactSolids.push_back(posToPlace);
	}
	catch (std::out_of_range&)
	{
		std::cerr << "No solid named " + arg;
	}
}

void CommandHandler::GhostModeToggle(const std::string& arg)
{
	camera.SetGhostMode(arg == "1");
}

void CommandHandler::SetupCommands()
{
	commands["spawn"]     = WrapInALambda(SpawnSolid)
	commands["ghostMode"] = WrapInALambda(GhostModeToggle)

	commands.SetDefaultValue([](const std::string&) { std::cout << "This command does not exist\n"; });
}

CommandHandler::CommandHandler(CameraSim& camera, SolidSimManager& solids) :
	camera(camera), solids(solids)
{
	SetupCommands();
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