#include "pch.h"

#include <vector>

#include "NameEditChecker.h"
#include "AdString.h"

void NameEditChecker::CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning)
{
	if (nameCheckFunc)
	{
		AdString nameStr;
		nameEdit.GetWindowTextW(nameStr);

		AdString digitlessNameStdStr = CommonStrEdits::RemoveDigitsFromStringEnd(nameStr);
		RemoveRestrictedSymbs(digitlessNameStdStr);

		if (nameStr != digitlessNameStdStr)
		{
			nameEdit.SetWindowTextW(digitlessNameStdStr);
		}

		AdString nameStdStrChecked = nameCheckFunc(digitlessNameStdStr);

		nameWarning.ShowWindow(digitlessNameStdStr != nameStdStrChecked);
	}
}

void NameEditChecker::SetNameCheckFunc(NameCheckFunc nameCheckFuncInp)
{
	nameCheckFunc = nameCheckFuncInp;
}

void NameEditChecker::RemoveRestrictedSymbs(AdString& str)
{
	std::string& stdStr = str;

	for (auto c : restrictedSymbs)
	{
		stdStr.erase(std::remove(stdStr.begin(), stdStr.end(), c), stdStr.end());
	}
}