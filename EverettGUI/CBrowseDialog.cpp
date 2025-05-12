#include "pch.h"
#include "CBrowseDialog.h"

CString CBrowseDialog::GetFileFilterString(const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter)
{
	std::string fileFilterStr = "";

	for (auto& currentFileFilter : fileFilter)
	{
		fileFilterStr += (
			currentFileFilter.first + 
			'(' + 
			currentFileFilter.second + 
			")|" +
			currentFileFilter.second + 
			'|'
		);
	}

	fileFilterStr += '|';

	return CA2T(fileFilterStr.c_str());
}

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

bool CBrowseDialog::OpenAndGetFilePath(
	CString& pathStr,
	CString& fileStr,
	const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter
)
{
	CFileDialog fileDlg(true, _T(""), nullptr, OFN_FILEMUSTEXIST, GetFileFilterString(fileFilter));

	if (fileDlg.DoModal() == IDOK)
	{
		pathStr = fileDlg.GetPathName();
		fileStr = fileDlg.GetFileName();
		return true;
	}

	return false;
}