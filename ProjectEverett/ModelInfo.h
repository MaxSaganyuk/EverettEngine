#pragma once

#include <string>
#include <unordered_set>

#include "LGLStructs.h"
#include "AnimSystem.h"

class SolidSim;

class ModelInfo
{
public:
	using FullModelInfo = std::pair<std::weak_ptr<LGLStructs::ModelInfo>, std::weak_ptr<AnimSystem::ModelAnim>>;

	ModelInfo() = default;
	ModelInfo(const std::string& modelName, const std::string& modelPath, FullModelInfo&& model);

	~ModelInfo();

	const std::string& GetModelName() const;
	const std::string& GetModelPath() const;
	const FullModelInfo& GetFullModelInfo() const;
	void InsertRelatedSolid(SolidSim& solid);
	void EraseFromRelatedSolids(SolidSim& solid);
	const std::unordered_set<SolidSim*>& GetRelatedSolids() const;
private:
	void ResetModelPtr();

	std::string modelName;
	std::string modelPath;
	FullModelInfo model;
	std::unordered_set<SolidSim*> relatedSolids;
};