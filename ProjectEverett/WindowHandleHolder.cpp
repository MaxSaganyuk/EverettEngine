#include "WindowHandleHolder.h"

#include <Windows.h>

WindowHandleHolder::WindowHandleHolder()
{
	windowHandleMap.SetDefaultValue(nullptr);
}

void WindowHandleHolder::AddCurrentWindowHandle(const std::string& name)
{
	windowHandleMap.emplace(name, GetActiveWindow());
}

void WindowHandleHolder::BringWindowOnTop(const std::string& name)
{
	BringWindowToTop(windowHandleMap[name]);
}