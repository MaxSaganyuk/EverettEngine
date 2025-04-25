#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/quaternion.hpp"

#include "stdEx/utilityEx.h"

#include <functional>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>

namespace LGLStructs
{
	struct BasicVertex
	{
		enum class BasicVertexData
		{
			Position,
			Normal,
			TexCoords,
			Tangent,
			Bitangent,
			_SIZE
		};
		
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		glm::vec3& operator[](const size_t index)
		{
			BasicVertexData vertData = static_cast<BasicVertexData>(index);

			switch (vertData)
			{
			case BasicVertexData::Position:
				return Position;
			case BasicVertexData::Normal:
				return Normal;
			case BasicVertexData::TexCoords:
				return TexCoords;
			case BasicVertexData::Tangent:
				return Tangent;
			case BasicVertexData::Bitangent:
				return Bitangent;
			default:
				assert(false && "Invaling index in VertexData");
			}
		}

		constexpr static size_t GetLocalMemberAmount()
		{
			constexpr size_t memberAmount = static_cast<size_t>(BasicVertexData::_SIZE);
			constexpr size_t memberAmountDoubleCheck = sizeof(BasicVertex) / sizeof(glm::vec3);
			static_assert(memberAmount == memberAmountDoubleCheck, "Member amount and enum mismatch");

			return memberAmount;
		}

		constexpr static size_t GetMemberElementSize()
		{
			return glm::vec3::length();
		}
	};


	struct Vertex : BasicVertex
	{
		constexpr static size_t maxWeightPerVertex = 4;

		enum class VertexData
		{
			BoneID,
			Weights,
			_SIZE
		};

		std::array<int, maxWeightPerVertex> boneIDs{};
		std::array<float, maxWeightPerVertex>  boneWeights{};

		void AddBoneData(unsigned int boneID, float boneWeight)
		{
			for (size_t i = 0; i < maxWeightPerVertex; ++i)
			{
				if (boneWeights[i] == 0.0f)
				{
					boneIDs[i] = boneID;
					boneWeights[i] = boneWeight;

					return;
				}
			}

			//assert(false && "Attempt to add more than 4 weights to vertex");
		}

		void NormalizeIfEmptyWeights()
		{
			float sum = 0.0f;
			
			for (auto weight : boneWeights)
			{
				if (weight > 0.0f)
				{
					return;
				}
			}

			boneWeights[0] = 1.0f;
		}

		constexpr static size_t GetLocalMemberAmount()
		{
			// Reconfirmation of equivalency of float and int raw sizes
			static_assert(sizeof(float) == sizeof(int), "Unexpected diffs in float and uint sizes");

			constexpr size_t memberAmount = static_cast<size_t>(VertexData::_SIZE);
			constexpr size_t memberAmountDoubleCheck =
				(sizeof(Vertex) - sizeof(BasicVertex)) / sizeof(std::array<float, maxWeightPerVertex>);
			static_assert(memberAmount == memberAmountDoubleCheck, "Member amount and enum mismatch");

			return memberAmount;
		}

		constexpr static size_t GetMemberElementSize()
		{
			return maxWeightPerVertex;
		}
	};

	struct Texture
	{
		using TextureData = unsigned char*;

		enum class TextureType
		{
			Diffuse,
			Specular,
			Normal,
			Height,
			_SIZE
		};

		struct TextureParams
		{
			enum class TextureOverlayType
			{
				Repeat,
				Mirrored,
				EdgeClamp,
				BorderClamp
			};

			struct BilinearFiltrationConfig
			{
				bool minFilter = false;
				bool maxFilter = true;
			};

			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			TextureOverlayType overlay = TextureOverlayType::Repeat;
			BilinearFiltrationConfig BFConfig = {};
			bool createMipmaps = false;
			BilinearFiltrationConfig mipmapBFConfig = {};
		};

		std::string name;
		TextureType type = TextureType::Diffuse;
		TextureData data = nullptr;
		TextureParams params;
		int width;
		int height;
		int channelAmount;

		constexpr static size_t GetTextureTypeAmount()
		{
			constexpr size_t typeAmount = static_cast<size_t>(TextureType::_SIZE);

			return typeAmount;
		}
	};

	struct Mesh
	{
		std::vector<Vertex> vert;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
	};

	struct MeshInfo
	{
		Mesh mesh;
		std::string meshName;
		stdEx::ValWithBackup<bool> isDynamic;
		stdEx::ValWithBackup<bool> render;
		stdEx::ValWithBackup<std::string> shaderProgram;
		stdEx::ValWithBackup<std::function<void(int)>> behaviour;

		MeshInfo(
			const Mesh& mesh, 
			const std::string& meshName, 
			bool& render, 
			bool& isDynamic, 
			std::string& shaderProgram, 
			std::function<void(int)>& behaviour
		)
		: 
			mesh(mesh), 
			meshName(meshName), 
			render(&render), 
			isDynamic(&isDynamic), 
			shaderProgram(&shaderProgram), 
			behaviour(&behaviour) 
		{}

	};
	
	struct ModelInfo
	{
		std::vector<MeshInfo> meshes;
		bool render;
		bool isDynamic;
		std::string shaderProgram;
		std::function<void()> modelBehaviour;
 		std::function<void(int)> generalMeshBehaviour;

		bool isTextureless = true;

		ModelInfo()
		{
			render = true;
			isDynamic = false;
			shaderProgram = "0";
			modelBehaviour = nullptr;
			generalMeshBehaviour = nullptr;
		}

		void AddMesh(const Mesh& mesh, const std::string meshName)
		{
			meshes.emplace_back(MeshInfo(mesh, meshName, render, isDynamic, shaderProgram, generalMeshBehaviour));
		}

		std::vector<std::string> GetMeshNames()
		{
			std::vector<std::string> meshNames;
			meshNames.reserve(meshes.size());

			for (auto& mesh : meshes)
			{
				meshNames.push_back(mesh.meshName);
			}

			return meshNames;
		}

		void ResetDefaults()
		{
			for (auto& mesh : meshes)
			{
				mesh.render.ResetBackup(&render);
				mesh.isDynamic.ResetBackup(&isDynamic);
				mesh.shaderProgram.ResetBackup(&shaderProgram);
				mesh.behaviour.ResetBackup(&generalMeshBehaviour);
			}
		}

		void NormalizeAllEmptyWeights()
		{
			for (auto& mesh : meshes)
			{
				for (auto& vert : mesh.mesh.vert)
				{
					vert.NormalizeIfEmptyWeights();
				}
			}
		}

		void RecheckIfTextureless()
		{
			for (auto& mesh : meshes)
			{
				if (!mesh.mesh.textures.empty())
				{
					isTextureless = false;
					break;
				}
			}
		}

		ModelInfo& operator=(const ModelInfo& modelInfo)
		{
			meshes = modelInfo.meshes;
			render = modelInfo.render;
			isDynamic = modelInfo.isDynamic;
			shaderProgram = modelInfo.shaderProgram;
			modelBehaviour = modelInfo.modelBehaviour;
			generalMeshBehaviour = modelInfo.generalMeshBehaviour;
			isTextureless = modelInfo.isTextureless;

			ResetDefaults();

			return *this;
		}
	};
}