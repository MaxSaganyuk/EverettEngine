#pragma once

#include <functional>
#include <string>

#include "AdString.h"

class NameEditChecker
{
public:
	using NameCheckFunc = std::function<std::string(const std::string&)>;

	static void CheckAndEditName(CEdit& nameEdit, CStatic& nameWarning);
	static void SetNameCheckFunc(NameCheckFunc nameCheckFuncInp);
	static AdString GetNameCheckedString(const AdString& str);
private:
	static void RemoveRestrictedSymbs(AdString& str);

	static inline std::string restrictedSymbs = "{}* ";
	static inline NameCheckFunc nameCheckFunc = nullptr;
};
