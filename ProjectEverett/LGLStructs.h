#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stdEx/utilityEx.h"

#include <functional>

namespace LGLStructs
{

	struct BilinearFiltrationConfig
	{
		bool minFilter = false;
		bool maxFilter = true;
	};

	struct TextureParams
	{
		enum class TextureOverlayType
		{
			REPEAT = GL_REPEAT,
			MIRRORED = GL_MIRRORED_REPEAT,
			EDGECLAMP = GL_CLAMP_TO_EDGE,
			BORDERCLAMP = GL_CLAMP_TO_BORDER
		};

		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		TextureOverlayType overlay = TextureOverlayType::MIRRORED;
		BilinearFiltrationConfig BFConfig = {};
		bool createMipmaps = false;
		BilinearFiltrationConfig mipmapBFConfig = {};
	};

	struct Vertex
	{
		enum class VertexData
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
			VertexData vertData = static_cast<VertexData>(index);

			switch (vertData)
			{
			case VertexData::Position:
				return Position;
			case VertexData::Normal:
				return Normal;
			case VertexData::TexCoords:
				return TexCoords;
			case VertexData::Tangent:
				return Tangent;
			case VertexData::Bitangent:
				return Bitangent;
			default:
				assert(false && "Invaling index in VertexData");
			}
		}

		constexpr static size_t GetMemberAmount()
		{
			constexpr size_t memberAmount = static_cast<size_t>(VertexData::_SIZE);
			constexpr size_t memberAmountDoubleCheck = sizeof(Vertex) / sizeof(glm::vec3);
			static_assert(memberAmount == memberAmount, "Member amount and enum mismatch");

			return memberAmount;
		}
	};

	struct Texture
	{
		enum class TextureType
		{
			Diffuse,
			Specular,
			Normal,
			Height,
			_SIZE
		};

		TextureType type;
		std::string name;

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
		stdEx::ValWithBackup<bool> isDynamic;
		stdEx::ValWithBackup<std::string> shaderProgram;
		stdEx::ValWithBackup<std::function<void()>> behaviour;

		MeshInfo(const Mesh& mesh, bool& isDynamic, std::string& shaderProgram, std::function<void()>& behaviour)
			: mesh(mesh), isDynamic(isDynamic), shaderProgram(shaderProgram), behaviour(behaviour) {}
	
	};
	
	struct ModelInfo
	{
		std::vector<MeshInfo> meshes;
		bool isDynamic = false;
		std::string shaderProgram = "0";
		std::function<void()> behaviour = nullptr;

		void AddMesh(const Mesh& mesh)
		{
			meshes.emplace_back(MeshInfo(mesh, isDynamic, shaderProgram, behaviour));
		}

		void ResetDefaults()
		{
			for (auto& mesh : meshes)
			{
				mesh.isDynamic.ResetBackup(isDynamic);
				mesh.shaderProgram.ResetBackup(shaderProgram);
				mesh.behaviour.ResetBackup(behaviour);
			}
		}

		ModelInfo& operator=(ModelInfo& modelInfo)
		{
			meshes = modelInfo.meshes;
			isDynamic = modelInfo.isDynamic;
			shaderProgram = modelInfo.shaderProgram;
			behaviour = modelInfo.behaviour;

			ResetDefaults();

			return *this;
		}
	};

}