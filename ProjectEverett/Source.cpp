#include <time.h>
#include <cmath>
#include <cstdlib>

#include "LGL.h"
#include "Verts.h"

#include "FileLoader.h"

#include "MaterialSim.h"
#include "LightSim.h"
#include "SolidSim.h"
#include "CameraSim.h"

#include "MazeGen.h"

namespace GeneralHelpers
{

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

	std::vector<glm::vec3> PlaceCubesInAMaze(MazeGen::MazeInfo& maze)
	{
		std::vector<glm::vec3> cubesPos;

		for (size_t i = 0; i < maze.matrix.size(); ++i)
		{
			for (size_t j = 0; j < maze.matrix[i].size(); ++j)
			{
				if (!maze.matrix[i][j])
				{
					cubesPos.push_back({ i, 0, j });
				}
			}
		}

		return cubesPos;
	}

	std::vector<LGL::Vertex> ConvertAVerySpecificFloatPointerToVertexVector(float* ptr, size_t size)
	{
		std::vector<LGL::Vertex> vert;

		for (int i = 0; i < size / sizeof(float) / 8; ++i)
		{
			vert.push_back(
				LGL::Vertex{
					{ ptr[i * 8 + 0], ptr[i * 8 + 1], ptr[i * 8 + 2] },
					{ ptr[i * 8 + 6], ptr[i * 8 + 7]                 },
					{ ptr[i * 8 + 3], ptr[i * 8 + 4], ptr[i * 8 + 5] }
				}
			);
		}

		return vert;
	}

	std::vector<SolidSim> ConvertPosesToSolids(const std::vector<glm::vec3>& posVect)
	{
		std::vector<SolidSim> solidVect;

		for (const auto& pos : posVect)
		{
			solidVect.push_back(SolidSim(pos));
		}

		return solidVect;
	}

	glm::vec3 SelectRandomPlaceInAMaze(const MazeGen::MazeInfo& maze)
	{
		bool avalible = false;
		size_t sizeN = maze.matrix.size();
		size_t sizeM = maze.matrix[0].size();
		size_t pickedN = 0;
		size_t pickedM = 0;

		while (!avalible)
		{
			pickedN = rand() % sizeN;
			pickedM = rand() % sizeM;
			avalible = maze.matrix[pickedN][pickedN];
		}

		return { static_cast<float>(pickedN), 0, static_cast<float>(pickedM) };
	}
}

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	constexpr int windowHeight = 800;
	constexpr int windowWidth = 600;

	LGL::InitOpenGL(3, 3);

	LGL lgl;

	lgl.CreateWindow(windowHeight, windowWidth, "ProjectEverett");
	lgl.InitGLAD();
	lgl.InitCallbacks();

	std::vector<std::string> shaderNames { "lightComb", "lamp" };

	lgl.LoadAndCompileShaders("shaders\\", shaderNames);

	std::vector<std::string> textureNames;

	FileLoader::GetFilesInDir(textureNames, "textures");
	lgl.LoadAndConfigureTextures("textures\\", textureNames);

	MaterialSim::Material mat = MaterialSim::GetMaterial(MaterialSim::MaterialID::GOLD);
	LightSim::Attenuation atte = LightSim::GetAttenuation(60);

	MazeGen::MazeInfo maze = MazeGen::GenerateMaze(20, 20);
	MazeGen::PrintExitPath(maze);

	CameraSim camera(windowHeight, windowWidth, GeneralHelpers::SelectRandomPlaceInAMaze(maze));
	camera.SetMode(CameraSim::Mode::Fly);

	std::vector<glm::vec3> cubesPos = GeneralHelpers::PlaceCubesInAMaze(maze);
	std::vector<glm::vec3> lightsPos = { {0.0f, 0.0f, 0.0f} };

	std::vector<SolidSim> cubes = GeneralHelpers::ConvertPosesToSolids(cubesPos);
	std::vector<SolidSim> lights = { SolidSim({ 10.0f, 5.0f, 10.0f }, { 0.35f, 0.35f, 0.35f })};

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	std::vector<std::pair<std::string, std::vector<std::string>>> lightShaderValueNames
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

	auto lightBeh = [&lgl, &cubes, &lights, &mat, &atte, &camera, &lightShaderValueNames]()
	{
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());

		lgl.SetShaderUniformStruct(lightShaderValueNames[0].first, lightShaderValueNames[0].second, 0, 1, mat.shininess);

		lgl.SetShaderUniformValue("lightAmount", static_cast<int>(lights.size()));
		for (int i = 0; i < lights.size(); ++i)
		{
			std::string accessIndex = lightShaderValueNames[1].first + "[" + std::to_string(i) + "]";

			lgl.SetShaderUniformStruct(
				accessIndex,
				lightShaderValueNames[1].second,
				lights[i].GetPositionVectorAddr(), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f),
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

		for (auto& cube : cubes)
		{
			glm::mat4 model = glm::mat4(1.0f);
			glm::vec3 scale = cube.GetScaleVectorAddr();
			model = glm::translate(model, cube.GetPositionVectorAddr());
			model = glm::scale(model, scale);
			//model = glm::rotate(model, (float)sin(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
			lgl.SetShaderUniformValue("model", model);
			lgl.SetShaderUniformValue("inv", glm::inverse(model), true);

			if (SolidSim::CheckForCollision(camera, cube))
			{
				camera.SetLastPosition();
			}
			if (SolidSim::CheckForCollision(lights[0], cube))
			{
				lights[0].SetLastPosition();
			}
		}
	};

	auto lampBeh = [&lgl, &camera, &lights, &lightColor]()
	{
		lgl.SetShaderUniformValue("lightColor", lightColor);

		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		for (auto& light : lights)
		{
			glm::mat4 model = glm::mat4(1.0f);
			light.Rotate({1, 0, 0});
			model = glm::translate(model, light.GetPositionVectorAddr());
			model = glm::scale(model, glm::vec3(0.2f));
			light.SetPosition(SolidSim::Direction::Nowhere);
			lgl.SetShaderUniformValue("model", model, true);
		}
	};

	std::vector<LGL::Vertex> cubeV;
	std::vector<unsigned int> cubeInd;

	lgl.GetMeshFromFile("models\\cup.obj", cubeV, cubeInd);
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

	std::vector<LGL::Vertex> a = GeneralHelpers::ConvertAVerySpecificFloatPointerToVertexVector(vertNT, sizeof(vertNT));


	lgl.CreateMesh(
		{
			a,
			{},
			false,
			"lightComb",
			{"box.png", "boxEdge.png"},
			lightBeh
		}
	);



	lgl.SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });

	lgl.SetCursorPositionCallback(
		[&camera](double xpos, double ypos) { camera.Rotate(static_cast<float>(xpos), static_cast<float>(ypos));}
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

	lgl.RunRenderingCycle(
		[&lgl, &camera]() 
		{ 
			camera.SetPosition(CameraSim::Direction::Nowhere);
			lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
			lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		}
	);

	LGL::TerminateOpenGL();
}