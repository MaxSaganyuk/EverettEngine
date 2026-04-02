#pragma once

#include "IObjectSim.h"

class IColliderSim : virtual public IObjectSim 
{
public:
	struct CollisionCallbackOptions
	{
		std::function<void()> collisionStart = nullptr;
		std::function<void()> collisionStop = nullptr;
		// If null, collision with any collider will trigger callbacks. 
		// If not null, only collision with provided collider will.
		IColliderSim* colliderToBindTo = nullptr;
		// If true, collisionStart callback will be called each frame
		// If false, only once collision detected. collisionStop is called only once regardless.
		bool holdable = false;
	};

	virtual void AddCollisionCallback(const CollisionCallbackOptions& callbackOpts) = 0;
	virtual void SetColliderActive(bool value = true) = 0;
};