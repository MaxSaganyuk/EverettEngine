#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <EverettEngine.h>
#include <EverettException.h>

#include <fstream>

#include <stdEx/mapEx.h>

struct Config
{
	int windowWidth = 800;
	int windowHeight = 600;
	std::string windowTitle = "";
	std::string startSave; // must have start save
	bool fullscreenForce = false;
	bool debugText = false;
	bool defaultWASD = false;
};

bool ReadConfigFile(Config& config)
{
	bool success = false;

	std::string line;
	std::fstream file("config.ini", std::ios::in);

	if (!file.is_open())
	{
		return false;
	}

	stdEx::map<std::string, int> expectedKeys;
	int indexer = 0;

	expectedKeys.emplace("WindowWidth",     indexer++);
	expectedKeys.emplace("WindowHeight",    indexer++);
	expectedKeys.emplace("WindowTitle",     indexer++);
	expectedKeys.emplace("StartSave",       indexer++);
	expectedKeys.emplace("FullscreenForce", indexer++);
	expectedKeys.emplace("DebugText",       indexer++);
	expectedKeys.emplace("DefaultWASD",     indexer++);

	expectedKeys.SetDefaultValue(-1);

	while (!file.eof())
	{
		std::getline(file, line);
		std::string key = line.substr(0, line.find('='));
		std::string value = line.substr(line.find('=') + 1);

		if (!(key.empty() && value.empty()))
		{
			switch (expectedKeys[key])
			{
			case 0:
				config.windowWidth = std::stoi(value);
				break;
			case 1:
				config.windowHeight = std::stoi(value);
				break;
			case 2:
				config.windowTitle = value;
				break;
			case 3:
				config.startSave = value;
				success = true;
				break;
			case 4:
				config.fullscreenForce = std::stoi(value);
				break;
			case 5:
				config.debugText = std::stoi(value);
				break;
			case 6:
				config.defaultWASD = std::stoi(value);
				break;
			}
		}
	}

	return success;
}

int main(int argc, char* argv[])
{
	Config config;

	if (ReadConfigFile(config))
	{
		try
		{
			EverettEngine engine;

			engine.CreateAndSetupMainWindow(
				config.windowWidth,
				config.windowHeight,
				config.windowTitle,
				config.fullscreenForce,
				config.debugText
			);

			if (config.defaultWASD)
			{
				engine.SetDefaultWASDControls();
			}

			engine.LoadDataFromFile(config.startSave);
			engine.RunRenderWindow();
		}
		catch (const EverettException&)
		{
			std::terminate();
		}
	}
}