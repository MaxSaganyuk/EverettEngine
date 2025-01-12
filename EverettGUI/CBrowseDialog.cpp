#include "pch.h"
#include "CBrowseDialog.h"

bool CBrowseDialog::OpenAndGetFolderPath(CString& pathStr)
{
	BROWSEINFO browseInfo{ 0 };
	LPITEMIDLIST pidl = SHBrowseForFolderW(&browseInfo);

	if (pidl)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDListW(pidl, path))
		{
			pathStr = path;
			return true;
		}
		CoTaskMemFree(pidl);
	}

	return false;
}

bool CBrowseDialog::OpenAndGetFilePath(CString& pathStr)
{
	CFileDialog fileDlg(true, _T("dll"), nullptr, OFN_FILEMUSTEXIST, _T("DLL files (*.dll)|*.dll", this));

	if (fileDlg.DoModal() == IDOK)
	{
		pathStr = fileDlg.GetPathName();
		return true;
	}

	return false;
}