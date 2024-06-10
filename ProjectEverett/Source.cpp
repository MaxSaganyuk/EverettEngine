#include <time.h>
#include <cmath>
#include <cstdlib>

#include "LGL.h"
#include "Verts.h"
#include "MaterialSim.h"
#include "LightSim.h"
#include "CameraSim.h"

constexpr int windowHeight = 800;
constexpr int windowWidth = 600;

std::vector<glm::vec3> GenerateRandomCubes(int amount)
{
	auto gen = []() -> float
	{
		return (float)(rand() % 200 - 100) / 10;
	};

	std::vector<glm::vec3> vec;

	for (int i = 0; i < amount; ++i)
	{
		vec.push_back(glm::vec3(gen(), gen(), gen()));
	
	}

	return vec;
}

std::vector<LGL::Vertex> ConvertAVerySpecificFloatPointerToVertexVector(float* ptr, size_t size)
{
	std::vector<LGL::Vertex> vert;

	for (int i = 0; i < size / sizeof(float) / 8; ++i)
	{
		vert.push_back(
			LGL::Vertex{
			{ ptr[i * 8], ptr[i * 8 + 1], ptr[i * 8 + 2] },
			{ ptr[i * 8 + 6], ptr[i * 8 + 7] },
			{ ptr[i * 8 + 3], ptr[i * 8 + 4], ptr[i * 8 + 5]}
			}
		);
	}

	return vert;
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	LGL::InitOpenGL(3, 3);

	LGL lgl;

	lgl.CreateWindow(windowHeight, windowWidth, "ProjectEverett");
	lgl.InitGLAD();
	lgl.InitCallbacks();

	std::vector<glm::vec3> cubesPos = GenerateRandomCubes(20);
	std::vector<glm::vec3> lightsPos = GenerateRandomCubes(1);

	lgl.LoadShaderFromFile("lightComb.vert");
	lgl.CompileShader();

	lgl.LoadShaderFromFile("lightComb.frag");
	lgl.CompileShader();

	lgl.CreateShaderProgram("colorsTex");

	lgl.LoadShaderFromFile("lamp.vert");
	lgl.CompileShader();

	lgl.LoadShaderFromFile("lamp.frag");
	lgl.CompileShader();

	lgl.CreateShaderProgram("lamp");

	lgl.LoadTextureFromFile("box.png");
	lgl.ConfigureTexture();

	lgl.LoadTextureFromFile("boxEdge.png");
	lgl.ConfigureTexture();

	auto rotateG = [&lgl, &cubesPos](float ia, float ib)
	{
		static float a = 0;
		static float b = 0;

		a += ia;
		b += ib;

		for (int i = 0; i < cubesPos.size(); ++i)
		{
			glm::mat4 trans = glm::mat4(1.0);
			trans = glm::translate(trans, cubesPos[i]);
			if (!i)
			{
				trans = glm::rotate(trans, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				trans = glm::rotate(trans, (float)sin(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
			}
			trans = glm::rotate(trans, glm::radians(a / 25), glm::vec3(-1.0, 0.0f, 0.0f));

			lgl.SetShaderUniformValue("model", trans, true);
		}
	};

	auto rotateUp = [&rotateG]()
	{
		rotateG(1, 0);
	};

	auto rotateLeft = [&rotateG]()
	{
		rotateG(0, 1);
	};

	auto rotateNo = [&rotateG]()
	{
		rotateG(0, 0);
	};

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	CameraSim camera(windowHeight, windowWidth);
	camera.SetMode(CameraSim::Mode::Walk);

	MaterialSim::Material mat = MaterialSim::GetMaterial(MaterialSim::MaterialID::GOLD);
	LightSim::Attenuation atte = LightSim::GetAttenuation(60);

	std::vector<std::pair<std::string, std::deque<std::string>>> lightShaderValueNames
	{
		{"material", { "diffuse", "specular", "shininess" }},
		{"pointLight",
			{
				"position", "ambient", "diffuse",
				"specular", "constant", "linear",
				"quadratic"
			}
		},
		{"spotLight",
			{
				"position", "direction", "ambient",
				"diffuse", "specular", "constant",
				"linear", "quadratic", "cutOff",
				"outerCutOff"
			}
		}
	};

	auto lightBeh = [&lgl, &cubesPos, &lightsPos, &mat, &atte, &camera, &lightShaderValueNames]()
	{
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());

		lgl.SetShaderUniformStruct(lightShaderValueNames[0].first, lightShaderValueNames[0].second, 0, 1, mat.shininess);

		//lgl.SetShaderUniformValue("dirLight.direction", lightPos);
		//lgl.SetShaderUniformValue("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		//lgl.SetShaderUniformValue("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		//lgl.SetShaderUniformValue("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

		lgl.SetShaderUniformValue("lightAmount", static_cast<int>(lightsPos.size()));
		for (int i = 0; i < lightsPos.size(); ++i)
		{
			std::string accessIndex = lightShaderValueNames[1].first + "[" + std::to_string(i) + "]";

			lgl.SetShaderUniformStruct(
				accessIndex,
				lightShaderValueNames[1].second,
				lightsPos[i], glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f),
				glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, atte.linear,
				atte.quadratic
			);
		}

		lgl.SetShaderUniformStruct(
			lightShaderValueNames[2].first,
			lightShaderValueNames[2].second,
			camera.GetPositionVectorAddr(), camera.GetFrontVectorAddr(), glm::vec3(0.2f, 0.2f, 0.2f),
			glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f,
			atte.linear, atte.quadratic, glm::cos(glm::radians(12.5f)),
			glm::cos(glm::radians(17.5f))
		);

		lgl.SetShaderUniformValue("viewPos", camera.GetPositionVectorAddr());

		for (auto& cubePos : cubesPos)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePos);
			model = glm::scale(model, glm::vec3(1.0f));
			//model = glm::rotate(model, (float)sin(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
			lgl.SetShaderUniformValue("model", model);
			lgl.SetShaderUniformValue("inv", glm::inverse(model), true);
		}
	};

	auto lampBeh = [&lgl, &camera, &lightsPos, &lightColor]()
	{
		lgl.SetShaderUniformValue("lightColor", lightColor);

		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		for (int i = 0; i < lightsPos.size(); ++i)
		{
			lightsPos[i].x = static_cast<float>(sin(glfwGetTime() + i * 230));
			lightsPos[i].y = static_cast<float>(cos(glfwGetTime() - i * 230));

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, lightsPos[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			//model = glm::rotate(model, (float)sin(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
			lgl.SetShaderUniformValue("model", model, true);
		}
	};
	/*
	//glfw.CreatePolygon(vert, sizeof(vert), false);

	lgl.CreateMesh(
		{
			cube,
			sizeof(cube),
			cubeSteps,
			sizeof(cubeSteps) / sizeof(int),
			nullptr,
			0,
			false,
			"lamp",
			lampBeh
		}
	);
	*/
	std::vector<LGL::Vertex> cubeV;
	std::vector<unsigned int> cubeInd;

	std::vector<LGL::Vertex> a = ConvertAVerySpecificFloatPointerToVertexVector(vertNT, sizeof(vertNT));


	lgl.GetMeshFromFile("sphere.obj", cubeV, cubeInd);

	lgl.CreateMesh(
		{
			a,
			{},
			false,
			"colorsTex",
			{"box.png", "boxEdge.png"},
			lightBeh
		}
	);

	lgl.CreateMesh(
		{
			cubeV,
			cubeInd,
			false,
			"lamp",
		    {},
			lampBeh
		}
	);

	lgl.SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });

	lgl.SetCursorPositionCallback(
		[&camera](double xpos, double ypos) { camera.Rotate(static_cast<float>(xpos), static_cast<float>(ypos)); }
	);
	lgl.SetScrollCallback(
		[&camera](double xpos, double ypos) { camera.Zoom(static_cast<float>(xpos), static_cast<float>(ypos)); }
	);

	lgl.SetInteractable(GLFW_KEY_W, [&camera]() { camera.SetPosition(CameraSim::Direction::Forward); });
	lgl.SetInteractable(GLFW_KEY_S, [&camera]() { camera.SetPosition(CameraSim::Direction::Backward); });
	lgl.SetInteractable(GLFW_KEY_A, [&camera]() { camera.SetPosition(CameraSim::Direction::Left); });
	lgl.SetInteractable(GLFW_KEY_D, [&camera]() { camera.SetPosition(CameraSim::Direction::Right); });

	lgl.GetMaxAmountOfVertexAttr();
	lgl.CaptureMouse();


	/*
	LGL other;
	other.CreateWindow(windowHeight, windowWidth, "Other");
	other.InitCallbacks();
	other.SetStaticBackgroundColor({ 1.0f, 0.0f, 0.0f, 0.0f });

	std::thread otherThread([&other]() { other.RunRenderingCycle(); });

	*/

	lgl.RunRenderingCycle(
		[&lgl, &camera]() 
		{ 
			camera.SetPosition(CameraSim::Direction::Nowhere);
			lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
			lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		}
	);

	LGL::TerminateOpenGL();

	//otherThread.join();
}