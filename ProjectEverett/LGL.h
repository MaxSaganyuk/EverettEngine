#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <thread>
#include <typeindex>
#include <glad/glad.h>
#include <glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define CALLBACK static void

/*
	Lambda (Open) GL
*/
class LGL
{
// Alias section
private:
	using VBO = unsigned int; // Vertex Buffer Object
	using VAO = unsigned int; // Vertex Array Object
	using EBO = unsigned int; // Element Buffer Object

	using Shader = unsigned int;
	using ShaderCode = std::string;
	using ShaderType = int;
	using ShaderProgram = unsigned int;

	using Texture = unsigned int;
	using TextureData = unsigned char*;

// Structs for internal use
	struct VAOInfo
	{
		VAO vboId = 0;
		size_t pointAmount;
		bool useIndices;
		std::string shader;
		std::vector<std::string> textures;
		std::function<void()> behaviour = nullptr;
	};

	struct ShaderInfo
	{
		Shader shaderId;
		ShaderCode shaderCode;
		bool compiled = true;
	};

	struct TextureInfo
	{
		Texture textureId;
		TextureData textureData;
		GLenum textureFormat;
		int width;
		int height;
		int nrChannels;
	};

	std::vector<int> steps{ 3, 2, 3 };

public:
	// Enums and structs for external use

	enum class TextureOverlayType
	{
		REPEAT      = GL_REPEAT,
		MIRRORED    = GL_MIRRORED_REPEAT,
		EDGECLAMP   = GL_CLAMP_TO_EDGE,
		BORDERCLAMP = GL_CLAMP_TO_BORDER
	};

	struct BilinearFiltrationConfig
	{
		bool minFilter = false;
		bool maxFilter = true;
	};

	struct TextureParams
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		TextureOverlayType overlay = TextureOverlayType::MIRRORED;
		BilinearFiltrationConfig BFConfig = {};
		bool createMipmaps = false;
		BilinearFiltrationConfig mipmapBFConfig = {};
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;
	};

	struct MeshInfo
	{
		std::vector<Vertex> vert;
		std::vector<unsigned int> indices;
		bool isDynamic = false;
		std::string shaderProgram = "";
		std::vector<std::string> textures;
		std::function<void()> behaviour = nullptr;
	};

	struct MeshInfoFromFile
	{
		std::string file;
		bool isDynamic = false;
		std::string shader = "";
		std::function<void()> behaviour = nullptr;
	};

	LGL(int major, int minor);
	~LGL();

	bool CreateWindow(const int height, const int width, const std::string& title);
	
	// Additional steps to rendering can be passed as a function pointer or a lambda
	// It is expected to get a lambda with a script for camera behaviour
	void RunRenderingCycle(std::function<void()> additionalSteps = nullptr);
	void SetStaticBackgroundColor(const glm::vec4& rgba);

	// Creates a VAO, VBO and (if indices are given) EBO
	// Must accept amount of steps for
	// You can pass a lambda to describe general behaviour for your shape
	// Behaviour function will be called inside the rendering cycle
	// exaclty at it's VAO binding.
	// If you need several shapes with similar behaviour, it's possible 
	// to render additional ones with SetShaderUniformValue function inside
	// the lambda script without creating additional VAOs
	void CreateMesh(const MeshInfo& meshInfo);
	void GetMeshFromFile(const std::string& file, std::vector<Vertex>& vertexes, std::vector<unsigned int>& indeces);

	// If no name is given will compile last loaded shader
	bool CompileShader(const std::string& name = "");
	bool LoadShader(const std::string& code, const std::string& type, const std::string& name);
	bool LoadShaderFromFile(const std::string& file, const std::string& shaderName = "");

	// If no list of shaders is provided, will create a program with all compiled shaders
	bool CreateShaderProgram(const std::string& name, const std::vector<std::string>& shaderVector = {});

	bool LoadTextureFromFile(const std::string& file, const std::string& textureName = "");
	bool ConfigureTexture(const std::string& textureName, const TextureParams& textureParams = {});
	bool ConfigureTexture(const TextureParams& textureParams = {});

	bool InitGLAD();
	void InitCallbacks();
	int GetMaxAmountOfVertexAttr();

	void CaptureMouse();

	template<class Type>
	bool SetShaderUniformValue(const std::string& valueName, Type&& value, bool render = false, const std::string& shaderProgramName = "");

	void SetInteractable(int key, std::function<void()> func);

	void SetCursorPositionCallback(std::function<void(double, double)> callbackFunc);
	void SetScrollCallback(std::function<void(double, double)> callbackFunc);

