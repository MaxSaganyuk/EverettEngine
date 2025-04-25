#pragma once

#include "IObjectSim.h"

#include <functional>
#include <vector>

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

	virtual std::vector<std::string> GetModelMeshNames() = 0;
	virtual size_t GetMeshAmount() = 0;
	virtual void SetAllMeshVisibility(bool value) = 0;
	virtual void SetModelMeshVisibility(const std::string name, bool value) = 0;
	virtual void SetModelMeshVisibility(size_t index, bool value) = 0;
	virtual bool GetModelMeshVisibility(const std::string name) = 0;
	virtual bool GetModelMeshVisibility(size_t index) = 0;

	virtual bool CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2) = 0;
};