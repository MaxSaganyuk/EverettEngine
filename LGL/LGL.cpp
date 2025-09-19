#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <array>
#include <chrono>

#include "LGLUniformHasher.h"

#define LGL_EXPORT
#include "LGL.h"
#include "stdEx/mapEx.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLExecutor.h"

#include "LGLKeyToStringMap.h"

#include "ContextManager.h"
#define ContextLock ContextManager<GLFWwindow> mux(window, [this](GLFWwindow* context){ glfwMakeContextCurrent(context); });
#define HandshakeContextLock \
PauseRendering(); \
ContextLock \
PauseRendering(false);
std::recursive_mutex ContextManager<GLFWwindow>::rMutex;
size_t ContextManager<GLFWwindow>::counter = 0;

using namespace LGLStructs;

std::map<GLFWwindow*, LGL*> LGL::contextToInstance;

std::map<std::string, LGL::ShaderType> LGL::shaderTypeChoice =
{
	{"vert", GL_VERTEX_SHADER},
	{"frag", GL_FRAGMENT_SHADER},
	{"geom", GL_GEOMETRY_SHADER},
};

bool LGL::InternalModelInfo::IsSmartPtrUsed()
{
	return isSmartPtrUsed;
}

void LGL::InternalModelInfo::SetModelPtr(LGLStructs::ModelInfo* modelRawPtr)
{
	this->modelRawPtr = modelRawPtr;
}

void LGL::InternalModelInfo::SetModelPtr(std::weak_ptr<LGLStructs::ModelInfo> modelWeakPtr)
{
	isSmartPtrUsed = true;
	this->modelWeakPtr = modelWeakPtr;
}

LGLStructs::ModelInfo* LGL::InternalModelInfo::GetModelPtr()
{
	return IsSmartPtrUsed() ? modelWeakPtr.lock().get() : modelRawPtr;
}

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
	windowWidth = -1;
	windowHeight = -1;
	currentVAOToRender = {};
	window = nullptr;
	pauseRendering = false;
	stopRendering = false;
	uniformHasher = std::make_unique<LGLUniformHasher>();
	batchUniformVals = true;
	hashUniformVals = true;
	useVSync = true;
	renderDeltaTime = 1.0f;
	renderTextVOCreated = false;

	std::cout << "Created LambdaGL instance\n";
}

LGL::~LGL()
{
	if (!stopRendering)
	{
		StopRenderingCycle();
	}

	contextToInstance.erase(window);
	glfwDestroyWindow(window);
	window = nullptr;

	std::cout << "LambdaGL instance destroyed\n";
}

