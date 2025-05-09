#pragma once

#include "afxdialogex.h"

class CBrowseDialog
{
public:
	static bool OpenAndGetFolderPath(CString& pathStr);
	static bool OpenAndGetFilePath(CString& pathStr, CString& fileStr, const CString& fileTypes = _T(".dll"));
};