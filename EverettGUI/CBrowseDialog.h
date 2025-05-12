#pragma once

#include "afxdialogex.h"
#include <vector>
#include <string>

class CBrowseDialog
{
	using DescriptionString = std::string;
	using FileTypeString = std::string;

	static CString GetFileFilterString(const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter);

public:
	static bool OpenAndGetFolderPath(CString& pathStr);
	static bool OpenAndGetFilePath(
		CString& pathStr, 
		CString& fileStr, 
		const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter
	);
};