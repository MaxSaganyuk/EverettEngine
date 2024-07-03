#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "CameraSim.h"

#include "stdEx/mapEx.h"

using SolidSimManager = std::unordered_map<std::string, std::vector<SolidSim>>;

class CommandHandler
{
	CameraSim& camera;
	SolidSimManager& solids;
	stdEx::map<std::string, std::function<void(const std::string&)>> commands;

	void SpawnSolid(const std::string& arg);
	void GhostModeToggle(const std::string& arg);
	void SetupCommands();

public:
	CommandHandler(CameraSim& camera, SolidSimManager& solids);
	void RunCommandLine();
};