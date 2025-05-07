#include "pch.h"
#include "CBrowseDialog.h"

bool CBrowseDialog::OpenAndGetFolderPath(CString& pathStr)
{
	bool success = false;

	BROWSEINFO browseInfo{ 0 };
	LPITEMIDLIST pidl = SHBrowseForFolderW(&browseInfo);

	if (pidl)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDListW(pidl, path))
		{
			pathStr = path;
			success = true;
		}
		CoTaskMemFree(pidl);
	}

	return success;
}

bool CBrowseDialog::OpenAndGetFilePath(CString& pathStr, CString& fileStr, const CString& fileTypes)
{
	CFileDialog fileDlg(true, fileTypes, nullptr, OFN_FILEMUSTEXIST);

	if (fileDlg.DoModal() == IDOK)
	{
		pathStr = fileDlg.GetPathName();
		fileStr = fileDlg.GetFileName();
		return true;
	}

	return false;
}