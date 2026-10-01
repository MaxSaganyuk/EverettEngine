#include "pch.h"

#include "CommonStrEdits.h"

#include "NameEditChecker.h"
#include "AdString.h"

namespace NameEditChecker
{
	std::string restrictedSymbs = "{}* ";
	NameCheckFunc nameCheckFunc = nullptr;

	void RemoveRestrictedSymbs(AdString& str)
	{
		std::string& stdStr = str;

		for (auto c : restrictedSymbs)
		{
			stdStr.erase(std::remove(stdStr.begin(), stdStr.end(), c), stdStr.end());
		}
	}

	void CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning)
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

	void SetNameCheckFunc(NameCheckFunc nameCheckFuncInp)
	{
		nameCheckFunc = nameCheckFuncInp;
	}

	AdString GetNameCheckedString(const AdString& str)
	{
		return nameCheckFunc(str);
	}
}