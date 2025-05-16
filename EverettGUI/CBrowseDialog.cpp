#include "pch.h"
#include "CBrowseDialog.h"

CString CBrowseDialog::GetFileFilterString(const std::vector<std::pair<DescriptionString, FileTypeString>>& fileFilter)
{
	AdString fileFilterStr = "";

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

	return fileFilterStr;
}

bool CBrowseDialog::OpenAndGetFolderPath(AdString& pathStr)
{
	CFolderPickerDialog folderDlg(nullptr, OFN_PATHMUSTEXIST);

	if (folderDlg.DoModal() == IDOK)
	{
		pathStr = folderDlg.GetPathName();
		return true;
	}

	return false;
}

bool CBrowseDialog::OpenAndGetFilePath(
	AdString& pathStr,
	AdString& fileStr,
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