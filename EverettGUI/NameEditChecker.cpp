#include "pch.h"
#include "NameEditChecker.h"

void NameEditChecker::CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning)
{
	if (nameCheckFunc)
	{
		CString nameStr;
		nameEdit.GetWindowTextW(nameStr);

		std::string nameStdStr = CT2A(nameStr);
		std::string digitlessNameStdStr = CommonStrEdits::RemoveDigitsFromStringEnd(nameStdStr);

		if (nameStdStr != digitlessNameStdStr)
		{
			nameEdit.SetWindowTextW(CA2T(digitlessNameStdStr.c_str()));
		}

		std::string nameStdStrChecked = nameCheckFunc(digitlessNameStdStr);

		nameWarning.ShowWindow(digitlessNameStdStr != nameStdStrChecked);
	}
}

void NameEditChecker::SetNameCheckFunc(NameCheckFunc nameCheckFuncInp)
{
	nameCheckFunc = nameCheckFuncInp;
}

NameEditChecker::NameCheckFunc NameEditChecker::nameCheckFunc = nullptr;