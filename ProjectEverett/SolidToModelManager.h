#pragma once

#include <vector>
#include <string>

#include "LGLStructs.h"
#include "AnimSystem.h"

class SolidToModelManager
{
public:
	using FullModelInfo = std::pair<LGLStructs::ModelInfo, AnimSystem::ModelAnim>;

	SolidToModelManager();

	void InitializeSTMM(FullModelInfo& fullModelInfoRef);

	std::vector<std::string> GetMeshNames();
	size_t GetMeshAmount();
	void SetAllMeshVisibility(bool value);
	void SetMeshVisibility(size_t intex, bool value);
	void SetMeshVisibility(const std::string& name, bool value);
	bool GetMeshVisibility(size_t intex);
	bool GetMeshVisibility(const std::string& name);

private:
	void CheckIfInitialized();

	template<typename Type>
	size_t GetIndexByName(const std::string& name, const std::vector<Type>& cont);

	bool initialized;

	std::vector<bool> meshVisibility;
	FullModelInfo* fullModelInfoP;
};