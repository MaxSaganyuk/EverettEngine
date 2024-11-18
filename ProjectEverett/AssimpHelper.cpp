#include <iostream>
#include <map>

#include "AssimpHelper.h"
#include "FileLoader.h"

AssimpHelper::AssimpHelper(const std::string& file) 
{
	Assimp::Importer importer;
	modelHandle = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs);
	
	if (!modelHandle || modelHandle->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelHandle->mRootNode)
	{
		std::cerr << "AssimpHelper error modleHandle is invalid\n";
		return;
	}

	GetTextureFilenames(file);
	ProcessNode(modelHandle->mRootNode);
}

bool AssimpHelper::GetTextureFilenames(const std::string& path)
{
	auto GetTexturePath = [](const std::string& path)
	{
		return path.substr(0, path.rfind('\\') + 1) + "textures";
	};

	return FileLoader::GetFilesInDir(texturesFound, GetTexturePath(path));
}

LGLStructs::Mesh AssimpHelper::ProcessMesh(const aiMesh* meshHandle)
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

	auto ProcessVerteces = [&meshHandle, &GetVertexParam](LGLStructs::Mesh& mesh)
	{
		for (size_t i = 0; i < meshHandle->mNumVertices; ++i)
		{
			LGLStructs::Vertex vert;
			for (size_t vertData = 0; vertData < LGLStructs::Vertex::GetMemberAmount(); ++vertData)
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

	auto ProcessFaces = [&meshHandle](LGLStructs::Mesh& mesh)
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

	auto ProcessTextures = [this, &meshHandle](LGLStructs::Mesh& mesh)
	{
		using TextureType = LGLStructs::Texture::TextureType;

		struct TextureTypeInfo
		{
			TextureType lglTexType;
			aiTextureType assimpTexType;
			char filenameC;
		};

		static std::vector<TextureTypeInfo> textureTypeInfo
		{
			{TextureType::Diffuse,  aiTextureType_DIFFUSE,  'd'},
			{TextureType::Specular, aiTextureType_SPECULAR, 's'},
			{TextureType::Normal,   aiTextureType_NORMALS,  'n'},
			{TextureType::Height,   aiTextureType_HEIGHT,   'h'}
		};

		auto LoadTextureByMaterial = [](LGLStructs::Mesh& mesh, aiMaterial* material, size_t texTypeIndex)
		{
			aiTextureType convertedType = textureTypeInfo[texTypeIndex].assimpTexType;

			for (size_t i = 0; i < material->GetTextureCount(convertedType); ++i)
			{
				aiString str;
				material->GetTexture(convertedType, i, &str);
				std::string strWithoutPrefix = str.C_Str();

				while (strWithoutPrefix.find('\\') != std::string::npos)
				{
					strWithoutPrefix = strWithoutPrefix.substr(strWithoutPrefix.find('\\') + 1);
				}
				
				mesh.textures.push_back({ strWithoutPrefix, textureTypeInfo[texTypeIndex].lglTexType });
			}
		};

		auto CheckForAdditionalTextures = [this](LGLStructs::Mesh& mesh)
		{
			std::vector<bool> foundTexTypes(LGLStructs::Texture::GetTextureTypeAmount(), false);
			std::string name = mesh.textures.front().name;

			for (auto& tex : mesh.textures)
			{
				foundTexTypes[static_cast<int>(tex.type)] = true;
			}

			for(size_t i = 0; i < LGLStructs::Texture::GetTextureTypeAmount(); ++i)
			{
				if (!foundTexTypes[i])
				{
					//if(name.find('.') !)
					bool isUpper = std::isupper(name[name.find('.') - 1]);
					name[name.find('.') - 1] = isUpper ? std::toupper(textureTypeInfo[i].filenameC) : textureTypeInfo[i].filenameC;
					if (std::find(texturesFound.begin(), texturesFound.end(), name) != texturesFound.end())
					{
						mesh.textures.push_back({ name, static_cast<LGLStructs::Texture::TextureType>(i) });
					}
				}
			}
		};

		aiMaterial* material = modelHandle->mMaterials[meshHandle->mMaterialIndex];

		for (size_t i = 0; i < LGLStructs::Texture::GetTextureTypeAmount(); ++i)
		{
			LoadTextureByMaterial(mesh, material, i);
			if (mesh.textures.size())
			{
				CheckForAdditionalTextures(mesh);
			}
		}
	};

	LGLStructs::Mesh mesh;

	ProcessVerteces(mesh);
	ProcessFaces(mesh);
	ProcessTextures(mesh);

	return mesh;
}

void AssimpHelper::ProcessNode(const aiNode* nodeHandle)
{
	for (size_t i = 0; i < nodeHandle->mNumMeshes; ++i)
	{
		aiMesh* meshHandle = modelHandle->mMeshes[nodeHandle->mMeshes[i]];
		model.AddMesh(ProcessMesh(meshHandle));
	}

	for (size_t i = 0; i < nodeHandle->mNumChildren; ++i)
	{
		ProcessNode(nodeHandle->mChildren[i]);
	}
}


void AssimpHelper::GetModel(const std::string& file, LGLStructs::ModelInfo& model)
{
	AssimpHelper helper(file);
	model = helper.model;
}