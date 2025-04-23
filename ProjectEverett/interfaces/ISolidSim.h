#pragma once

#include "IObjectSim.h"

#include <functional>

class ISolidSim : virtual public IObjectSim
{
public:
	enum class SolidType
	{
		Static, // Set if solid is unchanging or changes it's position, rotation or scale rarely
		Dynamic // UNIMPLEMENTED Set if solid changes it's position, rotation or scale constantly or often
	};

	constexpr static float fullRotation = 360.0f;

	virtual void ForceModelUpdate() = 0;
	virtual glm::mat4& GetModelMatrixAddr() = 0;
	virtual void SetType(SolidType type) = 0;

	virtual bool CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2) = 0;
};