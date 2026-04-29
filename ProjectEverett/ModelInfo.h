#pragma once

#include <string>
#include <unordered_set>
#include <functional>

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
	void SetModelBehaviour(std::function<void()> func);
	void SetGeneralMeshBehaviour(std::function<void(int)> func);
	const std::unordered_set<SolidSim*>& GetRelatedSolids() const;
private:
	void SetupModelInfo(bool set);

	std::function<void()> modelBehaviour;
	std::function<void(int)> generalMeshBehaviour;

	std::string modelName;
	std::string modelPath;
	FullModelInfo model;
	std::unordered_set<SolidSim*> relatedSolids;
};