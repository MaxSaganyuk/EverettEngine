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

void WindowHandleHolder::RemoveWindowHandle(const std::string& name)
{
	windowHandleMap.erase(name);
}

void WindowHandleHolder::BringWindowOnTop(const std::string& name)
{
	BringWindowToTop(windowHandleMap[name]);
}

void WindowHandleHolder::CloseWindow(const std::string& name)
{
	if(windowHandleMap.contains(name))
	{ 
		PostMessage(windowHandleMap[name], WM_CLOSE, 0, 0);
		windowHandleMap.erase(name);
	}
}