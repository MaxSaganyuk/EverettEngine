#pragma once

#include <string>
#include <optional>
#include <functional>
#include <stdEx/mapEx.h>

struct HWND__;
using HWND = HWND__*;

class WindowHandleHolder
{
public:
	WindowHandleHolder();
	void AddCurrentWindowHandle(const std::string& name);
	void AddWindowHandle(HWND windowHandle, const std::string& name);
	void RemoveWindowHandle(const std::string& name);
	void BringWindowOnTop(const std::string& name);

	void ExecuteFuncOnMainThread(const std::function<void()>& func);
	void CloseWindow(const std::string& name);
	bool PostMessageToWindow(const std::string& name, size_t messageID, const void* data = nullptr);

	bool IsMainThreadInfoSet();
	void SetMainThreadInfo(const std::string& windowName, size_t mainThreadPostID);
private:
	stdEx::map<std::string, HWND> windowHandleMap;
	std::optional<std::pair<std::string, size_t>> mainThreadExecuteInfo;
};