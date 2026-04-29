#include "SolidSim.h"
#include "ModelInfo.h"

ModelInfo::ModelInfo(const std::string& modelName, const std::string& modelPath, FullModelInfo&& model)
	: modelName(modelName), modelPath(modelPath), model(std::move(model)) {}

ModelInfo::~ModelInfo()
{
	SetupModelInfo(false);
}

const std::string& ModelInfo::GetModelName() const
{
	return modelName;
}

const std::string& ModelInfo::GetModelPath() const
{
	return modelPath;
}

const ModelInfo::FullModelInfo& ModelInfo::GetFullModelInfo() const
{
	return model;
}

void ModelInfo::InsertRelatedSolid(SolidSim& solid)
{
	if (relatedSolids.empty())
	{
		SetupModelInfo(true);
	}

	solid.STMM.InitializeSTMM(model, modelName);
	relatedSolids.insert(&solid);
}

void ModelInfo::EraseFromRelatedSolids(SolidSim& solid)
{
	relatedSolids.erase(&solid);

	if (relatedSolids.empty())
	{
		SetupModelInfo(false);
	}
}

const std::unordered_set<SolidSim*>& ModelInfo::GetRelatedSolids() const
{
	return relatedSolids;
}

void ModelInfo::SetModelBehaviour(std::function<void()> func)
{
	modelBehaviour = std::move(func);
}

void ModelInfo::SetGeneralMeshBehaviour(std::function<void(int)> func)
{
	generalMeshBehaviour = std::move(func);
}

void ModelInfo::SetupModelInfo(bool set)
{
	auto modelPtr = model.first.lock();

	if (modelPtr)
	{
		modelPtr->render = set;
		modelPtr->modelBehaviour = set ? modelBehaviour : nullptr;
		modelPtr->generalMeshBehaviour = set ? generalMeshBehaviour : nullptr;

		if (!set)
		{
			for (auto& mesh : modelPtr->meshes)
			{
				mesh.behaviour.ResetValue();
			}
		}
	}
}