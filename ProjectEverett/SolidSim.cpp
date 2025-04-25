#include "SolidSim.h"

void SolidSim::ForceModelUpdate()
{
	ResetModelMatrix();
}

void SolidSim::ResetModelMatrix(const Rotation& toRotate)
{
	rotate += toRotate;

	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, scale);
	// gimbal lock will happen, will rewrite to use quaternions later
	for (int i = 0; i <3; ++i)
	{
		model = glm::rotate(model, rotate[i], { i == 0, i == 1, i == 2 });
	}
}

SolidSim::SolidSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const glm::vec3& front,
	const float speed
)
	: ObjectSim(pos, scale, front, speed)
{
	ResetModelMatrix();
	type = SolidType::Static;
}

glm::mat4& SolidSim::GetModelMatrixAddr()
{
	return model;
}

void SolidSim::SetType(SolidType type)
{
	this->type = type;
}

void SolidSim::SetPosition(ObjectSim::Direction dir, const glm::vec3& limitAxis)
{
	ObjectSim::SetPosition(dir, limitAxis);

	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix();
	}
	else 
	{
		model[3].x = pos.x;
		model[3].y = pos.y;
		model[3].z = pos.z;
	}
}

void SolidSim::Rotate(const Rotation& toRotate)
{
	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix(toRotate);
	}
	else 
	{
		for (int i = 0; i <3; ++i)
		{
			if (toRotate[i])
			{
				model = glm::rotate(model, toRotate[i], { i == 0, i == 1, i == 2 });
				rotate[i] += toRotate[i];
			}
		}
	}

	ObjectSim::Rotate(toRotate);
}

bool SolidSim::CheckForCollision(const SolidSim& solid1, const SolidSim& solid2)
{
	if (solid1.IsGhostMode() || solid2.IsGhostMode())
	{
		return false;
	}

	bool res = true;

	for (int i = 0; i <3; ++i)
	{
		res = ((solid1.pos[i] + solid1.scale[i] / 2) - (solid2.pos[i] - solid2.scale[i] / 2)) > 0.0f &&
			  ((solid1.pos[i] - solid1.scale[i] / 2) - (solid2.pos[i] + solid2.scale[i] / 2)) < 0.0f;

		if (!res) break;
	}

	return res;
}

bool SolidSim::CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2)
{
	return CheckForCollision(solid1, solid2);
}

size_t SolidSim::GetMeshAmount()
{
	return STMM.GetMeshAmount();
}
void SolidSim::SetAllMeshVisibility(bool value)
{
	STMM.SetAllMeshVisibility(value);
}

void SolidSim::SetBackwardsModelAccess(SolidToModelManager::FullModelInfo& model)
{
	STMM.InitializeSTMM(model);
}

std::vector<std::string> SolidSim::GetModelMeshNames()
{
	return STMM.GetMeshNames();
}

void SolidSim::SetModelMeshVisibility(const std::string name, bool value)
{
	STMM.SetMeshVisibility(name, value);
}

void SolidSim::SetModelMeshVisibility(size_t index, bool value)
{
	STMM.SetMeshVisibility(index, value);
}

bool SolidSim::GetModelMeshVisibility(const std::string name)
{
	return STMM.GetMeshVisibility(name);
}

bool SolidSim::GetModelMeshVisibility(size_t index)
{
	return STMM.GetMeshVisibility(index);
}