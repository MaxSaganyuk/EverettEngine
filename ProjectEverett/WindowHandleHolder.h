#pragma once

#include <map>
#include <string>
#include <vector>
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
	void CloseWindow(const std::string& name);
private:
	stdEx::map<std::string, HWND> windowHandleMap;
};