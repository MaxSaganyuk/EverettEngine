#pragma once

#include <functional>
#include <string>

#include "AdString.h"

namespace NameEditChecker
{
	using NameCheckFunc = std::function<std::string(const std::string&)>;

	void CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning);
	void SetNameCheckFunc(NameCheckFunc nameCheckFuncInp);
	AdString GetNameCheckedString(const AdString& str);
};