private:
	// Callbacks
	CALLBACK FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	
	CALLBACK CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static std::function<void(double, double)> cursorPositionFunc;
	
	CALLBACK ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static std::function<void(double, double)> scrollCallbackFunc;

	void ProcessInput();
	bool HandleGLError(const unsigned int& glId, int statusToCheck);
	void Render();

 	GLFWwindow* window;
	glm::vec4 background;

	VAOInfo currentVAOToRender;
	std::vector<VBO> VBOCollection;
	std::vector<VAOInfo> VAOCollection;
	std::vector<EBO> EBOCollection;

	// Shader
	std::map<std::string, ShaderType> shaderTypeChoice
	{
		{"vert", GL_VERTEX_SHADER},
		{"frag", GL_FRAGMENT_SHADER}
	};

	std::string lastShader;
	std::map<std::string, ShaderInfo> shaderInfoCollection;
	
	// Shader Program
	std::string lastProgram;
	std::map<std::string, ShaderProgram> shaderProgramCollection;

	// Texture
	std::string lastTexture;
	std::map<std::string, TextureInfo> textureCollection;

	std::vector<std::string> uniformErrorAntispam;

	// 
	std::map<int, std::function<void()>> interactCollection;

	std::unordered_map<std::type_index, std::function<void(int, void*)>> uniformValueLocators
	{
		{ typeid(int),   [](int uniformValueLocation, void* value) { glUniform1i(uniformValueLocation, *reinterpret_cast<int*>(value)); }},
		{ typeid(float), [](int uniformValueLocation, void* value) { glUniform1f(uniformValueLocation, *reinterpret_cast<float*>(value)); }},

		{ typeid(glm::vec3), [](int uniformValueLocation, void* value) 
		{ 
			glm::vec3* coords = reinterpret_cast<glm::vec3*>(value);
			glUniform3f(uniformValueLocation, coords->x, coords->y, coords->z);
		}},

		{ typeid(glm::vec4), [](int uniformValueLocation, void* value)
		{
			glm::vec4* coords = reinterpret_cast<glm::vec4*>(value);
			glUniform4f(uniformValueLocation, coords->x, coords->y, coords->z, coords->w);
		}},

		{ typeid(glm::mat4), [](int uniformValueLocation, void* value)
		{  
			glUniformMatrix4fv(uniformValueLocation, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<glm::mat4*>(value)));
		}}

	};
};

template<class Type>
bool LGL::SetShaderUniformValue(const std::string& valueName, Type&& value, bool render, const std::string& shaderProgramName)
{
	ShaderProgram shaderProgramToUse = shaderProgramCollection[shaderProgramName == "" ? lastProgram : shaderProgramName];

	int uniformValueLocation = glGetUniformLocation(shaderProgramToUse, valueName.c_str());

	if (uniformValueLocation == -1)
	{
		if (std::find(uniformErrorAntispam.begin(), uniformErrorAntispam.end(), valueName) == std::end(uniformErrorAntispam))
		{
			std::cout << "[ERROR] Shader value " + valueName << " could not be located\n";
			uniformErrorAntispam.push_back(valueName);
		}
		return false;
	}

	uniformValueLocators.at(typeid(Type))(uniformValueLocation, &value);

	if (render)
	{
		Render();
	}

	return true;
}