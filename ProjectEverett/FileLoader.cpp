#include "FileLoader.h"
#include <Windows.h>

bool FileLoader::GetFilesInDir(std::vector<std::string>& files, const std::string& dir)
{
	HANDLE dirHandle;
	WIN32_FIND_DATAA fileData;

	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);
	std::string currentDir = path;
	std::string pathToCheck = currentDir + '\\' + dir + "\\*";

	if ((dirHandle = FindFirstFileA(pathToCheck.c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		const std::string fileName = fileData.cFileName;
		const bool isDir = fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		if (fileName[0] == '.') continue;
		if (isDir) continue;

		files.push_back(fileName);

	} while (FindNextFileA(dirHandle, &fileData));

	FindClose(dirHandle);

	return true;
}
