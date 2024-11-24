#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <array>

#include <random>
#include <algorithm>

#define LGL_EXPORT
#include "LGL.h"
#include "stdEx/mapEx.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLExecutor.h"

#include "ContextManager.h"
#define ContextLock ContextManager<GLFWwindow> mux(window, [this](GLFWwindow* context){ glfwMakeContextCurrent(context); });
std::recursive_mutex ContextManager<GLFWwindow>::rMutex;
size_t ContextManager<GLFWwindow>::counter = 0;

using namespace LGLStructs;

std::function<void(double, double)> LGL::cursorPositionFunc = nullptr;
std::function<void(double, double)> LGL::scrollCallbackFunc = nullptr;

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

const std::vector<int> LGL::LGLEnumInterpreter::SpecialKeyInter =
{
	{
		GLFW_KEY_ENTER, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, 
		GLFW_KEY_TAB, GLFW_KEY_BACKSPACE, GLFW_KEY_ESCAPE 
	}
};

LGL::LGL()
{
	background = { 0, 0, 0, 1 };
	currentVAOToRender = {};
	window = nullptr;

	std::cout << "Created LambdaGL instance\n";
}

LGL::~LGL()
{
	for (auto& VAO : VAOCollection)
	{
		GLSafeExecute(glDeleteVertexArrays, 1, &VAO.vboId);
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

void LGL::CaptureMouse()
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	std::cout << "Mouse has been captured\n";
}

void LGL::SetCursorPositionCallback(std::function<void(double, double)> callbackFunc)
{
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	cursorPositionFunc = callbackFunc;

	std::cout << "Cursor callback set\n";
}

void LGL::SetScrollCallback(std::function<void(double, double)> callbackFunc)
{
	glfwSetScrollCallback(window, ScrollCallback);
	scrollCallbackFunc = callbackFunc;

	std::cout << "Scroll callback set\n";
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

		if (retCode == GLFW_PRESS && interact.second.first)
		{
			interact.second.first();
		}
		else if (retCode == GLFW_RELEASE && interact.second.second)
		{
			interact.second.second();
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
	}
}

void LGL::RunRenderingCycle(std::function<void()> additionalSteps)
{
	std::array<int, Texture::GetTextureTypeAmount()> textureTypesToUnbind;
	std::fill(textureTypesToUnbind.begin(), textureTypesToUnbind.end(), false);

	while (!glfwWindowShouldClose(window))
	{
		ContextLock

		ProcessInput();

		GLSafeExecute(glClearColor, background.r, background.g, background.b, background.a);
		GLSafeExecute(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (additionalSteps)
		{
			additionalSteps();
		}

		for (auto& currentVAO : VAOCollection)
		{
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

				std::function<void()> behaviourToCheck = currentVAO.meshInfo->behaviour;
				if (behaviourToCheck)
				{
					behaviourToCheck();
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

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void LGL::SetStaticBackgroundColor(const glm::vec4& rgba)
{
	background = rgba;
}

void LGL::CreateMesh(MeshInfo& meshInfo)
{
	ContextLock

	std::vector<int> steps{ 3, 3, 3, 3, 3 };

	VAOCollection.push_back({0, false});
	VAO* newVAO = &VAOCollection.back().vboId;
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
	size_t stride = 0;
	for (int i = 0; i < steps.size(); ++i)
	{
		stride += steps[i];
	}

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
		VAOCollection.back().useIndices = true;
		VAOCollection.back().pointAmount = meshInfo.mesh.indices.size();
	}
	else
	{
		VAOCollection.back().pointAmount = meshInfo.mesh.vert.size();
	}

	VAOCollection.back().meshInfo = &meshInfo;

	stride *= sizeof(float);

	size_t step = 0;
	for (int i = 0; i < steps.size(); ++i)
	{
		step = !i ? 0 : (step + steps[i - 1]);
		GLSafeExecute(glEnableVertexAttribArray, i);
		GLSafeExecute(glVertexAttribPointer, i, steps[i], GL_FLOAT, false, stride, (void*)(step * sizeof(float)));
	}
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	int polygons = VAOCollection.back().pointAmount / 3;
	std::cout << "Mesh with " << VAOCollection.back().pointAmount << " point(s) / " << polygons << " polygons created\n";

	LoadAndCompileShader(meshInfo.shaderProgram);
	for (auto& texture : meshInfo.mesh.textures)
	{
		ConfigureTexture(texture);
	}
}

void LGL::CreateModel(LGLStructs::ModelInfo& model)
{
	for (auto& mesh : model.meshes)
	{
		CreateMesh(mesh);
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

bool LGL::LoadShaderFromFile(const std::string& name, const std::string& file)
{
	size_t typePos = file.find('.') + 1;
	std::string shaderType = file.substr(typePos);
	if (shaderTypeChoice.find(shaderType) == shaderTypeChoice.end())
	{
		std::cout << "Unknown shader type\n";
		return false;
	}

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

bool LGL::LoadAndCompileShader(const std::string& name)
{
	if (shaderProgramCollection.find(name) != shaderProgramCollection.end())
	{
		return true;
	}

	shaderInfoCollection[name] = {};

	for (const auto& shaderFileType : shaderTypeChoice)
	{
		if (!LoadShaderFromFile(name, "shaders\\" + name + '.' + shaderFileType.first)) continue;
		if (!CompileShader(name)) // remove if did not compile
		{
			shaderInfoCollection[name].pop_back();
		}
	}

	return shaderInfoCollection[name].size() && CreateShaderProgram(name);
}

void LGL::SetInteractable(unsigned char key, const OnPressFunction& preFunc, const OnReleaseFunction& relFunc)
{
	interactCollection[std::toupper(key)] = {preFunc, relFunc};
	
	std::cout << "Interactable for keyId " << key << " set\n";
}

void LGL::SetInteractable(SpecialKeys key, const OnPressFunction& preFunc, const OnReleaseFunction& relFunc)
{
	int specialKeyID = LGLEnumInterpreter::SpecialKeyInter[static_cast<int>(key)];
	interactCollection[specialKeyID] = {preFunc, relFunc};

	std::cout << "Interactable for special keyid " << specialKeyID << " set\n";
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

void LGL::SetAssetOnOpenGLFailure(bool value)
{
	GLExecutor::SetAssertOnFailure(value);
	std::cout << "AssertOnFailure has been set to " << value << '\n';
}

const std::unordered_map<std::type_index, std::function<void(int, void*)>> uniformValueLocators
{
	{ typeid(int), [](int uniformValueLocation, void* value) { glUniform1i(uniformValueLocation, *reinterpret_cast<int*>(value)); } },
	{ typeid(float), [](int uniformValueLocation, void* value) { glUniform1f(uniformValueLocation, *reinterpret_cast<float*>(value)); } },

	{ typeid(glm::vec3), [](int uniformValueLocation, void* value)
	{
		glm::vec3* coords = reinterpret_cast<glm::vec3*>(value);
		glUniform3f(uniformValueLocation, coords->x, coords->y, coords->z);
	} },

	{ typeid(glm::vec4), [](int uniformValueLocation, void* value)
	{
		glm::vec4* coords = reinterpret_cast<glm::vec4*>(value);
		glUniform4f(uniformValueLocation, coords->x, coords->y, coords->z, coords->w);
	} },

	{ typeid(glm::mat4), [](int uniformValueLocation, void* value)
	{
		glUniformMatrix4fv(uniformValueLocation, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<glm::mat4*>(value)));
	} }

};


int LGL::CheckUniformValueLocation(const std::string& valueName, const std::string& shaderProgramName)
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
	}

	return uniformValueLocation;
}

template<typename Type>
bool LGL::SetShaderUniformValue(const std::string& valueName, Type&& value, bool render, const std::string& shaderProgramName)
{
	int uniformValueLocation = CheckUniformValueLocation(valueName, shaderProgramName);
	if (uniformValueLocation == -1)
	{
		return false;
	}

	uniformValueLocators.at(typeid(Type))(uniformValueLocation, &value);

	if (render)
	{
		Render();
	}

	return true;
}

#define ShaderUniformValueExplicit(Type) \
template LGL_API bool LGL::SetShaderUniformValue<Type>(const std::string& valueName, Type&& value, bool render, const std::string& shaderProgramName); \
template LGL_API bool LGL::SetShaderUniformValue<Type&>(const std::string& valueName, Type& value, bool render, const std::string& shaderProgramName);

ShaderUniformValueExplicit(int)
ShaderUniformValueExplicit(float)
ShaderUniformValueExplicit(glm::vec3)
ShaderUniformValueExplicit(glm::vec4)
ShaderUniformValueExplicit(glm::mat4)

#undef ShaderUniformValueExplicit