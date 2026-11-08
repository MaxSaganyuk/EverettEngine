#pragma once

#include <functional>

// Interactable logic is separated to start, process and stop.
// On the frame triggering execution - callbacks are processed in following way:
// Trigger enables.
// Start   - Executes once on first frame only
// Process - Executes each frame
// Trigger disables:
// Stop    - Executes once on single frame after the last
struct InteractableSetup
{
	std::function<void()> startFunc;
	std::function<void()> processFunc;
	std::function<void()> stopFunc;

	bool AnySet() const noexcept
	{
		return startFunc || processFunc || stopFunc;
	}
};