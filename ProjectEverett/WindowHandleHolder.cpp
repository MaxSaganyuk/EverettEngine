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

void WindowHandleHolder::ExecuteFuncOnMainThread(const std::function<void()>& func)
{
	if (mainThreadExecuteInfo)
	{
		const auto& [name, id] = *mainThreadExecuteInfo;
		PostMessageToWindow(name, id, &func);
	}
}

void WindowHandleHolder::CloseWindow(const std::string& name)
{
	if (PostMessageToWindow(name, WM_CLOSE))
	{
		windowHandleMap.erase(name);
	}
}

bool WindowHandleHolder::PostMessageToWindow(const std::string& name, size_t messageID, const void* data)
{
	if (auto iter = windowHandleMap.find(name); iter != windowHandleMap.end())
	{
		return PostMessage(iter->second, static_cast<UINT>(messageID), reinterpret_cast<WPARAM>(data), 0);
	}

	return false;
}

bool WindowHandleHolder::IsMainThreadInfoSet()
{
	return static_cast<bool>(mainThreadExecuteInfo);
}

void WindowHandleHolder::SetMainThreadInfo(const std::string& windowName, size_t mainThreadPostID)
{
	mainThreadExecuteInfo = { windowName, mainThreadPostID };
}
