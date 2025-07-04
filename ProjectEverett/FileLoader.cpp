#include <iostream>
#include <map>
#include <functional>
#include <Windows.h>

#ifdef _HAS_CXX17
#include <filesystem>
#endif

#include <assimp/Importer.hpp>
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"

#include "FileLoader.h"

#include "stb_image.h"

#include "SolidSim.h"

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "EverettException.h"

void ConvertFromAssimpToGLM(const aiMatrix4x4& assimpMatrix, glm::mat4& glmMatrix)
{
	glmMatrix = {
		assimpMatrix.a1, assimpMatrix.b1, assimpMatrix.c1, assimpMatrix.d1,
		assimpMatrix.a2, assimpMatrix.b2, assimpMatrix.c2, assimpMatrix.d2,
		assimpMatrix.a3, assimpMatrix.b3, assimpMatrix.c3, assimpMatrix.d3,
		assimpMatrix.a4, assimpMatrix.b4, assimpMatrix.c4, assimpMatrix.d4
	};
}

void ConvertFromAssimpToGLM(const aiVector3D& assimpVect, glm::vec3& glmVect)
{
	for (size_t axis = 0; axis < 3; ++axis)
	{
		glmVect[axis] = assimpVect[static_cast<unsigned int>(axis)];
	}
}

void ConvertFromAssimpToGLM(const aiQuaternion& assimpQuat, glm::quat& glmQuat)
{
	glmQuat.x = assimpQuat.x;
	glmQuat.y = assimpQuat.y;
	glmQuat.z = assimpQuat.z;
	glmQuat.w = assimpQuat.w;
}

std::string FileLoader::GetCurrentDir()
{
#ifdef _HAS_CXX17
	return std::filesystem::current_path().string();
#else
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	return path;
#endif
}

bool FileLoader::GetFilesInDir(std::vector<std::string>& files, const std::string& dir)
{
#ifdef _HAS_CXX17
	for (auto& entry : std::filesystem::directory_iterator(dir))
	{
		if (entry.is_regular_file())
		{
			files.push_back(entry.path().filename().string());
		}
	}
#else
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
#endif

	return true;
}

bool FileLoader::ModelLoader::LoadTexture(
	const std::string& file, 
	LGLStructs::Texture& texture, 
	unsigned char* data, 
	size_t dataSize
)
{
	stbi_set_flip_vertically_on_load(data == nullptr);

	texture.data = data ? 
		stbi_load_from_memory(data, static_cast<int>(dataSize), &texture.width, &texture.height, &texture.channelAmount, 0) :
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
	modelLoader.FreeTextureData();
	dllLoader.FreeDllData();
}

bool FileLoader::ModelLoader::GetTextureFilenames(const std::string& path)
{
	auto GetTexturePath = [](const std::string& path)
	{
		return path.substr(0, path.rfind('\\') + 1) + "textures";
	};

	return GetFilesInDir(extraTextureName, GetTexturePath(path));
}

