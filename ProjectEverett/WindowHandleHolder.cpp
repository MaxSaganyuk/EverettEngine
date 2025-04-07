#include "WindowHandleHolder.h"

#include <Windows.h>

WindowHandleHolder::WindowHandleHolder()
{
	windowHandleMap.SetDefaultValue(nullptr);
}

void WindowHandleHolder::AddCurrentWindowHandle(const std::string& name)
{
	AddWindowHandle(nullptr, name);
}

void WindowHandleHolder::AddWindowHandle(HWND windowHandle, const std::string& name)
{
	windowHandleMap.emplace(name, windowHandle ? windowHandle : GetActiveWindow());
}

void WindowHandleHolder::BringWindowOnTop(const std::string& name)
{
	BringWindowToTop(windowHandleMap[name]);
}