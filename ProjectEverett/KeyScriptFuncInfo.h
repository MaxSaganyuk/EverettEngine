#pragma once

#include <vector>
#include <functional>

class KeyScriptFuncInfo
{
	bool pressed{};
	std::vector<std::pair<std::function<void()>, bool>> pressedFuncs;
	std::vector<std::function<void()>> releasedFuncs;

public:
	void AddPressedFunc(std::function<void()> func, bool holdable)
	{
		pressedFuncs.push_back({ std::move(func), holdable });
	}

	void AddReleasedFunc(std::function<void()> func)
	{
		releasedFuncs.push_back(std::move(func));
	}

	void ButtonPressed()
	{
		for (auto& [func, holdable] : pressedFuncs)
		{
			if (func && (!pressed || holdable))
			{
				func();
			}
		}

		pressed = true;
	}

	void ButtonReleased()
	{
		pressed = false;

		for (auto& func : releasedFuncs)
		{
			if (func)
			{
				func();
			}
		}
	}
};