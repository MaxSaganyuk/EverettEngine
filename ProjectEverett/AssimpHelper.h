#pragma once

#ifndef LGL_DISABLEASSIMP
#include <assimp/Importer.hpp>
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#endif

#include "LGLStructs.h"

// Add texture params
class AssimpHelper
{
	const aiScene* modelHandle;
	LGLStructs::ModelInfo model;

	void ProcessNode(const aiNode* nodeHandle);
	LGLStructs::MeshInfo ProcessMesh(const aiMesh* meshHandle);
public:
	AssimpHelper(const std::string& file);
	void GetModel(LGLStructs::ModelInfo& model);
};