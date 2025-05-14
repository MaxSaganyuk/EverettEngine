#pragma once

#include "afxdialogex.h"
#include <vector>
#include <string>

#include "AdString.h"

class CBrowseDialog
{
	using DescriptionString = AdString;
	using FileTypeString = AdString;

	static CString GetFileFilterString(const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter);

public:
	static bool OpenAndGetFolderPath(AdString& pathStr);
	static bool OpenAndGetFilePath(
		AdString& pathStr, 
		AdString& fileStr, 
		const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter
	);
};