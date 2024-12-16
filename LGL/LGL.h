#pragma once

#ifdef LGL_EXPORT
#define LGL_API __declspec(dllexport)
#else
#define LGL_API __declspec(dllimport)
#endif

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <typeindex>
#include <unordered_set>

#include "LGLStructs.h"

#define CALLBACK static void

class GLFWwindow;

/*
	Lambda (Open) GL

	Todo:
	Add frame limiter (fix frame dependent camera movement speed)
	Maybe improve SetShaderUniformValue for arrays
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

	using TextureID = unsigned int;
	using TextureData = unsigned char*;

	using OnPressFunction = std::function<void()>;
	using OnReleaseFunction = std::function<void()>;

	// Structs for internal use
	struct VAOInfo
	{
		VAO vboId = 0;
		size_t pointAmount;
		bool useIndices;
		LGLStructs::MeshInfo* meshInfo = nullptr;
	};

	struct ShaderInfo
	{
		Shader shaderId;
		ShaderCode shaderCode;
	};

	class LGLEnumInterpreter
	{
	public:
		static const std::vector<int> DepthTestModeInter;
		static const std::vector<int> TextureOverlayTypeInter;
		static const std::vector<int> SpecialKeyInter;
	};

public:

	// Enums
	enum class DepthTestMode
	{
		Disable,
		Always,
		Never,
		Less,
		Greater,
		Equal,
		NotEqual,
		LessOrEqual,
		GreaterOrEqual
	};

	enum class SpecialKeys
	{
		Enter,
		Space,
		LeftShift,
		RightShift,
		Tab,
		Backspace,
		Escape
	};

	// Public functions
	LGL_API LGL();
	LGL_API ~LGL();

	LGL_API bool CreateWindow(const int height, const int width, const std::string& title);
	
	// Additional steps to rendering can be passed as a function pointer or a lambda
	// It is expected to get a lambda with a script for camera behaviour
	LGL_API void RunRenderingCycle(std::function<void()> additionalSteps = nullptr);
	LGL_API void SetStaticBackgroundColor(const glm::vec4& rgba);

	// Creates a VAO, VBO and (if indices are given) EBO
	// Must accept amount of steps for
	// You can pass a lambda to describe general behaviour for your shape
	// Behaviour function will be called inside the rendering cycle
	// exaclty at it's VAO binding.
	// If you need several shapes with similar behaviour, it's possible 
	// to render additional ones with SetShaderUniformValue function inside
	// the lambda script without creating additional VAOs
#ifdef ENABLE_OLD_MODEL_IMPORT
	LGL_API void GetMeshFromFile(const std::string& file, std::vector<LGLStructs::Vertex>& vertexes, std::vector<unsigned int>& indeces);
#else	
	LGL_API void CreateMesh(LGLStructs::MeshInfo& meshInfo);
	LGL_API void CreateModel(LGLStructs::ModelInfo& model);
#endif
	LGL_API bool ConfigureTexture(const LGLStructs::Texture& texture);

	LGL_API static void InitOpenGL(int major, int minor);

	LGL_API static void TerminateOpenGL();

	LGL_API void SetDepthTest(DepthTestMode depthTestMode);

	LGL_API int GetMaxAmountOfVertexAttr();

	LGL_API void CaptureMouse();

	LGL_API void SetInteractable(unsigned char key, const OnPressFunction& preFunc, const OnReleaseFunction& relFunc = nullptr);
	LGL_API void SetInteractable(SpecialKeys key, const OnPressFunction& preFunc, const OnReleaseFunction& relFunc = nullptr);

	LGL_API void SetAssetOnOpenGLFailure(bool value);

	LGL_API void SetShaderFolder(const std::string& path);

	//Callback setters
	LGL_API void SetCursorPositionCallback(std::function<void(double, double)> callbackFunc);
	LGL_API void SetScrollCallback(std::function<void(double, double)> callbackFunc);

	//Function templates
	template<typename Type>
	LGL_API bool SetShaderUniformValue(const std::string& valueName, Type&& value, const std::string& shaderProgramName = "");

private:
	bool InitGLAD();
	void InitCallbacks();

	int CheckUniformValueLocation(const std::string& valueName, const std::string& shaderProgramName);

	// If no name is given will compile last loaded shader
	bool CompileShader(const std::string& name = "");
	bool LoadShaderFromFile(const std::string& name, const std::string& file, const std::string& shaderType);

	// If no list of shaders is provided, will create a program with all compiled shaders
	bool CreateShaderProgram(const std::string& name, const std::vector<std::string>& shaderVector = {});

	// If shader file names can be identical to shader program name, general load and compile can be used
	bool LoadAndCompileShader(const std::string& name);



	// Callbacks
	CALLBACK GLFWErrorCallback(int errorCode, const char* description);

	CALLBACK FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	
	CALLBACK CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static std::function<void(double, double)> cursorPositionFunc;
	
	CALLBACK ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static std::function<void(double, double)> scrollCallbackFunc;

	void ProcessInput();
	void Render();

	GLFWwindow* window;

	glm::vec4 background;

	VAOInfo currentVAOToRender;
	std::vector<VBO> VBOCollection;
	std::vector<VAOInfo> VAOCollection;
	std::vector<EBO> EBOCollection;

	// Shader
	std::string shaderPath;
	static std::map<std::string, ShaderType> shaderTypeChoice;

	std::map<std::string, std::vector<ShaderInfo>> shaderInfoCollection;
	
	std::string lastProgram;
	std::map<std::string, ShaderProgram> shaderProgramCollection;

	// Texture
	std::map<std::string, TextureID> textureCollection;

	std::vector<std::string> uniformErrorAntispam;

	std::map<size_t, std::pair<OnPressFunction, OnReleaseFunction>> interactCollection;

	std::unordered_set<size_t> uniformLocationTracker;
};

#undef CALLBACK