LGLStructs::Mesh FileLoader::ModelLoader::ProcessMesh(
	const aiMesh* meshHandle, 
	BoneMap& boneMap
)
{
	auto GetVertexParam = [&meshHandle](LGLStructs::Vertex::BasicVertexData whichData)
	{
		using BasicVertexData = LGLStructs::Vertex::BasicVertexData;

		aiVector3D* tempVector = nullptr;

		switch (whichData)
		{
		case BasicVertexData::Position:
			tempVector = meshHandle->mVertices;
			break;
		case BasicVertexData::Normal:
			tempVector = meshHandle->mNormals;
			break;
		case BasicVertexData::TexCoords:
			tempVector = meshHandle->mTextureCoords[0];
			break;
		case BasicVertexData::Tangent:
			tempVector = meshHandle->mTangents;
			break;
		case BasicVertexData::Bitangent:
			tempVector = meshHandle->mBitangents;
			break;
		default:
			ThrowExceptionWMessage("Vertex param does not exist");
		}

		return tempVector;
	};

	auto ProcessVerteces = [&meshHandle, &GetVertexParam](LGLStructs::Mesh& mesh)
	{
		for (size_t i = 0; i < meshHandle->mNumVertices; ++i)
		{
			LGLStructs::Vertex vert;
			for (size_t vertData = 0; vertData < 5; ++vertData)
			{
				glm::vec3 glmVect{ 0.0f, 0.0f, 0.0f };
				aiVector3D* assimpVect = GetVertexParam(static_cast<LGLStructs::Vertex::BasicVertexData>(vertData));
	
				if (assimpVect)
				{
					ConvertFromAssimpToGLM(*(assimpVect + i), glmVect);
				}
				
				vert[vertData] = glmVect;
			}

			mesh.vert.push_back(vert);
		}
	};

	auto ProcessBones = [this, &meshHandle](LGLStructs::Mesh& mesh, BoneMap& boneMap)
	{
		for (size_t i = 0; i < meshHandle->mNumBones; ++i)
		{
			aiBone* bone = meshHandle->mBones[i];

			std::string boneName = meshHandle->mBones[i]->mName.C_Str();

			if (boneMap.find(boneName) == boneMap.end())
			{
				boneMap[boneName].id = static_cast<int>(boneMap.size());
			}

			bool bonesSet = true;
			for (size_t j = 0; j < bone->mNumWeights; ++j)
			{
				aiVertexWeight& vertWeight = bone->mWeights[j];

				if (vertWeight.mWeight == 0.0f)
				{
					bonesSet = false;
					break;
				}
				
				mesh.vert[vertWeight.mVertexId].AddBoneData(boneMap[boneName].id, vertWeight.mWeight);
			}

			ConvertFromAssimpToGLM(bone->mOffsetMatrix, boneMap[boneName].offsetMatrix);
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
				material->GetTexture(convertedType, static_cast<unsigned int>(i), &str);
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
	ProcessBones(mesh, boneMap);

	return mesh;
}

void FileLoader::ModelLoader::ProcessNodeForModelInfo(
	const aiNode* nodeHandle,
	LGLStructs::ModelInfo& model,
	BoneMap& boneMap
)
{
	for (size_t i = 0; i < nodeHandle->mNumMeshes; ++i)
	{
		aiMesh* meshHandle = modelHandle->mMeshes[nodeHandle->mMeshes[i]];
		model.AddMesh(ProcessMesh(meshHandle, boneMap), meshHandle->mName.C_Str());
	}

	for (size_t i = 0; i < nodeHandle->mNumChildren; ++i)
	{
		ProcessNodeForModelInfo(nodeHandle->mChildren[i], model, boneMap);
	}
}

void FileLoader::ModelLoader::ProcessNodeForBoneTree(
	const std::string& rootNodeName,
	const aiNode* nodeHandle,
	BoneMap& boneMap,
	AnimSystem::BoneTree::TreeManagerNode* parentTreeNode,
	glm::mat4& globalTransform
)
{
	std::string nodeName = nodeHandle->mName.C_Str();

	AnimSystem::BoneTree::TreeManagerNode* currentNode = parentTreeNode;
	AnimSystem::BoneInfo* currentBoneInfo = &parentTreeNode->GetValue();

	if (nodeName != rootNodeName)
	{
		parentTreeNode->AddNode(nodeName, {});
		currentNode = parentTreeNode->FindNodeBy(nodeName);
		currentBoneInfo = &currentNode->GetValue();
		ConvertFromAssimpToGLM(nodeHandle->mTransformation, currentBoneInfo->localTransform);
		globalTransform = currentBoneInfo->globalTransform = parentTreeNode->GetValue().globalTransform * currentBoneInfo->localTransform;
	}
	else
	{
		ConvertFromAssimpToGLM(nodeHandle->mTransformation, currentBoneInfo->localTransform);
		globalTransform = currentBoneInfo->globalTransform = currentBoneInfo->localTransform;
	}

	if (boneMap.find(nodeName) != boneMap.end())
	{
		currentBoneInfo->id = boneMap[nodeName].id;
		currentBoneInfo->offsetMatrix = boneMap[nodeName].offsetMatrix;
	}

	for (size_t i = 0; i < nodeHandle->mNumChildren; ++i)
	{
		ProcessNodeForBoneTree(rootNodeName, nodeHandle->mChildren[i], boneMap, currentNode, globalTransform);
	}
}

template<typename AssimpType, typename GLMCont>
void FileLoader::ModelLoader::ParseAnimInfo(AssimpType* keys, size_t keyAmount, GLMCont& glmCont)
{
	if (keys && keyAmount)
	{
		bool validKeys = false;

		glmCont.reserve(keyAmount);
	
		for (size_t animInfoIndex = 0; animInfoIndex < keyAmount; ++animInfoIndex)
		{
			glmCont.push_back({});
			glmCont.back().first = (keys + animInfoIndex)->mTime;

			if (glmCont.back().first != 0.0f)
			{
				validKeys = true;
			}

			ConvertFromAssimpToGLM((keys + animInfoIndex)->mValue, glmCont.back().second);
		}

		CheckAndThrowExceptionWMessage(validKeys, "Invalid keys for animation, check if animation is baked");
	}
}

void FileLoader::ModelLoader::LoadAnimations(
	AnimSystem::AnimKeyMap& animKeyMap,
	AnimSystem::AnimInfoVect& animInfoVect
)
{
	size_t animAmount = modelHandle->mNumAnimations;

	for (size_t animIndex = 0; animIndex < animAmount; ++animIndex)
	{
		const aiAnimation* animHandle = modelHandle->mAnimations[animIndex];

		if (animHandle)
		{
			animInfoVect.push_back({ animHandle->mName.C_Str(), animHandle->mDuration , animHandle->mTicksPerSecond });

			for (size_t channelIndex = 0; channelIndex < animHandle->mNumChannels; ++channelIndex)
			{
				const aiNodeAnim* animNode = animHandle->mChannels[channelIndex];

				AnimSystem::AnimKeys currentAnimInfo;

				ParseAnimInfo(animNode->mPositionKeys, animNode->mNumPositionKeys, currentAnimInfo.positionKeys);
				ParseAnimInfo(animNode->mRotationKeys, animNode->mNumRotationKeys, currentAnimInfo.rotationKeys);
				ParseAnimInfo(animNode->mScalingKeys,  animNode->mNumScalingKeys,  currentAnimInfo.scalingKeys );

				std::string nodeName = animNode->mNodeName.C_Str();
				if (animKeyMap.find(nodeName) == animKeyMap.end())
				{
					animKeyMap[nodeName].resize(animAmount);
				}

				animKeyMap[nodeName][animIndex] = currentAnimInfo;
			}
		}
	}
}


void FileLoader::ModelLoader::FreeTextureData()
{
	for (auto& [textureName, texture] : texturesLoaded)
	{
		stbi_image_free(texture.data);
		texture.data = nullptr;
	}

	texturesLoaded.clear();
}

bool FileLoader::ModelLoader::LoadModel(
	const std::string& file,
	const std::string& name,
	LGLStructs::ModelInfo& model,
	AnimSystem::ModelAnim& modelAnim
)
{
	Assimp::Importer importer;
	modelHandle = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights);

	if (!modelHandle || modelHandle->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelHandle->mRootNode)
	{
		std::cerr << "AssimpHelper error moduleHandle is invalid\n";
		return false;
	}

	nameToSet = name;
	GetTextureFilenames(file);

	BoneMap boneMap;

	ProcessNodeForModelInfo(modelHandle->mRootNode, model, boneMap);
	model.RecheckIfTextureless();
	model.NormalizeAllEmptyWeights();

	modelAnim.boneAmount = boneMap.size();

	glm::mat4 globalTransform = glm::mat4(1.0f);
	std::string rootNodeName = modelHandle->mRootNode->mName.C_Str();
	modelAnim.boneTree.AddRootNode(rootNodeName, {});
	ProcessNodeForBoneTree(rootNodeName, modelHandle->mRootNode, boneMap, modelAnim.boneTree.FindNodeBy(rootNodeName), globalTransform);
	modelAnim.globalInverseTransform = glm::inverse(modelAnim.boneTree.FindNodeBy(rootNodeName)->GetValue().globalTransform);

	LoadAnimations(modelAnim.animKeyMap, modelAnim.animInfoVect);
	
	return true;
}

bool FileLoader::DLLLoader::GetScriptFuncFromDLL(
	const std::string& dllPath,
	const std::string& funcName,
	std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncWeakPtr
)
{
	bool success = false;

	HMODULE dllHandle = LoadLibraryA(dllPath.c_str());

	if (dllHandle)
	{
		success = true;

		if (dllHandleMap.find(dllPath) != dllHandleMap.end())
		{
			if (dllHandle != dllHandleMap[dllPath].first)
			{
				dllHandleMap[dllPath].first = dllHandle;

				for (auto& [currentFuncName, func] : dllHandleMap[dllPath].second)
				{
					if (funcName != currentFuncName)
					{
						success &= GetScriptFuncFromDLLImpl(dllPath, currentFuncName, scriptFuncWeakPtr);
					}
				}

			}
		}
		else
		{
			dllHandleMap[dllPath].first = dllHandle;
		}

		success &= GetScriptFuncFromDLLImpl(dllPath, funcName, scriptFuncWeakPtr);
	}

	return success;

}

bool FileLoader::DLLLoader::GetScriptFuncFromDLLImpl(
	const std::string& dllPath,
	const std::string& funcName,
	std::weak_ptr<ScriptFuncStorage::InterfaceScriptFunc>& scriptFuncWeakPtr
)
{
	using ScriptWrapperType = void(*)(void*);
	ScriptWrapperType scriptWrapperFunc = reinterpret_cast<ScriptWrapperType>(
		GetProcAddress(dllHandleMap[dllPath].first, funcName.c_str())
	);

	if (scriptWrapperFunc)
	{
		auto scriptFunc = [this, scriptWrapperFunc](IObjectSim* solid)
		{
			std::lock_guard<std::recursive_mutex> lock(scriptWrapperLock);
			scriptWrapperFunc(reinterpret_cast<void*>(solid));
		};

		if (dllHandleMap[dllPath].second.find(funcName) == dllHandleMap[dllPath].second.end())
		{
			dllHandleMap[dllPath].second.emplace(funcName, std::make_shared<ScriptFuncStorage::InterfaceScriptFunc>(scriptFunc));
			scriptFuncWeakPtr = dllHandleMap[dllPath].second[funcName];
		}
		else
		{
			ScriptFuncStorage::InterfaceScriptFunc* currentScriptWrapperFunc = dllHandleMap[dllPath].second[funcName].get();
			*currentScriptWrapperFunc = scriptFunc;
		}

		return true;
	}

	return false;
}

void FileLoader::DLLLoader::UnloadScriptDLL(const std::string& dllPath)
{
	scriptWrapperLock.lock();
	for (auto& [funcName, func] : dllHandleMap[dllPath].second)
	{
		ScriptFuncStorage::InterfaceScriptFunc* scriptFuncWrapper = func.get();
		*scriptFuncWrapper = nullptr;
	}
	scriptWrapperLock.unlock();

	FreeLibrary(dllHandleMap[dllPath].first);
	dllHandleMap[dllPath].first = nullptr;
}

std::vector<std::pair<std::string, std::string>> FileLoader::DLLLoader::GetLoadedScriptDlls()
{
	std::vector<std::pair<std::string, std::string>> loadedDlls;
	loadedDlls.reserve(dllHandleMap.size());

	for (auto& [dllPath, dllInfo] : dllHandleMap)
	{
		loadedDlls.push_back({ dllPath, dllPath.substr(dllPath.rfind('\\') + 1, std::string::npos) });
	}

	return loadedDlls;
}

void FileLoader::DLLLoader::FreeDllData()
{
	for (auto& [dllName, dllInfo] : dllHandleMap)
	{
		auto& [dllHandle, funcMap] = dllInfo;
		if (dllHandle)
		{
			FreeLibrary(dllHandle);
		}
	}

	dllHandleMap.clear();
}

FileLoader::FontLoader::FontLoader()
{
	CheckAndThrowExceptionWMessage(!FT_Init_FreeType(&ft), "Cannot init FreeType lib");
}

FileLoader::FontLoader::~FontLoader()
{
	for (auto& faceInfo : fontToFaceMap)
	{
		FreeFaceInfoByFont(faceInfo.first);
	}
	fontToFaceMap.clear();

	FT_Done_FreeType(ft);
}

std::string FileLoader::FontLoader::LoadFontFromPath(const std::string& fontPath, int fontSize)
{
	std::string fontName = fontPath.substr(fontPath.rfind('\\') + 1, std::string::npos);

	fontToFaceMap[fontName] = {};
	fontToFaceMap[fontName].face = nullptr;

	bool failure = FT_New_Face(ft, fontPath.c_str(), 0, &fontToFaceMap[fontName].face);

	CheckAndThrowExceptionWMessage(!failure, "Failed to load font");

	failure = !failure && FT_Set_Pixel_Sizes(fontToFaceMap[fontName].face, 0, fontSize);

	CheckAndThrowExceptionWMessage(!failure, "Cannot set font size");

	if (failure)
	{
		fontToFaceMap.erase(fontName);
		return "";
	}

	return fontName;
}

LGLStructs::GlyphTexture FileLoader::FontLoader::GetGlyphTextureOf(const std::string& fontName, char c)
{
	if (fontToFaceMap.contains(fontName))
	{
		return GetGlyphTextureOfImpl(fontToFaceMap[fontName], c);
	}

	ThrowExceptionWMessage("Invalid font name");
}

LGLStructs::GlyphInfo& FileLoader::FontLoader::GetAllGlyphTextures(const std::string& fontName)
{
	if (fontToFaceMap.contains(fontName))
	{
		if (!fontToFaceMap[fontName].glyphInfo.glyphs.size())
		{
			fontToFaceMap[fontName].glyphInfo.fontName = fontName;
			for (char c = 32; c < 127; ++c)
			{
				fontToFaceMap[fontName].glyphInfo.glyphs.emplace(c, GetGlyphTextureOfImpl(fontToFaceMap[fontName], c));
			}
		}

		return fontToFaceMap[fontName].glyphInfo;
	}

	ThrowExceptionWMessage("Invalid font name");
}

LGLStructs::GlyphTexture FileLoader::FontLoader::GetGlyphTextureOfImpl(FaceInfo& faceInfo, char c)
{
	if (!FT_Load_Char(faceInfo.face, c, FT_LOAD_RENDER))
	{
		auto glyphToUse = faceInfo.face->glyph;

		if (glyphToUse)
		{
			size_t bufferSize = glyphToUse->bitmap.width * glyphToUse->bitmap.rows;
			faceInfo.glyphData[c].resize(bufferSize);
			std::memcpy(faceInfo.glyphData[c].data(), glyphToUse->bitmap.buffer, bufferSize);

			return {
				c,
				faceInfo.glyphData[c].data(),
				static_cast<int>(glyphToUse->bitmap.width),
				static_cast<int>(glyphToUse->bitmap.rows),
				glyphToUse->bitmap_left,
				glyphToUse->bitmap_top,
				glyphToUse->advance.x
			};
		}
	}

	ThrowExceptionWMessage("Could not load glyph");
}

void FileLoader::FontLoader::FreeFaceInfoByFont(const std::string& fontName, bool keepGlyphData)
{
	if (fontToFaceMap.contains(fontName))
	{
		FT_Done_Face(fontToFaceMap[fontName].face);
		fontToFaceMap[fontName].face = nullptr;

		if (!keepGlyphData)
		{
			fontToFaceMap[fontName].glyphData.clear();
		}

		return;
	}

	ThrowExceptionWMessage("Invalid font name");
}