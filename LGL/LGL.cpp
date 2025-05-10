#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <array>

#define LGL_EXPORT
#include "LGL.h"
#include "stdEx/mapEx.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLExecutor.h"

#include "LGLKeyToStringMap.h"

#include "ContextManager.h"
#define ContextLock ContextManager<GLFWwindow> mux(window, [this](GLFWwindow* context){ glfwMakeContextCurrent(context); });
std::recursive_mutex ContextManager<GLFWwindow>::rMutex;
size_t ContextManager<GLFWwindow>::counter = 0;

using namespace LGLStructs;

std::function<void(double, double)> LGL::cursorPositionFunc = nullptr;
std::function<void(double, double)> LGL::scrollCallbackFunc = nullptr;
std::function<void(int, int, int, int)> LGL::keyPressCallbackFunc = nullptr;

std::map<std::string, LGL::ShaderType> LGL::shaderTypeChoice =
{
	{"vert", GL_VERTEX_SHADER},
	{"frag", GL_FRAGMENT_SHADER},
	{"geom", GL_GEOMETRY_SHADER},
};

const std::vector<int> LGL::LGLEnumInterpreter::DepthTestModeInter =
{
	{0, GL_ALWAYS, GL_NEVER, GL_LESS, GL_GREATER, GL_EQUAL, GL_NOTEQUAL, GL_LEQUAL, GL_GEQUAL}
};

const std::vector<int> LGL::LGLEnumInterpreter::TextureOverlayTypeInter =
{
	{GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER}
};

LGL::LGL()
{
	background = { 0, 0, 0, 1 };
	currentVAOToRender = {};
	window = nullptr;
	pauseRendering = false;

	std::cout << "Created LambdaGL instance\n";
}

LGL::~LGL()
{
	for (auto& model : modelToVAOMap)
	{
		for (auto& VAO : model.second.second)
		{
			GLSafeExecute(glDeleteVertexArrays, 1, &VAO.vboId);
		}
	}
	for (auto& VBO : VBOCollection)
	{
		GLSafeExecute(glDeleteBuffers, 1, &VBO);
	}
	for (auto& EBO : EBOCollection)
	{
		GLSafeExecute(glDeleteBuffers, 1, &EBO);
	}
	for (auto& shaderInfo : shaderInfoCollection)
	{
		for (auto& shader : shaderInfo.second)
		{
			GLSafeExecute(glDeleteShader, shader.shaderId);
		}
	}
	for (auto& shaderProgram : shaderProgramCollection)
	{
		GLSafeExecute(glDeleteProgram, shaderProgram.second);
	}
	for (auto& texture : textureCollection)
	{
		GLSafeExecute(glDeleteTextures, 1, &texture.second);
	}

	std::cout << "LambdaGL instance destroyed\n";
}

bool LGL::CreateWindow(const int height, const int width, const std::string& title)
{
	if (window)
	{
		std::cout << "Window already created\n";
		return false;
	}

	window = glfwCreateWindow(height, width, title.c_str(), nullptr, nullptr);

	if (!window)
	{
		std::cout << "Failed to crate GLFW window\n";
		return false;
	}

	glfwMakeContextCurrent(window);

	if (!glfwGetCurrentContext())
	{
		std::cerr << "Context does not set correctly\n";
		return false;
	}

	std::cout << "Created a GLFW window by address " << window << '\n';

	InitGLAD();
	InitCallbacks();

	return true;
}

void LGL::InitOpenGL(int major, int minor)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::cout << "OpenGL initialized\n";
}

bool LGL::InitGLAD()
{
	ContextLock

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to init GLAD\n";
		return false;
	}

	SetDepthTest(DepthTestMode::Less);

	std::cout << "GLAD initialized\n";

	return true;
}

void LGL::InitCallbacks()
{
	ContextLock

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetErrorCallback(GLFWErrorCallback);
}

void LGL::TerminateOpenGL()
{
	glfwTerminate();

	std::cout << "Terminate OpenGL\n";
}

void LGL::SetDepthTest(DepthTestMode depthTestMode)
{
	ContextLock

	if (depthTestMode == DepthTestMode::Disable)
	{
		GLSafeExecute(glDisable, GL_DEPTH_TEST);
	}
	else
	{
		GLSafeExecute(glEnable, GL_DEPTH_TEST);
		GLSafeExecute(glDepthFunc, static_cast<GLenum>(LGLEnumInterpreter::DepthTestModeInter[static_cast<GLenum>(depthTestMode)]));
	}
}

