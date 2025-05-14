#include "pch.h"
#include "NameEditChecker.h"
#include "AdString.h"

void NameEditChecker::CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning)
{
	if (nameCheckFunc)
	{
		AdString nameStr;
		nameEdit.GetWindowTextW(nameStr);

		AdString digitlessNameStdStr = CommonStrEdits::RemoveDigitsFromStringEnd(nameStr);

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

NameEditChecker::NameCheckFunc NameEditChecker::nameCheckFunc = nullptr;