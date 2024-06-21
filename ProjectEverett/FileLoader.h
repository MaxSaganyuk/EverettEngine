#pragma once

#include <vector>
#include <string>

namespace FileLoader
{
	bool GetFilesInDir(std::vector<std::string>& files, const std::string& dir);
}