void LGL::CaptureMouse(bool value)
{
	ContextLock

	glfwSetInputMode(window, GLFW_CURSOR, value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	std::cout << "Mouse has been captured\n";
}

void LGL::SetCursorPositionCallback(std::function<void(double, double)> callbackFunc)
{
	glfwSetCursorPosCallback(window, callbackFunc ? CursorPositionCallback : nullptr);
	cursorPositionFunc = callbackFunc;

	std::cout << "Cursor callback set\n";
}

void LGL::SetScrollCallback(std::function<void(double, double)> callbackFunc)
{
	glfwSetScrollCallback(window, callbackFunc ? ScrollCallback : nullptr);
	scrollCallbackFunc = callbackFunc;

	std::cout << "Scroll callback set\n";
}

void LGL::SetKeyPressCallback(std::function<void(int, int, int, int)> callbackFunc)
{
	glfwSetKeyCallback(window, callbackFunc ? KeyPressCallback : nullptr);
	keyPressCallbackFunc = callbackFunc;

	std::cout << "Key press callback set\n";
}

int LGL::GetMaxAmountOfVertexAttr()
{
	ContextLock

	int attr;
	GLSafeExecute(glGetIntegerv, GL_MAX_VERTEX_ATTRIBS, &attr);
	
	std::cout << "Max amount of vertex attributes: " << attr << '\n';
	
	return attr;
}

void LGL::ProcessInput()
{
	for (auto& interact : interactCollection)
	{
		int retCode = glfwGetKey(window, interact.first);

		if (retCode == GLFW_PRESS)
		{
			if (interact.second.pressedFunc && (interact.second.holdable || !interact.second.pressed))
			{
				interact.second.pressedFunc();
			}

			interact.second.pressed = true;
		}
		else if (retCode == GLFW_RELEASE && interact.second.pressed)
		{
			interact.second.pressed = false;

			if (interact.second.releasedFunc)
			{
				interact.second.releasedFunc();
			}
		}
	}
}

void LGL::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void LGL::Render()
{
	if (currentVAOToRender.vboId != 0)
	{
		if (!currentVAOToRender.useIndices)
		{
			GLSafeExecute(glDrawArrays, GL_TRIANGLES, 0, currentVAOToRender.pointAmount);
		}
		else
		{
			GLSafeExecute(glDrawElements, GL_TRIANGLES, currentVAOToRender.pointAmount, GL_UNSIGNED_INT, nullptr);
		}

		uniformLocationTracker.clear();
	}
}

void LGL::RunRenderingCycle(std::function<void()> additionalSteps)
{
	std::array<int, Texture::GetTextureTypeAmount()> textureTypesToUnbind;
	std::fill(textureTypesToUnbind.begin(), textureTypesToUnbind.end(), false);

	while (!glfwWindowShouldClose(window))
	{
		ContextLock

		if(pauseRendering) continue;

		ProcessInput();

		GLSafeExecute(glClearColor, background.r, background.g, background.b, background.a);
		GLSafeExecute(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (additionalSteps)
		{
			additionalSteps();
		}

		for (auto& currentModelToProcess : modelToVAOMap)
		{
			std::function<void()>& modelBeh = currentModelToProcess.second.first->modelBehaviour;
			if (modelBeh)
			{
				modelBeh();
			}

			for (size_t meshIndex = 0; meshIndex < currentModelToProcess.second.second.size(); ++meshIndex)
			{
				auto& currentVAO = currentModelToProcess.second.second[meshIndex];

				if (currentVAO.meshInfo->render)
				{
					currentVAOToRender = currentVAO;

					std::string shaderProgramToCheck = currentVAO.meshInfo->shaderProgram;
					if (lastProgram != shaderProgramToCheck)
					{
						lastProgram = shaderProgramToCheck;
						GLSafeExecute(glUseProgram, shaderProgramCollection[lastProgram]);
					}

					GLSafeExecute(glBindVertexArray, currentVAO.vboId);

					for (auto& texture : currentVAO.meshInfo->mesh.textures)
					{
						auto currentTextureIter = textureCollection.find(texture.name);
						if (currentTextureIter != textureCollection.end())
						{
							TextureID textureID = (*currentTextureIter).second;
							int convertedTextureType = static_cast<int>(texture.type);
							GLSafeExecute(glActiveTexture, GL_TEXTURE0 + convertedTextureType);
							GLSafeExecute(glBindTexture, GL_TEXTURE_2D, textureID);

							textureTypesToUnbind[convertedTextureType] = true;
						}
					}

					std::function<void(int)> behaviourToCheck = currentVAO.meshInfo->behaviour;
					if (behaviourToCheck)
					{
						behaviourToCheck(meshIndex);
					}

					Render();

					for (auto& textureTypeToUnbind : textureTypesToUnbind)
					{
						if (textureTypeToUnbind)
						{
							GLSafeExecute(glActiveTexture, GL_TEXTURE0 + textureTypeToUnbind);
							GLSafeExecute(glBindTexture, GL_TEXTURE_2D, 0);
							textureTypeToUnbind = false;
						}
					}
				}
			}

			currentVAOToRender = {};
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void LGL::PauseRendering(bool value)
{
	pauseRendering = value;
}

void LGL::SetStaticBackgroundColor(const glm::vec4& rgba)
{
	background = rgba;
}

void LGL::CreateMesh(const std::string& modelName, MeshInfo& meshInfo)
{
	ContextLock

	auto CollectSteps = []() {
		std::vector<size_t> steps;

		for (size_t i = 0; i < LGLStructs::BasicVertex::GetLocalMemberAmount(); ++i)
		{
			steps.push_back(LGLStructs::BasicVertex::GetMemberElementSize());
		}

		for (size_t i = 0; i < LGLStructs::Vertex::GetLocalMemberAmount(); ++i)
		{
			steps.push_back(LGLStructs::Vertex::GetMemberElementSize());
		}

		return steps;
	};

	if (modelToVAOMap.find(modelName) == modelToVAOMap.end())
	{
		assert(false && "Trying to add mesh to non existent model");
		return;
	}

	auto& newVAOInfo = modelToVAOMap[modelName];

	std::vector<size_t> steps = CollectSteps();

	newVAOInfo.second.push_back({});
	VAO* newVAO = &newVAOInfo.second.back().vboId;
	GLSafeExecute(glGenVertexArrays, 1, newVAO);
	GLSafeExecute(glBindVertexArray, *newVAO);

	VBOCollection.push_back(VBO());
	VBO* newVBO = &VBOCollection.back();

	GLSafeExecute(glGenBuffers, 1, newVBO);
	GLSafeExecute(glBindBuffer, GL_ARRAY_BUFFER, *newVBO);
	GLSafeExecute(
		glBufferData,
		GL_ARRAY_BUFFER, 
		meshInfo.mesh.vert.size() * sizeof(Vertex),
		&meshInfo.mesh.vert[0],
		meshInfo.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
	);

	if (!meshInfo.mesh.indices.empty())
	{
		EBOCollection.push_back(EBO());
		EBO* newEBO = &EBOCollection.back();

		GLSafeExecute(glGenBuffers, 1, newEBO);
		GLSafeExecute(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, *newEBO);
		GLSafeExecute(
			glBufferData,
			GL_ELEMENT_ARRAY_BUFFER, 
			meshInfo.mesh.indices.size() * sizeof(unsigned int),
			&meshInfo.mesh.indices[0],
			meshInfo.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
		);
		newVAOInfo.second.back().useIndices = true;
		newVAOInfo.second.back().pointAmount = meshInfo.mesh.indices.size();
	}
	else
	{
		newVAOInfo.second.back().pointAmount = meshInfo.mesh.vert.size();
	}

	newVAOInfo.second.back().meshInfo = &meshInfo;


	// The whole secton needs to be generalized more
	size_t step = 0;
	size_t stride = 0;
	for (int i = 0; i < steps.size(); ++i)
	{
		if (i == 5)
		{
			stride += steps[i] * sizeof(int);
		}
		else
		{
			stride += steps[i] * sizeof(float);
		}
	}

	size_t byteOffset = 0;
	for (int i = 0; i < steps.size(); ++i)
	{
		glEnableVertexAttribArray(i);

		if (i == 5)
		{
			glVertexAttribIPointer(i, steps[i], GL_INT, stride, (void*)(byteOffset));
			byteOffset += steps[i] * sizeof(int);
		}
		else
		{
			glVertexAttribPointer(i, steps[i], GL_FLOAT, GL_FALSE, stride, (void*)(byteOffset));
			byteOffset += steps[i] * sizeof(float);
		}
	}

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	int polygons = newVAOInfo.second.back().pointAmount / 3;
	std::cout << "Mesh with " << newVAOInfo.second.back().pointAmount << " point(s) / " << polygons << " polygons created\n";

	LoadAndCompileShader(meshInfo.shaderProgram);
	for (auto& texture : meshInfo.mesh.textures)
	{
		ConfigureTexture(texture);
	}
}

void LGL::CreateModel(const std::string& modelName, LGLStructs::ModelInfo& model)
{
	if (modelToVAOMap.find(modelName) == modelToVAOMap.end())
	{
		modelToVAOMap.emplace(modelName, ModelAndVAOCollection{ &model, {} });
	}

	for (auto& mesh : model.meshes)
	{
		CreateMesh(modelName, mesh);
	}
}

#ifdef ENABLE_OLD_MODEL_IMPORT

void LGL::GetMeshFromFile(const std::string& file, std::vector<Vertex>& vertexes, std::vector<unsigned int>& indeces)
{
	std::ifstream reader(file);

	if (!reader)
	{
		std::cout << "Could not open mesh file " << file << '\n';
	}
	
	std::map<std::string, int> typeNames
	{
		{"v", 0},
		{"vt", 1},
		{"vn", 2}
	};

	int fileSection = 0;

	std::string line;
	std::string indexValue;

	std::vector<Vertex> allVertexes;
	std::unordered_map<std::string, int> objIndeces;
	int indeceCount = 0;
	std::vector<Vertex> resVertexes;
	std::map<int, int> indexes;

	std::vector<int> meshIndexer { 0, 0, 0 };
	std::vector<unsigned int> resIndeces;

	auto parseOneObjIndece = [](const std::string& value)
	{
		std::map<int, int> indexes;
		std::string collected;
		int count = 0;
			
		for (auto& i : value)
		{
			if (i == '/')
			{
				indexes[count++] = collected == "" ? 0 : std::stoi(collected) - 1;
				collected = "";
			}
			else
			{
				collected += i;
			}
		}

		indexes[count++] = collected == "" ? 0 : std::stoi(collected) - 1;

		return indexes;
	};

	auto CheckForFileSectionChange = [&line, &fileSection]()
	{
		stdEx::map<char, int> charToFilesection;
		charToFilesection['v'] = 1;
		charToFilesection['f'] = 2;
		charToFilesection.SetDefaultValue(0);

		fileSection = charToFilesection.at(line[0]);
	};

	auto CheckVertexVectors = [&line, &fileSection, &meshIndexer, &typeNames, &allVertexes]()
	{
		if (line[0] == 'v')
		{
			std::string geoType = "";
			std::string value = "";
			glm::vec3 convertedValues;
			int indexOfValueToCollect = 0;

			for (int i = 0; i < line.size() + 1 && indexOfValueToCollect < 4; ++i)
			{
				if (!value.empty() && (line[i] == ' ' || line[i] == '\0'))
				{
					if (indexOfValueToCollect == 0)
					{
						geoType = value;
					}
					else
					{
						convertedValues[indexOfValueToCollect - 1] = std::stof(value);
					}

					++indexOfValueToCollect;
					value = "";
				}
				else
				{
					value += line[i];
				}
			}

			if (meshIndexer[typeNames[geoType]] >= allVertexes.size())
			{
				allVertexes.push_back(Vertex{});
			}

			Vertex& vertexToSet = allVertexes[meshIndexer[typeNames[geoType]]];

			switch (typeNames[geoType])
			{
			case 0:
				vertexToSet.Position = convertedValues;
				break;
			case 1:
				vertexToSet.TexCoords = convertedValues;
				break;
			case 2:
				vertexToSet.Normal = convertedValues;
				break;
			}

			++meshIndexer[typeNames[geoType]];
		}
	};

	auto CheckIndecesAndFormResult = 
		[&line, &objIndeces, &resIndeces, &indeceCount, &indexes, &parseOneObjIndece, &resVertexes, &allVertexes]()
	{
		int currentCoordType = 0;
		std::string value = "";

		if (line[0] == 'f')
		{
			for (int i = 2; i < line.size() + 1; ++i)
			{
				if (!value.empty() && (line[i] == ' ' || line[i] == '\0'))
				{
					if (objIndeces.find(value) == objIndeces.end())
					{
						resIndeces.push_back(indeceCount);
						objIndeces[value] = indeceCount++;

						indexes = parseOneObjIndece(value);
						resVertexes.push_back(
							Vertex{
								allVertexes[indexes[0]].Position,
								allVertexes[indexes[1]].TexCoords,
								allVertexes[indexes[2]].Normal
							}
						);
					}
					else 
					{
						resIndeces.push_back(objIndeces[value]);
					}

					value = "";
				}
				else
				{
					value += line[i];
				}
			}
		}
	};

	stdEx::map<int, std::function<void()>> processFileFuncs;
	processFileFuncs[1] = CheckVertexVectors;
	processFileFuncs[2] = CheckIndecesAndFormResult;
	processFileFuncs.SetDefaultValue([](){});

	while (std::getline(reader, line))
	{
		CheckForFileSectionChange();
		processFileFuncs.at(fileSection)();
	}

	vertexes = resVertexes;
	indeces = resIndeces;
}
#endif

bool LGL::CompileShader(const std::string& name)
{
	using AcceptableShaderCode = const char* const;

	ContextLock

	if (shaderInfoCollection.find(name) == shaderInfoCollection.end())
	{
		std::cout << "Shader by this name is not found\n";
		return false;
	}

	ShaderInfo& currentShaderInfo = shaderInfoCollection[name].back();
	AcceptableShaderCode shaderToC = currentShaderInfo.shaderCode.c_str();

	Shader* newShader = &currentShaderInfo.shaderId;

	GLSafeExecute(glShaderSource, *newShader, 1, &shaderToC, nullptr);
	bool shaderCompiled = GLSafeExecute(glCompileShader, *newShader);

	return shaderCompiled;
}

bool LGL::LoadShaderFromFile(const std::string& name, const std::string& file, const std::string& shaderType)
{
	std::string shader; // change to stringstream
	std::string line;

	bool atLeastOneFileLoaded = shaderInfoCollection.find(name) != shaderInfoCollection.end();

	std::ifstream reader(file);

	if (!reader)
	{
		std::cout << file + " shader does not exist\n";
		return atLeastOneFileLoaded;
	}

	while (std::getline(reader, line))
	{
		shader += (line + '\n');
	}

	shaderInfoCollection[name].emplace_back(
		ShaderInfo{
			glCreateShader(shaderTypeChoice[shaderType]),
			shader
		}
	);

	std::cout << "Shader " << file << " loaded\n";

	return true;
}

bool LGL::ConfigureTexture(const Texture& texture)
{
	ContextLock

	if (textureCollection.find(texture.name) != textureCollection.end())
	{
		return true;
	}

	textureCollection[texture.name] = TextureID();

	TextureID& newTextureID = textureCollection[texture.name];
	GLSafeExecute(glGenTextures, 1, &newTextureID);
	GLSafeExecute(glBindTexture, GL_TEXTURE_2D, newTextureID);

	float color[] {
		texture.params.color.r,
		texture.params.color.g,
		texture.params.color.b,
		texture.params.color.a,
	};

	GLSafeExecute(
		glTexParameteri,
		GL_TEXTURE_2D, 
		GL_TEXTURE_WRAP_S, 
		LGLEnumInterpreter::TextureOverlayTypeInter[static_cast<int>(texture.params.overlay)]
	);
	GLSafeExecute(
		glTexParameteri,
		GL_TEXTURE_2D, 
		GL_TEXTURE_WRAP_T, 
		LGLEnumInterpreter::TextureOverlayTypeInter[static_cast<int>(texture.params.overlay)]
	);

	//GLSafeExecute(glTexParameterfv, GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	if (texture.params.createMipmaps) 
	{
		int glMipParams[2][2]
		{
			{ GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST },
			{ GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST}
		};

		GLSafeExecute(
			glTexParameteri,
			GL_TEXTURE_2D, 
			GL_TEXTURE_MIN_FILTER, 
			GL_NEAREST_MIPMAP_NEAREST//glMipParams[texture.params.mipmapBFConfig.minFilter][texture.params.BFConfig.minFilter]
		);
		GLSafeExecute(
			glTexParameteri,
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST//glMipParams[texture.params.mipmapBFConfig.maxFilter][texture.params.BFConfig.maxFilter]
		);

		GLSafeExecute(glGenerateMipmap, GL_TEXTURE_2D);
	}
	else 
	{
		int glParams[]{ GL_LINEAR, GL_NEAREST };

		GLSafeExecute(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glParams[texture.params.BFConfig.minFilter]);
		GLSafeExecute(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glParams[texture.params.BFConfig.maxFilter]);
	}

	unsigned int textureFormat;

	switch (texture.channelAmount)
	{
	case 1:
		textureFormat = GL_RED;
		break;
	case 3:
		textureFormat = GL_RGB;
		break;
	case 4:
		textureFormat = GL_RGBA;
		break;
	default:
		std::cout << "Unknown format\n";
		return false;
	}

	GLSafeExecute(glPixelStorei, GL_UNPACK_ALIGNMENT, textureFormat == 3 ? 1 : 4);


	GLSafeExecute(
		glTexImage2D,
		GL_TEXTURE_2D, 
		0, 
		textureFormat,
		texture.width, 
		texture.height, 
		0, 
		textureFormat,
		GL_UNSIGNED_BYTE, 
		texture.data
	);

	std::cout << "Texture " << texture.name << " configured\n";

	return true;
}

bool LGL::CreateShaderProgram(const std::string& name, const std::vector<std::string>& shaderNames)
{	
	ContextLock

	shaderProgramCollection.emplace(name, glCreateProgram());
	ShaderProgram* newShaderProgram = &shaderProgramCollection[name];

	for (auto& shaderInfo : shaderInfoCollection[name])
	{
		GLSafeExecute(glAttachShader, *newShaderProgram, shaderInfo.shaderId);
	}
	GLSafeExecute(glLinkProgram, *newShaderProgram);

	int success;

	GLSafeExecute(glGetProgramiv, *newShaderProgram, GL_LINK_STATUS, &success);

	std::cout << "Shader program: " << name << " created\n";

	return success;
}

void LGL::SetShaderFolder(const std::string& path)
{
	shaderPath = path;
}

void LGL::RecompileShader(const std::string& shaderName)
{
	ContextLock

	DeleteShader(shaderName);

	shaderInfoCollection.erase(shaderName);
	shaderProgramCollection.erase(shaderName);

	LoadAndCompileShader(shaderName);
}

void LGL::DeleteShader(const std::string& shaderName)
{
	lastProgram.clear();
	GLSafeExecute(glUseProgram, 0);

	for (auto& shaderInfo : shaderInfoCollection[shaderName])
	{
		GLSafeExecute(glDeleteShader, shaderInfo.shaderId);
	}

	GLSafeExecute(glDeleteProgram, shaderProgramCollection[shaderName]);
}

bool LGL::LoadAndCompileShader(const std::string& name)
{
	if (shaderProgramCollection.find(name) != shaderProgramCollection.end())
	{
		return true;
	}

	shaderInfoCollection[name] = {};

	for (const auto& shaderFileType : shaderTypeChoice)
	{
		if (!LoadShaderFromFile(name, shaderPath + '\\' + name + '.' + shaderFileType.first, shaderFileType.first)) continue;
		if (!CompileShader(name)) // remove if did not compile
		{
			shaderInfoCollection[name].pop_back();
		}
	}

	return shaderInfoCollection[name].size() && CreateShaderProgram(name);
}

void LGL::SetInteractable(
	int keyID,
	bool holdable,
	const std::function<void()>& pressedFunc,
	const std::function<void()>& releasedFunc
)
{
	interactCollection[keyID] = { false, holdable, pressedFunc, releasedFunc };
	
	std::cout << "Interactable for keyId " << keyID << " set\n";
}

std::string LGL::ConvertKeyTo(int keyId)
{
	auto& keyToStringMap = LGLKeyToStringMap::keyToStringMap;

	return keyToStringMap.Exists(keyId) ? keyToStringMap[keyId] : "InvalidKey";
}

int LGL::ConvertKeyTo(const std::string& keyName)
{
	auto& keyToStringMap = LGLKeyToStringMap::keyToStringMap;

	return keyToStringMap.Exists(keyName) ? keyToStringMap[keyName] : -1;
}

void LGL::GLFWErrorCallback(int errorCode, const char* description)
{
	int x = errorCode;
}

void LGL::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (cursorPositionFunc)
	{
		cursorPositionFunc(xpos, ypos);
	}
}

void LGL::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (scrollCallbackFunc)
	{
		scrollCallbackFunc(xoffset, yoffset);
	}
}

void LGL::KeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(keyPressCallbackFunc)
	{
		keyPressCallbackFunc(key, scancode, action, mods);
	}
}

void LGL::SetAssetOnOpenGLFailure(bool value)
{
	GLExecutor::SetAssertOnFailure(value);
	std::cout << "AssertOnFailure has been set to " << value << '\n';
}

#define UniformAdapterSection

#define ShaderCallTypeAndVector(type, glFunc)                                                        \
{ typeid(type), [](int uniformValueLocation, void* value)                                            \
{                                                                                                    \
	GLSafeExecute(glFunc, uniformValueLocation, *reinterpret_cast<type*>(value));                    \
}                                                                                                    \
},                                                                                                   \
{ typeid(std::vector<type>), [](int uniformValueLocation, void* value)								 \
{																									 \
	std::vector<type>& vals = *reinterpret_cast<std::vector<type>*>(value);							 \
	GLSafeExecute(##glFunc##v, uniformValueLocation, vals.size(), &vals[0]);						 \
}																									 \
}                                                                                                    

#define ShaderCallVectVector(type, glFunc)                                                           \
{ typeid(std::vector<type>), [](int uniformValueLocation, void* value)                               \
{                                                                                                    \
	std::vector<type>& vals = *reinterpret_cast<std::vector<type>*>(value);                          \
	GLSafeExecute(##glFunc##v, uniformValueLocation, vals.size(), glm::value_ptr(vals[0]));          \
}                                                                                                    \
}

#define ShaderCallVect2AndVector(type, glFunc)                                                       \
{ typeid(type), [](int uniformValueLocation, void* value)                                            \
{                                                                                                    \
	type& coords = *reinterpret_cast<type*>(value);                                                  \
	GLSafeExecute(glFunc, uniformValueLocation, coords.x, coords.y);                                 \
}                                                                                                    \
},                                                                                                   \
ShaderCallVectVector(type, glFunc)                                                                   \

#define ShaderCallVect3AndVector(type, glFunc)                                                       \
{ typeid(type), [](int uniformValueLocation, void* value)                                            \
{                                                                                                    \
	type& coords = *reinterpret_cast<type*>(value);                                                  \
	GLSafeExecute(glFunc, uniformValueLocation, coords.x, coords.y, coords.z);                       \
}                                                                                                    \
},                                                                                                   \
ShaderCallVectVector(type, glFunc)

#define ShaderCallVect4AndVector(type, glFunc)                                                       \
{ typeid(type), [](int uniformValueLocation, void* value)                                            \
{                                                                                                    \
	type& coords = *reinterpret_cast<type*>(value);                                                  \
	GLSafeExecute(glFunc, uniformValueLocation, coords.x, coords.y, coords.z, coords.w);             \
}                                                                                                    \
},                                                                                                   \
ShaderCallVectVector(type, glFunc)

#define ShaderCallMatrixAndVector(type, glFunc)                                                                 \
{ typeid(type), [](int uniformValueLocation, void* value)                                                       \
{                                                                                                               \
	GLSafeExecute(glFunc, uniformValueLocation, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<type*>(value)));  \
}                                                                                                               \
},                                                                                                              \
{ typeid(std::vector<type>), [](int uniformValueLocation, void* value)                                          \
{                                                                                                               \
	std::vector<type>& vals = *reinterpret_cast<std::vector<type>*>(value);                                     \
	GLSafeExecute(##glFunc, uniformValueLocation, vals.size(), GL_FALSE, glm::value_ptr(vals[0]));              \
}                                                                                                               \
}

const std::unordered_map<std::type_index, std::function<void(int, void*)>> uniformValueLocators
{
	ShaderCallTypeAndVector  (int,          glUniform1i),
	ShaderCallTypeAndVector  (unsigned int, glUniform1ui),
	ShaderCallTypeAndVector  (float,        glUniform1f),
	ShaderCallVect2AndVector (glm::ivec2,   glUniform2i),
	ShaderCallVect3AndVector (glm::ivec3,   glUniform3i),
	ShaderCallVect4AndVector (glm::ivec4,   glUniform4i),
	ShaderCallVect2AndVector (glm::uvec2,   glUniform2ui),
	ShaderCallVect3AndVector (glm::uvec3,   glUniform3ui),
	ShaderCallVect4AndVector (glm::uvec4,   glUniform4ui),
	ShaderCallVect2AndVector (glm::vec2,    glUniform2f),
	ShaderCallVect3AndVector (glm::vec3,    glUniform3f),
	ShaderCallVect4AndVector (glm::vec4,    glUniform4f),
	ShaderCallMatrixAndVector(glm::mat2,    glUniformMatrix2fv),
	ShaderCallMatrixAndVector(glm::mat3,    glUniformMatrix3fv),
	ShaderCallMatrixAndVector(glm::mat4,    glUniformMatrix4fv),
};


int LGL::CheckUniformValueLocation(const std::string& valueName, const std::string& shaderProgramName)
{
	std::string shaderProgToUse = shaderProgramName == "" ? lastProgram : shaderProgramName;

	if (shaderProgramCollection.find(shaderProgToUse) != shaderProgramCollection.end())
	{
		ShaderProgram shaderProgramToUse = shaderProgramCollection[shaderProgToUse];

		int uniformValueLocation = glGetUniformLocation(shaderProgramToUse, valueName.c_str());

		if (uniformValueLocation == -1)
		{
			if (std::find(uniformErrorAntispam.begin(), uniformErrorAntispam.end(), valueName) == std::end(uniformErrorAntispam))
			{
				std::cout << "[ERROR] Shader value " + valueName << " could not be located\n";
				uniformErrorAntispam.push_back(valueName);
			}
		}

		return uniformValueLocation;
	}

	return -1;
}

template<typename Type>
bool LGL::SetShaderUniformValue(const std::string& valueName, Type&& value, const std::string& shaderProgramName)
{
	int uniformValueLocation = CheckUniformValueLocation(valueName, shaderProgramName);

	if (uniformValueLocation == -1 || lastProgram.empty())
	{
		return false;
	}

	if (uniformLocationTracker.find(uniformValueLocation) != uniformLocationTracker.end())
	{
		Render();
	}
	uniformLocationTracker.insert(uniformValueLocation);
	uniformValueLocators.at(typeid(Type))(uniformValueLocation, &value);

	return true;
}

// Produces explicit instantiation of type and vector of type
#define ShaderUniformValueExplicit(Type) \
template LGL_API bool LGL::SetShaderUniformValue<Type>(const std::string& valueName, Type&& value, const std::string& shaderProgramName);                           \
template LGL_API bool LGL::SetShaderUniformValue<Type&>(const std::string& valueName, Type& value, const std::string& shaderProgramName);                           \
template LGL_API bool LGL::SetShaderUniformValue<std::vector<Type>>(const std::string& valueName, std::vector<Type>&& value, const std::string& shaderProgramName); \
template LGL_API bool LGL::SetShaderUniformValue<std::vector<Type>&>(const std::string& valueName, std::vector<Type>& value, const std::string& shaderProgramName); 

ShaderUniformValueExplicit(int)
ShaderUniformValueExplicit(unsigned int)
ShaderUniformValueExplicit(float)
ShaderUniformValueExplicit(glm::ivec2)
ShaderUniformValueExplicit(glm::ivec3)
ShaderUniformValueExplicit(glm::ivec4)
ShaderUniformValueExplicit(glm::uvec2)
ShaderUniformValueExplicit(glm::uvec3)
ShaderUniformValueExplicit(glm::uvec4)
ShaderUniformValueExplicit(glm::vec2)
ShaderUniformValueExplicit(glm::vec3)
ShaderUniformValueExplicit(glm::vec4)
ShaderUniformValueExplicit(glm::mat2)
ShaderUniformValueExplicit(glm::mat3)
ShaderUniformValueExplicit(glm::mat4)

#undef ShaderCallTypeAndVector
#undef ShaderCallVectVector
#undef ShaderCallVect2AndVector
#undef ShaderCallVect3AndVector
#undef ShaderCallVect4AndVector
#undef ShaderCallMatrixAndVector

#undef ShaderUniformValueExplicit

#undef UniformAdapterSection