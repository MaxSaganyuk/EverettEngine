#include <iostream>

#include "AssimpHelper.h"

AssimpHelper::AssimpHelper(const std::string& file) 
{
	Assimp::Importer importer;
	modelHandle = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs);
	
	if (!modelHandle || modelHandle->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelHandle->mRootNode)
	{
		std::cerr << "AssimpHelper error modleHandle is invalid\n";
		return;
	}

	ProcessNode(modelHandle->mRootNode);
}

LGLStructs::MeshInfo AssimpHelper::ProcessMesh(const aiMesh* meshHandle)
{
	auto GetVertexParam = [&meshHandle](LGLStructs::Vertex::VertexData whichData)
	{
		using VertexData = LGLStructs::Vertex::VertexData;

		aiVector3D* tempVector = nullptr;

		switch (whichData)
		{
		case VertexData::Position:
			tempVector = meshHandle->mVertices;
			break;
		case VertexData::Normal:
			tempVector = meshHandle->mNormals;
			break;
		case VertexData::TexCoords:
			tempVector = meshHandle->mTextureCoords[0];
			break;
		case VertexData::Tangent:
			tempVector = meshHandle->mTangents;
			break;
		case VertexData::Bitangent:
			tempVector = meshHandle->mBitangents;
			break;
		default:
			assert(false && "Vertex param does not exist");
		}

		return tempVector;
	};

	auto ProcessVerteces = [&meshHandle, &GetVertexParam](LGLStructs::MeshInfo& mesh)
	{
		for (size_t i = 0; i < meshHandle->mNumVertices; ++i)
		{
			LGLStructs::Vertex vert;
			for (size_t vertData = 0; vertData < LGLStructs::Vertex::DataAmount; ++vertData)
			{
				glm::vec3 glmVect{ 0.0f, 0.0f, 0.0f };
				aiVector3D* assimpVect = GetVertexParam(static_cast<LGLStructs::Vertex::VertexData>(vertData));
	
				if (assimpVect)
				{
					for (size_t axis = 0; axis < 3; ++axis)
					{
						glmVect[axis] = (assimpVect + i)->operator[](axis);
					}
				}
				
				vert[vertData] = glmVect;
			}

			mesh.vert.push_back(vert);
		}
	};

	auto ProcessFaces = [&meshHandle](LGLStructs::MeshInfo& mesh)
	{
		for (size_t i = 0; i < meshHandle->mNumFaces; ++i)
		{
			aiFace face = meshHandle->mFaces[i];

			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				mesh.indices.push_back(face.mIndices[j]);
			}
		}
	};

	LGLStructs::MeshInfo mesh;

	ProcessVerteces(mesh);
	ProcessFaces(mesh);

	return mesh;
}

void AssimpHelper::ProcessNode(const aiNode* nodeHandle)
{
	for (size_t i = 0; i < nodeHandle->mNumMeshes; ++i)
	{
		aiMesh* meshHandle = modelHandle->mMeshes[nodeHandle->mMeshes[i]];
		model.push_back(ProcessMesh(meshHandle));
	}

	for (size_t i = 0; i < nodeHandle->mNumChildren; ++i)
	{
		ProcessNode(nodeHandle->mChildren[i]);
	}
}

void AssimpHelper::GetModel(LGLStructs::ModelInfo& model)
{
	model = this->model;
}