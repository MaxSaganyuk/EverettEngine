#include "SolidSim.h"
#include "ModelInfo.h"

#include <algorithm>

ModelInfo::ModelInfo(const std::string& modelPath, FullModelInfo&& model)
	: modelPath(modelPath), model(std::move(model)) {}

ModelInfo::~ModelInfo()
{
	SetupModelInfo(false);
}

void ModelInfo::RecheckIfAllRelatedSolidsAreVisible()
{
	model.first.lock()->render = std::any_of(
		relatedSolids.begin(), relatedSolids.end(), [](SolidSim* solid) { return solid->GetModelVisibility(); }
	);
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

void ModelInfo::InsertRelatedSolid(SolidSim& solid)
{
	CheckModelNamePtrSet();

	if (relatedSolids.empty())
	{
		SetupModelInfo(true);
	}

	solid.STMM.InitializeSTMM(*this, modelNamePtr);
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

void ModelInfo::SetModelBehaviour(std::function<void(const ModelInfo&)> func)
{
	modelBehaviour = [this, func = std::move(func)]() { func(*this); };
}

void ModelInfo::SetGeneralMeshBehaviour(std::function<void(const ModelInfo&, int)> func)
{
	generalMeshBehaviour = [this, func = std::move(func)](int meshIndex) { func(*this, meshIndex); };
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