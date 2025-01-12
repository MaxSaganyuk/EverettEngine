#include <iostream>
#include <map>
#include <Windows.h>

#include <assimp/Importer.hpp>
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "FileLoader.h"

#include "stb_image.h"

static std::map<std::string, HMODULE> dllHandleMap;

std::string FileLoader::GetCurrentDir()
{
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	return path;
}

bool FileLoader::GetFilesInDir(std::vector<std::string>& files, const std::string& dir)
{
	HANDLE dirHandle;
	WIN32_FIND_DATAA fileData;

	std::string pathToCheck = dir + "\\*";

	if (dir.find(':') == dir.npos)
	{
		pathToCheck = GetCurrentDir() + '\\' + pathToCheck;
	}

	if ((dirHandle = FindFirstFileA(pathToCheck.c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		const std::string fileName = fileData.cFileName;
		const bool isDir = fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		if (fileName[0] == '.') continue;
		if (isDir) continue;

		files.push_back(fileName);

	} while (FindNextFileA(dirHandle, &fileData));

	FindClose(dirHandle);

	return true;
}

bool FileLoader::LoadTexture(
	const std::string& file, 
	LGLStructs::Texture& texture, 
	unsigned char* data, 
	size_t dataSize
)
{
	stbi_set_flip_vertically_on_load(data == nullptr);

	texture.data = data ? 
		stbi_load_from_memory(data, dataSize, &texture.width, &texture.height, &texture.channelAmount, 0) :
		stbi_load(file.c_str(), &texture.width, &texture.height, &texture.channelAmount, 0);

	if (texture.data)
	{
		std::string textureName = texture.name.size() ? texture.name : file;

		while (textureName.find('\\') != std::string::npos)
		{
			textureName = textureName.substr(textureName.find('\\') + 1);
		}

		texture.name = textureName;
		texturesLoaded[texture.name] = texture;

		return true;
	}

	return false;
}

FileLoader::FileLoader() {}

FileLoader::~FileLoader()
{
	FreeTextureData();
	FreeDllData();
}

bool FileLoader::GetTextureFilenames(const std::string& path)
{
	auto GetTexturePath = [](const std::string& path)
	{
		return path.substr(0, path.rfind('\\') + 1) + "textures";
	};

	return GetFilesInDir(extraTextureName, GetTexturePath(path));
}

LGLStructs::Mesh FileLoader::ProcessMesh(const aiMesh* meshHandle)
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

		auto LoadTextureByMaterial = [this](LGLStructs::Mesh& mesh, aiMaterial* material, size_t texTypeIndex)
		{
			/*
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
			*/

			aiTextureType convertedType = textureTypeInfo[texTypeIndex].assimpTexType;

			for (size_t i = 0; i < material->GetTextureCount(convertedType); ++i)
			{
				aiString str;
				material->GetTexture(convertedType, i, &str);
				std::string strWithoutPrefix = str.C_Str();

				LGLStructs::Texture newTexture;

				newTexture.name = nameToSet + '_' + strWithoutPrefix;
				newTexture.type = textureTypeInfo[texTypeIndex].lglTexType;

				const aiTexture* embeddedTexture = modelHandle->GetEmbeddedTexture(strWithoutPrefix.c_str());

				if (texturesLoaded.find(newTexture.name) == texturesLoaded.end())
				{
					LoadTexture(
						newTexture.name,
						newTexture,
						embeddedTexture ? reinterpret_cast<unsigned char*>(embeddedTexture->pcData) : nullptr,
						embeddedTexture ? embeddedTexture->mWidth : -1
					);
				}
				else
				{
					newTexture = texturesLoaded[newTexture.name];
				}

				if (newTexture.data)
				{
					mesh.textures.push_back(newTexture);
				}
			}
		};

		aiMaterial* material = modelHandle->mMaterials[meshHandle->mMaterialIndex];

		for (size_t i = 0; i < LGLStructs::Texture::GetTextureTypeAmount(); ++i)
		{
			LoadTextureByMaterial(mesh, material, i);
		}
	};

	LGLStructs::Mesh mesh;

	ProcessVerteces(mesh);
	ProcessFaces(mesh);
	ProcessTextures(mesh);

	return mesh;
}

void FileLoader::ProcessNode(const aiNode* nodeHandle, LGLStructs::ModelInfo& model)
{
	for (size_t i = 0; i < nodeHandle->mNumMeshes; ++i)
	{
		aiMesh* meshHandle = modelHandle->mMeshes[nodeHandle->mMeshes[i]];
		model.AddMesh(ProcessMesh(meshHandle));
	}

	for (size_t i = 0; i < nodeHandle->mNumChildren; ++i)
	{
		ProcessNode(nodeHandle->mChildren[i], model);
	}
}

void FileLoader::FreeTextureData()
{
	for (auto& texture : texturesLoaded)
	{
		stbi_image_free(texture.second.data);
		texture.second.data = nullptr;
	}

	texturesLoaded.clear();
}

bool FileLoader::LoadModel(const std::string& file, const std::string& name, LGLStructs::ModelInfo& model)
{
	Assimp::Importer importer;
	modelHandle = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!modelHandle || modelHandle->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelHandle->mRootNode)
	{
		std::cerr << "AssimpHelper error moduleHandle is invalid\n";
		return false;
	}

	nameToSet = name;
	GetTextureFilenames(file);
	ProcessNode(modelHandle->mRootNode, model);

	return true;
}

bool FileLoader::GetScriptFuncFromDLL(
	std::function<void(const std::string&, ISolidSim&)>& scriptFunc,
	const std::string& dllPath,
	const std::string& funcName
)
{
	bool success = false;

	HMODULE dllHandle = LoadLibraryA(dllPath.c_str());

	if (dllHandle)
	{
		if (dllHandleMap.find(dllPath) != dllHandleMap.end())
		{
			if (dllHandleMap[dllPath] != dllHandle)
			{
				FreeLibrary(dllHandleMap[dllPath]);
				dllHandleMap.erase(dllPath);
				dllHandleMap[dllPath] = dllHandle;
			}
		}
		else
		{
			dllHandleMap[dllPath] = dllHandle;
		}

		using ScriptWrapperType = void(*)(const char*, void*);
		ScriptWrapperType scriptWrapperFunc = reinterpret_cast<ScriptWrapperType>(
			GetProcAddress(dllHandleMap[dllPath], funcName.c_str())
		);

		if (scriptWrapperFunc)
		{
			scriptFunc = [scriptWrapperFunc](const std::string& name, ISolidSim& solid) 
			{ 
				scriptWrapperFunc(name.c_str(), reinterpret_cast<void*>(&solid));
			};

			success = true;
		}
	}

	return success;

}

void FileLoader::FreeDllData()
{
	for (auto& dllHandle : dllHandleMap)
	{
		if (dllHandle.second)
		{
			FreeLibrary(dllHandle.second);
		}
	}

	dllHandleMap.clear();
}