#pragma once

#include "IObjectSim.h"

class IColliderSim : virtual public IObjectSim 
{
public:
	virtual void AddAnyCollisionCallback(
		std::function<void()> collisionStart, std::function<void()> collisionStop = nullptr
	) = 0;
	virtual void AddBindedCollisionCallback(
		IColliderSim& otherCollider, std::function<void()> collisionStart, std::function<void()> collisionStop = nullptr
	) = 0;
	virtual void SetColliderActive(bool value = true) = 0;
};