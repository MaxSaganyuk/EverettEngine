#include <iostream>
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

	std::vector<LGLStructs::Vertex> ConvertAVerySpecificFloatPointerToVertexVector(float* ptr, size_t size)
	{
		std::vector<LGLStructs::Vertex> vert;

		for (int i = 0; i < size / sizeof(float) / 8; ++i)
		{
			vert.push_back(
				LGLStructs::Vertex{
					{ ptr[i * 8 + 0], ptr[i * 8 + 1], ptr[i * 8 + 2] },
					{ ptr[i * 8 + 3], ptr[i * 8 + 4], ptr[i * 8 + 5] },
					{ ptr[i * 8 + 6], ptr[i * 8 + 7], 0.0f           }
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
			avalible = maze.matrix[pickedN][pickedM];
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
	std::vector<std::string> extraTextureNames;

	FileLoader::GetFilesInDir(textureNames, "textures");
	FileLoader::GetFilesInDir(extraTextureNames, "extraStuff\\textures");
	lgl.LoadAndConfigureTextures("textures\\", textureNames);
	lgl.LoadAndConfigureTextures("extraStuff\\textures\\", extraTextureNames);

	MaterialSim::Material mat = MaterialSim::GetMaterial(MaterialSim::MaterialID::GOLD);
	LightSim::Attenuation atte = LightSim::GetAttenuation(60);

	MazeGen::MazeInfo maze = MazeGen::GenerateMaze(20, 20);
	MazeGen::PrintExitPath(maze);

	CameraSim camera(windowHeight, windowWidth, {0.0f, 0.0f, 3.0f});
	camera.SetMode(CameraSim::Mode::Fly);

	std::vector<glm::vec3> cubesPos = GeneralHelpers::PlaceCubesInAMaze(maze);
	std::vector<glm::vec3> lightsPos = { {0.0f, 0.0f, 0.0f} };

	std::vector<SolidSim> cubes = GeneralHelpers::ConvertPosesToSolids(cubesPos);
	std::vector<SolidSim> lights = { SolidSim(camera.GetPositionVectorAddr(), {0.005f, 0.005f, 0.005f})};
	lights.begin()->SetType(SolidSim::SolidType::Dynamic);

	SolidSim coilSim({ 0.0f, 0.0f, 0.0f }, {0.2f, 0.2f, 0.2f});

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

	auto lightBeh = [&lgl, &cubes, &lights, &mat, &atte, &camera, &lightShaderValueNames, &coilSim]()
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
		
	};

	auto cubeBeh = [&lgl, &camera, &cubes]()
	{
		for (auto& cube : cubes)
		{
			glm::mat4& model = cube.GetModelMatrixAddr();
			lgl.SetShaderUniformValue("model", model);
			lgl.SetShaderUniformValue("inv", glm::inverse(model), true);

			if (SolidSim::CheckForCollision(camera, cube))
			{
				camera.SetLastPosition();
			}
		}
	};

	auto coilBeh = [&lgl, &lightBeh, &coilSim]()
	{
		lightBeh();
		lgl.SetShaderUniformValue("model", coilSim.GetModelMatrixAddr());
		lgl.SetShaderUniformValue("inv", glm::inverse(coilSim.GetModelMatrixAddr()), true);
	};

	auto lampBeh = [&lgl, &camera, &lights, &lightColor]()
	{
		lgl.SetShaderUniformValue("lightColor", lightColor);

		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		for (auto& light : lights)
		{
			light.SetPosition(SolidSim::Direction::Nowhere);
			light.Rotate({ 0.0f, 0.005f, 0.0f });
			light.SetPosition(SolidSim::Direction::Forward);
			lgl.SetShaderUniformValue("model", light.GetModelMatrixAddr());;
		}
	};

	/*
	std::vector<LGLStructs::Vertex> cubeV = GeneralHelpers::ConvertAVerySpecificFloatPointerToVertexVector(vertNT, sizeof(vertNT));
	std::vector<unsigned int> cubeInd;

	LGLStructs::ModelInfo cubeModel;

	cubeModel.behaviour = lightBeh;
	cubeModel.shaderProgram = "lightComb";
	cubeModel.AddMesh({ cubeV, cubeInd });
	cubeModel.meshes[0].textures = c
	lgl.CreateModel(cubeModel);

	*/
	LGLStructs::ModelInfo coil;

	lgl.GetModelFromFile("extraStuff\\coilHead.obj", coil);
	coil.shaderProgram = "lightComb";
	coil.behaviour = coilBeh;
	lgl.CreateModel(coil);

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
	lgl.SetInteractable(GLFW_KEY_R, [&camera]() { camera.SetPosition(CameraSim::Direction::Up); });

	lgl.GetMaxAmountOfVertexAttr();
	lgl.CaptureMouse();

	lgl.RunRenderingCycle(
		[&lgl, &camera]() {		
			camera.SetPosition(CameraSim::Direction::Nowhere);
			lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
			lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
		}
	);

	LGL::TerminateOpenGL();
}