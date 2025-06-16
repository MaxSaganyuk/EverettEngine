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

struct GLFWwindow;
class LGLUniformHasher;

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

	struct VAOInfo;
	struct InternalModelInfo;
	using InternalModelMap = std::map<std::string, InternalModelInfo>;

	// Structs for internal use
	struct VAOInfo
	{
		VAO vboId;
		size_t pointAmount;
		bool useIndices;
		LGLStructs::MeshInfo* meshInfo;

		VAOInfo()
		{
			vboId = 0;
			pointAmount = 0;
			useIndices = false;
			meshInfo = nullptr;
		}
	};

	struct InternalModelInfo
	{
		LGLStructs::ModelInfo* modelPtr = nullptr;
		std::vector<VAOInfo> VAOs;
		std::map<std::string, TextureID> textureIDs;
	};

	struct ShaderInfo
	{
		Shader shaderId;
		ShaderCode shaderCode;
	};

	struct InteractableInfo
	{
		bool pressed = false;
		bool holdable = false;
		std::function<void()> pressedFunc;
		std::function<void()> releasedFunc;
	};

	class LGLEnumInterpreter
	{
	public:
		static const std::vector<int> DepthTestModeInter;
		static const std::vector<int> TextureOverlayTypeInter;
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

	// Public functions
	LGL_API LGL();
	LGL_API ~LGL();

	LGL_API bool CreateWindow(const int height, const int width, const std::string& title);
	
	// Additional steps to rendering can be passed as a function pointer or a lambda
	// It is expected to get a lambda with a script for camera behaviour
	LGL_API void RunRenderingCycle(std::function<void()> additionalSteps = nullptr);
	LGL_API void PauseRendering(bool value = true);
	LGL_API void SetStaticBackgroundColor(const glm::vec4& rgba);
	LGL_API void EnableVSync(bool value = true);

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
	LGL_API void CreateMesh(const std::string& modelName, LGLStructs::MeshInfo& meshInfo);
	LGL_API void CreateModel(const std::string& modelName, LGLStructs::ModelInfo& model);

	LGL_API void DeleteModel(const std::string& modelName);
#endif
	LGL_API bool ConfigureTexture(const std::string& modelName, const LGLStructs::Texture& texture);

	LGL_API static void InitOpenGL(int major, int minor);

	LGL_API static void TerminateOpenGL();

	LGL_API void SetDepthTest(DepthTestMode depthTestMode);

	LGL_API int GetMaxAmountOfVertexAttr();

	LGL_API void CaptureMouse(bool value);

	LGL_API void SetInteractable(
		int keyID, 
		bool holdable,
		const std::function<void()>& pressedFunc, 
		const std::function<void()>& releasedFunc = nullptr
	);
	
	LGL_API static std::string ConvertKeyTo(int keyId);
	LGL_API static int         ConvertKeyTo(const std::string& keyName);

	LGL_API void SetAssetOnOpenGLFailure(bool value);

	LGL_API void SetShaderFolder(const std::string& path);
	LGL_API void RecompileShader(const std::string& shaderName);

	LGL_API void ResetLGL();

	//Callback setters - Pass nothing to make callback self-contained
	LGL_API void SetCursorPositionCallback(std::function<void(double, double)> callbackFunc);
	LGL_API void SetScrollCallback(std::function<void(double, double)> callbackFunc);
	LGL_API void SetKeyPressCallback(std::function<void(int, int, int, int)> callbackFunc);
	LGL_API void SetRenderDeltaCallback(std::function<void(float)> callbackFunc);

	//Function templates
	template<typename Type>
	LGL_API bool SetShaderUniformValue(const std::string& valueName, Type&& value, const std::string& shaderProgramName = "");

	LGL_API void EnableUniformValueBatchSending(bool value = true);
	LGL_API void EnableUniformValueHashing(bool value = true);

private:
	bool InitGLAD();
	void InitCallbacks();

	void DeleteGLObjects();
	void DeleteShader(const std::string& shaderName);

	int CheckUniformValueLocation(
		const std::string& valueName, 
		const std::string& shaderProgramName, 
		ShaderProgram& shaderProgramID
	);

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

	CALLBACK KeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static std::function<void(int, int, int, int)> keyPressCallbackFunc;

	static std::function<void(float)> renderTimeCallbackFunc;

	void ProcessInput();
	void Render();

	float renderDeltaTime;

	bool useVSync; // Passed value is not bool, but will do for on/off switch
	bool pauseRendering;

	GLFWwindow* window;

	glm::vec4 background;

	VAOInfo currentVAOToRender;
	std::vector<VBO> VBOCollection;
	InternalModelMap internalModelMap;
	std::vector<EBO> EBOCollection;

	// Shader
	std::string shaderPath;
	static std::map<std::string, ShaderType> shaderTypeChoice;

	std::map<std::string, std::vector<ShaderInfo>> shaderInfoCollection;
	
	std::string lastProgram;
	std::map<std::string, ShaderProgram> shaderProgramCollection;

	std::map<size_t, InteractableInfo> interactCollection;

	bool batchUniformVals;
	bool hashUniformVals;
	std::vector<std::string> uniformErrorAntispam;
	std::unordered_set<size_t> uniformLocationTracker;
	std::unique_ptr<LGLUniformHasher> uniformHasher;
};

#undef CALLBACK
