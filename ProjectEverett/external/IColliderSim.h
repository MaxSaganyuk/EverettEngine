#pragma once

#include "InteractableSetup.h"

#include "IObjectSim.h"

class IColliderSim : virtual public IObjectSim 
{
public:
	struct CollisionCallbackOptions
	{
		InteractableSetup interSetup;
		// If null, collision with any collider will trigger callbacks. 
		// If not null, only collision with provided collider will.
		IColliderSim* colliderToBindTo = nullptr;
	};

	virtual void AddCollisionCallback(CollisionCallbackOptions callbackOpts) = 0;
	virtual void ClearCollisionCallbacks() = 0;
};