#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
			Bitangent
		};

		constexpr static size_t DataAmount = 5;
		
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
	};

	struct MeshInfo
	{
		std::vector<Vertex> vert;
		std::vector<unsigned int> indices;
	};
	
	struct ModelInfo
	{
		std::vector<MeshInfo> meshes;
		bool isDynamic = false;
		std::string shaderProgram = "";
		std::vector<std::string> textures;
		std::function<void()> behaviour = nullptr;
	};

}