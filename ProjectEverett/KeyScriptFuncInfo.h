#pragma once

#include <vector>
#include <functional>

class KeyScriptFuncInfo
{
	struct PressedFuncInfo
	{
		std::function<void()> func;
		bool holdable{};
		bool persistent{};
	};

	bool pressed{};
	std::vector<PressedFuncInfo> pressedFuncs;
	std::vector<std::function<void()>> releasedFuncs;

public:
	void AddPressedFunc(PressedFuncInfo&& pressedFuncInfo)
	{
		pressedFuncs.push_back(std::move(pressedFuncInfo));
	}

	void AddReleasedFunc(std::function<void()>&& func)
	{
		releasedFuncs.push_back(std::move(func));
	}

	bool Clear()
	{
		std::erase_if(pressedFuncs, [](PressedFuncInfo& info) { return !info.persistent; });
		releasedFuncs.clear();

		return pressedFuncs.empty();
	}

	void ButtonPressed()
	{
		for (auto& [func, holdable, _] : pressedFuncs)
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