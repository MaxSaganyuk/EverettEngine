#pragma once

#include "IObjectSim.h"
#include "ISolidSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"
#include "ICameraSim.h"
#include "IColliderSim.h"

#include <optional>
#include <chrono>

// Engine interface is for external use, will only expose necessary calls to the engine from scripting
class IEverettEngine
{
public:
	enum class ObjectTypes
	{
		Camera, Solid, Light, Sound, Collider, _SIZE
	};

	struct TimedCallbackSetup
	{
		std::chrono::seconds period;
		std::function<void(size_t)> callback; // size_t will equal to amount of times 'this' callback was called
		std::optional<size_t> amountOfCalls; // Infinite if not set, stops callbacks on reached amount
		std::optional<std::reference_wrapper<bool>> endTrigger; // Stops callbacks on 'true' value (regardless on amount of calls)
	};

	virtual void SetDebugLogVisible(bool value = true) = 0;

	virtual void AddInteractable(
		int key,
		bool holdable,
		std::function<void()> pressFunc,
		std::function<void()> releaseFunc = nullptr
	) = 0;
	virtual void AddTimedCallback(TimedCallbackSetup timedCallbackSetup) = 0;
	virtual void AddMouseScrollCallback(std::function<void(double)> callback) = 0;
	virtual void AddMouseMoveCallback(std::function<void(double, double)> callback) = 0;

	virtual IObjectSim* GetObjectInterface(
		const char* objectName, std::optional<ObjectTypes> hintType = std::nullopt
	) = 0;
	virtual ISolidSim* GetSolidInterface(const char* solidName) = 0;
	virtual ILightSim* GetLightInterface(const char* lightName) = 0;
	virtual ISoundSim* GetSoundInterface(const char* soundName) = 0;
	virtual IColliderSim* GetColliderInterface(const char* colliderName) = 0;
	virtual ICameraSim* GetCameraInterface() = 0;

	virtual int ConvertKeyTo(char c) = 0;
	virtual int ConvertKeyTo(const char* keyName) = 0;

	virtual void RequestWorldLoad(const char* path) = 0;

	virtual void CreateLogReport() = 0;
};