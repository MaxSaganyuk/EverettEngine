#pragma once

/*
* Despite LGL being C++20 compiled dll, the LGL header must stay C++14 compatible
*/

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
*/
class LGL
{
	// Alias section
private:
	using VBO = unsigned int; // Vertex Buffer Object
	using VAO = unsigned int; // Vertex Array Object
	using EBO = unsigned int; // Element Buffer Object

	using ShaderID = unsigned int;
	using ShaderCode = std::string;
	using ShaderType = int;
	using ShaderProgramID = unsigned int;
	using ShaderName = std::string;

	using TextureID = unsigned int;
	using TextureData = unsigned char*;

	struct VAOInfo;
	class InternalModelInfo;
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

	struct ShaderInfo
	{
		ShaderID shaderId;
		ShaderCode shaderCode;
	};

	struct InteractableInfo
	{
		bool pressed = false;
		bool holdable = false;
		std::function<void()> pressedFunc;
		std::function<void()> releasedFunc;
	};

	struct RenderCharVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
	};

	class InternalModelInfo
	{
		LGLStructs::ModelInfo* modelRawPtr = nullptr;
		std::weak_ptr<LGLStructs::ModelInfo> modelWeakPtr;
		bool isSmartPtrUsed = false;
	public:
		std::vector<VAOInfo> VAOs;
		std::map<std::string, TextureID> textureIDs;

		bool IsSmartPtrUsed();

		void SetModelPtr(LGLStructs::ModelInfo* modelRawPtr);
		void SetModelPtr(std::weak_ptr<LGLStructs::ModelInfo> modelWeakPtr);

		LGLStructs::ModelInfo* GetModelPtr();
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

	LGL_API bool CreateWindow(const int width, const int height, const std::string& title, bool fullscreen = false);

	LGL_API int GetCurrentWindowWidth();
	LGL_API int GetCurrentWindowHeight();
	
	// Additional steps to rendering can be passed as a function pointer or a lambda
	// It is expected to get a lambda with a script for camera behaviour
	LGL_API void RunRenderingCycle(std::function<void()> additionalSteps = nullptr);
	LGL_API void StopRenderingCycle();
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
	LGL_API void CreateModel(const std::string& modelName, std::weak_ptr<LGLStructs::ModelInfo> model);
	LGL_API void CreateText(const std::string& textLabel, LGLStructs::TextInfo& text);

	LGL_API void DeleteModel(const std::string& modelName);
	LGL_API void DeleteText(const std::string& textLabel);
#endif
	LGL_API bool ConfigureTexture(const std::string& modelName, const LGLStructs::Texture& texture);
	LGL_API bool ConfigueGlyphTexture(const std::string& collectionName, const LGLStructs::GlyphTexture& glyphText);

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
	LGL_API void SetFramebufferSizeCallback(std::function<void(int, int)> callbackFunc);
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

	void UpdateWindowSize(int width, int height);

	int CheckUniformValueLocation(
		const std::string& valueName, 
		const std::string& shaderProgramName, 
		ShaderProgramID& shaderProgramID
	);

	void CreateRenderTextVO();

	// If no name is given will compile last loaded shader
	bool CompileShader(ShaderType shaderType, const std::string& name = "");
	bool LoadShaderFromFile(const std::string& name, const std::string& file, const std::string& shaderType);

	// If no list of shaders is provided, will create a program with all compiled shaders
	bool CreateShaderProgram(const std::string& name, const std::vector<std::string>& shaderVector = {});
	ShaderProgramID SetCurrentShaderProg(const std::string& shaderProg);

	bool ConfigureTextureImpl(TextureID& newTextureID, const LGLStructs::Texture& texture);

	// If shader file names can be identical to shader program name, general load and compile can be used
	bool LoadAndCompileShader(const std::string& name);

	// Callbacks
	CALLBACK GLFWErrorCallback(int errorCode, const char* description);

	CALLBACK FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	std::function<void(int, int)> framebufferSizeFunc;
	
	CALLBACK CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	std::function<void(double, double)> cursorPositionFunc;
	
	CALLBACK ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	std::function<void(double, double)> scrollCallbackFunc;

	CALLBACK KeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	std::function<void(int, int, int, int)> keyPressCallbackFunc;

	std::function<void(float)> renderTimeCallbackFunc;

	static LGL* CheckAndGetInstanceByContext(GLFWwindow* window);
	static std::map<GLFWwindow*, LGL*> contextToInstance;

	void ProcessInput();
	void Render();
	void RenderText();

	int windowWidth;
	int windowHeight;

	float renderDeltaTime;

	bool useVSync; // Passed value is not bool, but will do for on/off switch
	bool pauseRendering;
	bool stopRendering;
	std::mutex pauserMux;
	std::condition_variable pauser;

	GLFWwindow* window;

	glm::vec4 background;

	VAOInfo currentVAOToRender;
	std::vector<VBO> VBOCollection;
	InternalModelMap internalModelMap;
	std::vector<EBO> EBOCollection;

	VAO renderTextVAO;
	VBO renderTextVBO;
	bool renderTextVOCreated;
	std::map<std::string, LGLStructs::TextInfo*> internalTextMap;
	std::map<std::string, std::map<char, TextureID>> collectionToCharTextures;

	// Shader
	static std::map<std::string, ShaderType> shaderTypeChoice;
	std::string shaderPath;
	std::string lastProgram;
	std::map<ShaderName, std::pair<ShaderProgramID, std::map<ShaderType, ShaderInfo>>> shaderInfoCollection;

	std::map<size_t, InteractableInfo> interactCollection;

	bool batchUniformVals;
	bool hashUniformVals;
	std::vector<std::string> uniformErrorAntispam;
	std::unordered_set<size_t> uniformLocationTracker;
	std::unordered_map<ShaderProgramID, std::unordered_map<std::string, int>> uniformLocationCache;
	std::unique_ptr<LGLUniformHasher> uniformHasher;
};

#undef CALLBACK