void LGL::DeleteGLObjects()
{
	HandshakeContextLock

	GLSafeExecute(glBindVertexArray, 0);
	GLSafeExecute(glBindBuffer, GL_ARRAY_BUFFER, 0);
	GLSafeExecute(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSafeExecute(glUseProgram, 0);
	GLSafeExecute(glBindTexture, GL_TEXTURE_2D, 0);
	
	for (auto& modelIter : internalModelMap)
	{
		for (auto& VAO : modelIter.second.VAOs)
		{
			GLSafeExecute(glDeleteVertexArrays, 1, &VAO.vboId);
		}
		for (auto& texture : modelIter.second.textureIDs)
		{
			GLSafeExecute(glDeleteTextures, 1, &texture.second);
		}
	}
	internalModelMap.clear();
	internalTextMap.clear();

	for (auto& fontAndChars : collectionToCharTextures)
	{
		for (auto& chars : fontAndChars.second)
		{
			GLSafeExecute(glDeleteTextures, 1, &chars.second);
		}
	}
	collectionToCharTextures.clear();

	if (renderTextVOCreated)
	{
		GLSafeExecute(glDeleteVertexArrays, 1, &renderTextVAO);
		GLSafeExecute(glDeleteBuffers, 1, &renderTextVBO);
		renderTextVOCreated = false;
	}

	for (auto& VBO : VBOCollection)
	{
		GLSafeExecute(glDeleteBuffers, 1, &VBO);
	}
	VBOCollection.clear();

	for (auto& EBO : EBOCollection)
	{
		GLSafeExecute(glDeleteBuffers, 1, &EBO);
	}
	EBOCollection.clear();

	for (auto& shaderProgInfo : shaderInfoCollection)
	{
		for (auto& shaderIDInfo : shaderProgInfo.second.second)
		{
			GLSafeExecute(glDeleteShader, shaderIDInfo.second.shaderId);
		}
		GLSafeExecute(glDeleteProgram, shaderProgInfo.second.first);
	}

	if (uniformHasher)
	{
		uniformHasher->ResetHasher();
	}
	lastProgram.clear();
}

bool LGL::CreateWindow(const int width, const int height, const std::string& title, bool fullscreen)
{
	if (window)
	{
		std::cout << "Window already created\n";
		return false;
	}

	window = glfwCreateWindow(width, height, title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

	windowWidth = width;
	windowHeight = height;

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

	contextToInstance[window] = this;

	InitGLAD();
	InitCallbacks();

	return true;
}

int LGL::GetCurrentWindowWidth()
{
	return windowWidth;
}

int LGL::GetCurrentWindowHeight()
{
	return windowHeight;
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

	GLSafeExecute(glEnable, GL_BLEND);
	GLSafeExecute(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cout << "GLAD initialized\n";

	return true;
}

void LGL::InitCallbacks()
{
	ContextLock

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetErrorCallback(GLFWErrorCallback);

	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, true);
}

void LGL::TerminateOpenGL()
{
	glfwTerminate();

	std::cout << "Terminate OpenGL\n";
}

void LGL::SetDepthTest(DepthTestMode depthTestMode)
{
	HandshakeContextLock

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
	HandshakeContextLock

	glfwSetInputMode(window, GLFW_CURSOR, value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	std::cout << "Mouse has been captured\n";
}

void LGL::SetFramebufferSizeCallback(std::function<void(int, int)> callbackFunc)
{
	framebufferSizeFunc = callbackFunc;

	std::cout << "Framebuffer size callback\n";
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

void LGL::SetRenderDeltaCallback(std::function<void(float)> callbackFunc)
{
	renderTimeCallbackFunc = callbackFunc;

	std::cout << "Render delta callback set\n";
}

int LGL::GetMaxAmountOfVertexAttr()
{
	HandshakeContextLock

	int attr;
	GLSafeExecute(glGetIntegerv, GL_MAX_VERTEX_ATTRIBS, &attr);
	
	std::cout << "Max amount of vertex attributes: " << attr << '\n';
	
	return attr;
}

void LGL::ProcessInput()
{
	for (auto& interact : interactCollection)
	{
		int retCode = glfwGetKey(window, static_cast<int>(interact.first));

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
	LGL* instance = CheckAndGetInstanceByContext(window);

	if(instance)
	{
		if (instance->framebufferSizeFunc)
		{
			instance->framebufferSizeFunc(width, height);
		}

		instance->UpdateWindowSize(width, height);
	}
}

void LGL::EnableVSync(bool value)
{
	useVSync = value;
}

void LGL::RenderText()
{
	ContextLock

	for (auto& text : internalTextMap)
	{
		if (!text.second->render) continue;

		SetCurrentShaderProg(text.second->shaderProgram);

		GLSafeExecute(glActiveTexture, GL_TEXTURE0);
		GLSafeExecute(glBindVertexArray, renderTextVAO);

		if (text.second->behaviour)
		{
			text.second->behaviour();
		}

		// Needs to be optimized to batch and draw string by string, likely
		std::vector<RenderCharVertex> currentRenderCharVertVec;
		currentRenderCharVertVec.reserve(6);
		glm::vec3 pos = text.second->position;

		for (auto c : text.second->text)
		{
			const LGLStructs::GlyphTexture& glyph = text.second->glyphInfo.glyphs.at(c);

			float xpos = pos.x + glyph.bitmap_left * pos.z;
			float ypos = pos.y - (glyph.height - glyph.bitmap_top) * pos.z;
			float wpos = glyph.width * pos.z;
			float hpos = glyph.height * pos.z;

			currentRenderCharVertVec.push_back({{ xpos, ypos + hpos        }, { 0.0f, 0.0f }});
			currentRenderCharVertVec.push_back({{ xpos, ypos               }, { 0.0f, 1.0f }});
			currentRenderCharVertVec.push_back({{ xpos + wpos, ypos        }, { 1.0f, 1.0f }});
			currentRenderCharVertVec.push_back({{ xpos, ypos + hpos        }, { 0.0f, 0.0f }});
			currentRenderCharVertVec.push_back({{ xpos + wpos, ypos        }, { 1.0f, 1.0f }});
			currentRenderCharVertVec.push_back({{ xpos + wpos, ypos + hpos }, { 1.0f, 0.0f }});           

			GLSafeExecute(glBindTexture, GL_TEXTURE_2D, collectionToCharTextures[text.second->glyphInfo.fontName][c]);

			GLSafeExecute(glBindBuffer, GL_ARRAY_BUFFER, renderTextVBO);

			GLSafeExecute(
				glBufferData,
				GL_ARRAY_BUFFER,
				currentRenderCharVertVec.size() * sizeof(RenderCharVertex),
				currentRenderCharVertVec.data(),
				GL_DYNAMIC_DRAW
			);
			GLSafeExecute(glDrawArrays, GL_TRIANGLES, 0, currentRenderCharVertVec.size());
			currentRenderCharVertVec.clear();

			pos.x += (glyph.advanceX >> 6);
		}
	}

	GLSafeExecute(glActiveTexture, GL_TEXTURE0);
	GLSafeExecute(glBindTexture, GL_TEXTURE_2D, 0);
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

	while (!(stopRendering || glfwWindowShouldClose(window)))
	{
		if (pauseRendering)
		{
			std::unique_lock<std::mutex> pauseLock(pauserMux);
			pauser.wait(pauseLock, [this]() { return !pauseRendering; });
		}

		ContextLock

		std::chrono::system_clock::time_point renderStartTime = std::chrono::system_clock::now();
		glfwSwapInterval(useVSync);

		ProcessInput();
		glfwPollEvents();

		GLSafeExecute(glClearColor, background.r, background.g, background.b, background.a);
		GLSafeExecute(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (additionalSteps)
		{
			additionalSteps();
		}

		for (auto& currentModelToProcess : internalModelMap)
		{
			SetCurrentShaderProg(currentModelToProcess.second.GetModelPtr()->shaderProgram);

			std::function<void()>& modelBeh = currentModelToProcess.second.GetModelPtr()->modelBehaviour;
			if (modelBeh)
			{
				modelBeh();
			}

			for (size_t meshIndex = 0; meshIndex < currentModelToProcess.second.VAOs.size(); ++meshIndex)
			{
				auto& currentVAO = currentModelToProcess.second.VAOs[meshIndex];

				if (currentVAO.meshInfo->render)
				{
					currentVAOToRender = currentVAO;

					SetCurrentShaderProg(currentVAO.meshInfo->shaderProgram);

					GLSafeExecute(glBindVertexArray, currentVAO.vboId);

					for (auto& texture : currentVAO.meshInfo->mesh.textures)
					{
						auto currentTextureIter = currentModelToProcess.second.textureIDs.find(texture.name);
						if (currentTextureIter != currentModelToProcess.second.textureIDs.end())
						{
							TextureID textureID = (*currentTextureIter).second;
							int convertedTextureType = static_cast<int>(texture.type);
							GLSafeExecute(glActiveTexture, GL_TEXTURE0 + convertedTextureType);
							GLSafeExecute(glBindTexture, GL_TEXTURE_2D, textureID);

							textureTypesToUnbind[convertedTextureType] = true;
						}
					}

					std::function<void(int)>& behaviourToCheck = currentVAO.meshInfo->behaviour;
					if (behaviourToCheck)
					{
						behaviourToCheck(static_cast<int>(meshIndex));
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

		RenderText();

		glfwSwapBuffers(window);

		renderDeltaTime = std::chrono::duration<float>(std::chrono::system_clock::now() - renderStartTime).count();
		if (renderTimeCallbackFunc)
		{
			renderTimeCallbackFunc(renderDeltaTime);
		}
	}

	stopRendering = true;
	DeleteGLObjects();
}

void LGL::StopRenderingCycle()
{
	stopRendering = true;
}

void LGL::PauseRendering(bool value)
{
	pauseRendering = value;

	if (!pauseRendering)
	{
		pauser.notify_one();
	}
}

void LGL::SetStaticBackgroundColor(const glm::vec4& rgba)
{
	background = rgba;
}

void LGL::CreateRenderTextVO()
{
	HandshakeContextLock

	GLSafeExecute(glGenVertexArrays, 1, &renderTextVAO);
	GLSafeExecute(glGenBuffers, 1, &renderTextVBO);
	GLSafeExecute(glBindVertexArray, renderTextVAO);
	GLSafeExecute(glBindBuffer, GL_ARRAY_BUFFER, renderTextVBO);
	GLSafeExecute(glBufferData, GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
	GLSafeExecute(glEnableVertexAttribArray, 0);
	GLSafeExecute(glVertexAttribPointer, 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
}

void LGL::CreateMesh(const std::string& modelName, MeshInfo& meshInfo)
{
	HandshakeContextLock

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

	if (internalModelMap.find(modelName) == internalModelMap.end())
	{
		assert(false && "Trying to add mesh to non existent model");
		return;
	}

	auto& newVAOInfo = internalModelMap[modelName];

	std::vector<size_t> steps = CollectSteps();

	newVAOInfo.VAOs.push_back({});
	VAO* newVAO = &newVAOInfo.VAOs.back().vboId;
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
		newVAOInfo.VAOs.back().useIndices = true;
		newVAOInfo.VAOs.back().pointAmount = meshInfo.mesh.indices.size();
	}
	else
	{
		newVAOInfo.VAOs.back().pointAmount = meshInfo.mesh.vert.size();
	}

	newVAOInfo.VAOs.back().meshInfo = &meshInfo;


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
			GLSafeExecute(glVertexAttribIPointer, i, static_cast<int>(steps[i]), GL_INT, stride, (void*)(byteOffset));
			byteOffset += steps[i] * sizeof(int);
		}
		else
		{
			GLSafeExecute(
				glVertexAttribPointer, i, static_cast<int>(steps[i]), GL_FLOAT, GL_FALSE, stride, (void*)(byteOffset)
			);
			byteOffset += steps[i] * sizeof(float);
		}
	}

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	size_t polygons = newVAOInfo.VAOs.back().pointAmount / 3;
	std::cout << "Mesh with " << newVAOInfo.VAOs.back().pointAmount << " point(s) / " << polygons << " polygons created\n";

	LoadAndCompileShader(meshInfo.shaderProgram);
	for (auto& texture : meshInfo.mesh.textures)
	{
		ConfigureTexture(modelName, texture);
	}
}

void LGL::CreateModel(const std::string& modelName, LGLStructs::ModelInfo& model)
{
	if (internalModelMap.find(modelName) == internalModelMap.end())
	{
		internalModelMap.emplace(modelName, InternalModelInfo{});
		internalModelMap[modelName].SetModelPtr(&model);

		for (auto& mesh : internalModelMap[modelName].GetModelPtr()->meshes)
		{
			CreateMesh(modelName, mesh);
		}
	}
}

void LGL::CreateModel(const std::string& modelName, std::weak_ptr<LGLStructs::ModelInfo> model)
{
	if (internalModelMap.find(modelName) == internalModelMap.end())
	{
		internalModelMap.emplace(modelName, InternalModelInfo{});
		internalModelMap[modelName].SetModelPtr(model);

		for (auto& mesh : internalModelMap[modelName].GetModelPtr()->meshes)
		{
			CreateMesh(modelName, mesh);
		}
	}
}

void LGL::CreateText(const std::string& textLabel, LGLStructs::TextInfo& text)
{
	if (!renderTextVOCreated)
	{
		CreateRenderTextVO();
		renderTextVOCreated = true;
	}
	LoadAndCompileShader(text.shaderProgram);
	if (collectionToCharTextures.find(text.glyphInfo.fontName) == collectionToCharTextures.end())
	{
		for (auto& charTexture : text.glyphInfo.glyphs)
		{
			ConfigueGlyphTexture(text.glyphInfo.fontName, charTexture.second);
		}
	}

	internalTextMap[textLabel] = &text;
}

void LGL::DeleteModel(const std::string& modelName)
{
	HandshakeContextLock

	if(internalModelMap.find(modelName) != internalModelMap.end())
	{
		GLSafeExecute(glBindVertexArray, 0);

		for (auto& VAO : internalModelMap[modelName].VAOs)
		{
			GLSafeExecute(glDeleteVertexArrays, 1, &VAO.vboId);
		}
		for (auto& texture : internalModelMap[modelName].textureIDs)
		{
			GLSafeExecute(glDeleteTextures, 1, &texture.second);
		}

		internalModelMap.erase(modelName);
	}
}

void LGL::DeleteText(const std::string& textLabel)
{
	HandshakeContextLock

	if (internalTextMap.find(textLabel) != internalTextMap.end())
	{
		internalTextMap.erase(textLabel);
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

bool LGL::CompileShader(ShaderType shaderType, const std::string& name)
{
	using AcceptableShaderCode = const char* const;

	HandshakeContextLock

	if (shaderInfoCollection.find(name) == shaderInfoCollection.end())
	{
		std::cerr << "Shader " + name + "not found\n";
		return false;
	}

	ShaderInfo& currentShaderInfo = shaderInfoCollection[name].second[shaderType];
	AcceptableShaderCode shaderToC = currentShaderInfo.shaderCode.c_str();

	ShaderID* newShader = &currentShaderInfo.shaderId;

	GLSafeExecute(glShaderSource, *newShader, 1, &shaderToC, nullptr);
	bool shaderCompiled = GLSafeExecute(glCompileShader, *newShader);

	return shaderCompiled;
}

bool LGL::LoadShaderFromFile(const std::string& name, const std::string& file, const std::string& shaderType)
{
	HandshakeContextLock

	ShaderType shaderTypeID = shaderTypeChoice[shaderType];
	std::string shader; // change to stringstream
	std::string line;

	std::ifstream reader(file);

	if (!reader)
	{
		std::cout << name + '.' + shaderType + " shader does not exist\n";
		return false;
	}

	while (std::getline(reader, line))
	{
		shader += (line + '\n');
	}

	shaderInfoCollection[name].second.emplace(
		shaderTypeID,
		ShaderInfo{
			GLSafeExecuteRet(glCreateShader, shaderTypeID),
			shader
		}
	);

	std::cout << "Shader " << name + '.' + shaderType << " loaded\n";

	return true;
}

LGL::ShaderProgramID LGL::SetCurrentShaderProg(const std::string& shaderProg)
{
	ShaderProgramID shaderProgID = ~ShaderProgramID{};
	if (shaderInfoCollection.find(shaderProg) != shaderInfoCollection.end())
	{
		shaderProgID = shaderInfoCollection[shaderProg].first;

		if (lastProgram != shaderProg)
		{
			lastProgram = shaderProg;
			GLSafeExecute(glUseProgram, shaderProgID);
		}
	}

	return shaderProgID;
}

bool LGL::ConfigureTextureImpl(TextureID& newTextureID, const Texture& texture)
{
	HandshakeContextLock

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
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

	GLSafeExecute(glPixelStorei, GL_UNPACK_ALIGNMENT, textureFormat != GL_RGBA ? 1 : 4);

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

bool LGL::ConfigureTexture(const std::string& modelName, const LGLStructs::Texture& texture)
{
	if (internalModelMap[modelName].textureIDs.find(texture.name) != internalModelMap[modelName].textureIDs.end())
	{
		return true;
	}

	internalModelMap[modelName].textureIDs[texture.name] = TextureID();

	TextureID& newTextureID = internalModelMap[modelName].textureIDs[texture.name];

	return ConfigureTextureImpl(newTextureID, texture);
}

bool LGL::ConfigueGlyphTexture(const std::string& collectionName, const LGLStructs::GlyphTexture& glyphTexture)
{
	if (collectionToCharTextures.find(collectionName) == collectionToCharTextures.end())
	{
		collectionToCharTextures[collectionName] = {};
	}

	TextureID& currentRenderCharTexture = collectionToCharTextures[collectionName][glyphTexture.c];

	return ConfigureTextureImpl(currentRenderCharTexture, glyphTexture);
}

bool LGL::CreateShaderProgram(const std::string& name, const std::vector<std::string>& shaderNames)
{	
	HandshakeContextLock

	shaderInfoCollection[name].first = GLSafeExecuteRet(glCreateProgram);
	ShaderProgramID* newShaderProgram = &shaderInfoCollection[name].first;

	for (auto& shaderInfo : shaderInfoCollection[name].second)
	{
		GLSafeExecute(glAttachShader, *newShaderProgram, shaderInfo.second.shaderId);
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
	HandshakeContextLock

	DeleteShader(shaderName);

	shaderInfoCollection.erase(shaderName);

	LoadAndCompileShader(shaderName);
}

void LGL::DeleteShader(const std::string& shaderName)
{
	lastProgram.clear();
	if (uniformHasher)
	{
		uniformHasher->ResetHashesByShader(shaderInfoCollection[shaderName].first);
	}

	GLSafeExecute(glUseProgram, 0);

	for (auto& shaderInfo : shaderInfoCollection[shaderName].second)
	{
		GLSafeExecute(glDeleteShader, shaderInfo.second.shaderId);
	}

	GLSafeExecute(glDeleteProgram, shaderInfoCollection[shaderName].first);
}

void LGL::UpdateWindowSize(int width, int height)
{
	HandshakeContextLock

	windowWidth = width;
	windowHeight = height;

	GLSafeExecute(glViewport, 0, 0, width, height);
}

void LGL::ResetLGL()
{
	DeleteGLObjects();
}

bool LGL::LoadAndCompileShader(const std::string& name)
{
	if (shaderInfoCollection.find(name) != shaderInfoCollection.end())
	{
		return true;
	}

	shaderInfoCollection[name] = {};

	for (const auto& shaderFileType : shaderTypeChoice)
	{
		if (!LoadShaderFromFile(name, shaderPath + '\\' + name + '.' + shaderFileType.first, shaderFileType.first)) continue;
		if (!CompileShader(shaderFileType.second, name)) // remove if did not compile
		{
			shaderInfoCollection[name].second.erase(shaderFileType.second);
		}
	}

	return shaderInfoCollection[name].second.size() && CreateShaderProgram(name);
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

LGL* LGL::CheckAndGetInstanceByContext(GLFWwindow* window)
{
	if (contextToInstance.find(window) != contextToInstance.end() && contextToInstance[window])
	{
		return contextToInstance[window];
	}

	return nullptr;
}

void LGL::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	LGL* instance = CheckAndGetInstanceByContext(window);

	if (instance && instance->cursorPositionFunc)
	{
		instance->cursorPositionFunc(xpos, ypos);
	}
}

void LGL::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	LGL* instance = CheckAndGetInstanceByContext(window);

	if (instance && instance->scrollCallbackFunc)
	{
		instance->scrollCallbackFunc(xoffset, yoffset);
	}
}

void LGL::KeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	LGL* instance = CheckAndGetInstanceByContext(window);

	if (instance && instance->keyPressCallbackFunc)
	{
		instance->keyPressCallbackFunc(key, scancode, action, mods);
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

void LGL::EnableUniformValueBatchSending(bool value)
{
	batchUniformVals = value;
}

void LGL::EnableUniformValueHashing(bool value)
{
	hashUniformVals = value;
}

int LGL::CheckUniformValueLocation(
	const std::string& valueName, 
	const std::string& shaderProgramName, 
	ShaderProgramID& shaderProgramID
)
{
	const std::string& shaderProgramNameToUse = shaderProgramName == "" ? lastProgram : shaderProgramName;

	if ((shaderProgramID = SetCurrentShaderProg(shaderProgramNameToUse)) != ~ShaderProgramID{})
	{
		int uniformValueLocation = GLSafeExecuteRet(glGetUniformLocation, shaderProgramID, valueName.c_str());

		if (uniformValueLocation == -1)
		{
			if (std::find(uniformErrorAntispam.begin(), uniformErrorAntispam.end(), valueName) == std::end(uniformErrorAntispam))
			{
				std::cerr << "[ERROR] Shader value " + valueName << " could not be located\n";
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
	ShaderProgramID shaderProgramIDToUse = 0;
	int uniformValueLocation = CheckUniformValueLocation(valueName, shaderProgramName, shaderProgramIDToUse);

	if (uniformValueLocation == -1 || lastProgram.empty())
	{
		return false;
	}

	if (!batchUniformVals || uniformLocationTracker.find(uniformValueLocation) != uniformLocationTracker.end())
	{
		Render();
	}

	if (!hashUniformVals || 
		(uniformHasher && uniformHasher->CheckIfDiffersAndHashValue(shaderProgramIDToUse, uniformValueLocation, value)))
	{
		uniformLocationTracker.insert(uniformValueLocation);
		uniformValueLocators.at(typeid(Type))(uniformValueLocation, &value);
	}

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