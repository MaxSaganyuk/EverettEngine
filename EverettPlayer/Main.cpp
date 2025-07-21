#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <EverettEngine.h>
#include <EverettException.h>

#include <fstream>

#include <stdEx/mapEx.h>

struct Config
{
	int windowWidth = 800;
	int windowHeight = 600;
	std::string windowTitle;
	std::string startSave;
	bool fullscreenForce;
	bool debugText;
	bool defaultWASD;
};

void ReadConfigFile(Config& config)
{
	stdEx::map<std::string, int> expectedKeys;

	expectedKeys.emplace("WindowWidth",     0);
	expectedKeys.emplace("WindowHeight",    1);
	expectedKeys.emplace("WindowTitle",     2);
	expectedKeys.emplace("StartSave",       3);
	expectedKeys.emplace("FullscreenForce", 4);
	expectedKeys.emplace("DebugText",       5);
	expectedKeys.emplace("DefaultWASD",     6);
;
	expectedKeys.SetDefaultValue(-1);

	std::fstream file("config.ini", std::ios::in);
	std::string line;

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
}

int main(int argc, char* argv[])
{
	Config config;

	ReadConfigFile(config);

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