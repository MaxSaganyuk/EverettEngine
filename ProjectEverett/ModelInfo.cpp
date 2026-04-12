#include "SolidSim.h"
#include "ModelInfo.h"

ModelInfo::ModelInfo(const std::string& modelName, const std::string& modelPath, FullModelInfo&& model)
	: modelName(modelName), modelPath(modelPath), model(std::move(model)) {}

ModelInfo::~ModelInfo()
{
	ResetModelPtr();
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
	solid.STMM.InitializeSTMM(model, modelName);
	relatedSolids.insert(&solid);
}

void ModelInfo::EraseFromRelatedSolids(SolidSim& solid)
{
	relatedSolids.erase(&solid);

	if (relatedSolids.empty())
	{
		ResetModelPtr();
	}
}

const std::unordered_set<SolidSim*>& ModelInfo::GetRelatedSolids() const
{
	return relatedSolids;
}

void ModelInfo::ResetModelPtr()
{
	auto modelPtr = model.first.lock();

	if (modelPtr)
	{
		modelPtr->render = false;
		modelPtr->modelBehaviour = nullptr;
		modelPtr->generalMeshBehaviour = nullptr;

		for (auto& mesh : modelPtr->meshes)
		{
			mesh.behaviour.ResetValue();
		}
	}
}