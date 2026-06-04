#include "SolidSim.h"
#include "ModelInfo.h"

ModelInfo::ModelInfo(const std::string& modelPath, FullModelInfo&& model)
	: modelPath(modelPath), model(std::move(model)) {}

ModelInfo::~ModelInfo()
{
	SetupModelInfo(false);
}

void ModelInfo::SetModelNamePtr(const std::string& modelAddr)
{
	modelNamePtr = &modelAddr;
}

const std::string& ModelInfo::GetModelName() const
{
	CheckModelNamePtrSet();

	return *modelNamePtr;
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
	CheckModelNamePtrSet();

	if (relatedSolids.empty())
	{
		SetupModelInfo(true);
	}

	solid.STMM.InitializeSTMM(model, modelNamePtr);
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

void ModelInfo::CheckModelNamePtrSet() const
{
	CheckAndThrowExceptionWMessage(modelNamePtr, "ModelNamePtr is unset");
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