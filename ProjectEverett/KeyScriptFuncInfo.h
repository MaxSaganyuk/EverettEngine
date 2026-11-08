#pragma once

#include <vector>
#include <functional>

#include "external/InteractableSetup.h"

class KeyScriptFuncInfo
{
	struct InteractableInfo
	{
		InteractableSetup callbacks;
		bool persistent{};
	};

	bool pressed{};
	std::vector<InteractableInfo> interInfo;

public:
	void AddCallbacks(InteractableInfo&& pressedFuncInfo)
	{
		interInfo.push_back(std::move(pressedFuncInfo));
	}

	bool Clear()
	{
		std::erase_if(interInfo, [](InteractableInfo& info) { return !info.persistent; });

		return interInfo.empty();
	}

	void ButtonPressed()
	{
		for (auto& [callbacks, _] : interInfo)
		{
			if (!pressed && callbacks.startFunc)
			{
				callbacks.startFunc();
			}

			if (callbacks.processFunc)
			{
				callbacks.processFunc();
			}
		}

		pressed = true;
	}

	void ButtonReleased()
	{
		pressed = false;

		for (auto& [callbacks, _] : interInfo)
		{
			if (callbacks.stopFunc)
			{
				callbacks.stopFunc();
			}
		}
	}
};