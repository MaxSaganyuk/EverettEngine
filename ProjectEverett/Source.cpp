#include <iostream>
#include <time.h>
#include <cmath>
#include <cstdlib>

#include "LGL.h"
#include "LGLUtils.h"
#include "Verts.h"

#include "FileLoader.h"

#include "MaterialSim.h"
#include "LightSim.h"
#include "SolidSim.h"
#include "CameraSim.h"
#include "SoundSim.h"

#include "CommandHandler.h"

#include "MazeGen.h"

#include "stdEx/mapEx.h"

using SolidSimManager = std::unordered_map<std::string, std::vector<SolidSim>>;

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
	SoundSim::InitOpenAL();

	LGL lgl;
	lgl.SetAssetOnOpenGLFailure(false);

	lgl.CreateWindow(windowHeight, windowWidth, "ProjectEverett");

	FileLoader fileLoader;

	MaterialSim::Material mat = MaterialSim::GetMaterial(MaterialSim::MaterialID::GOLD);
	LightSim::Attenuation atte = LightSim::GetAttenuation(60);

	MazeGen::MazeInfo maze = MazeGen::GenerateMaze(20, 20);
	MazeGen::PrintExitPath(maze);

	CameraSim camera(windowHeight, windowWidth, {0.0f, 0.0f, 3.0f});
	camera.SetMode(CameraSim::Mode::Fly);

	SoundSim::SetCamera(camera);

	std::vector<glm::vec3> cubesPos = GeneralHelpers::PlaceCubesInAMaze(maze);
	std::vector<glm::vec3> lightsPos = { {0.0f, 0.0f, 0.0f} };
	
	SolidSimManager solids;

	solids["cube"]  = GeneralHelpers::ConvertPosesToSolids(cubesPos);
	solids["light"] = { SolidSim(camera.GetPositionVectorAddr(), {0.005f, 0.005f, 0.005f}) };
	solids.at("light").begin()->SetType(SolidSim::SolidType::Dynamic);
	solids["coilHead"] = { SolidSim({ 0.0f, 0.0f, 0.0f }, {0.4f, 0.4f, 0.4f}) };

	glm::vec3 lightColor(1.0f, 0.0f, 0.0f);

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
	
	auto lightBeh = [&lgl, &solids, &mat, &atte, &camera, &lightShaderValueNames]()
	{
			
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());

		LGLUtils::SetShaderUniformStruct(lgl, lightShaderValueNames[0].first, lightShaderValueNames[0].second, 0, 1, mat.shininess);

		std::vector<SolidSim>& lights = solids.at("light");
		lgl.SetShaderUniformValue("lightAmount", static_cast<int>(lights.size()));
		for (int i = 0; i < lights.size(); ++i)
		{
			LGLUtils::SetShaderUniformArrayAt(
				lgl,
				lightShaderValueNames[1].first,
				i,
				lightShaderValueNames[1].second,
				lights[i].GetPositionVectorAddr(), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f),
				glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, atte.linear,
				atte.quadratic
			);
		}

		LGLUtils::SetShaderUniformStruct(
			lgl,
			lightShaderValueNames[2].first,
			lightShaderValueNames[2].second,
			camera.GetPositionVectorAddr(), camera.GetFrontVectorAddr(), glm::vec3(0.2f, 0.2f, 0.2f),
			glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f,
			atte.linear, atte.quadratic, glm::cos(glm::radians(12.5f)),
			glm::cos(glm::radians(17.5f))
		);

		lgl.SetShaderUniformValue("viewPos", camera.GetPositionVectorAddr());
		
	};

	auto cubeBeh = [&lgl, &lightBeh, &camera, &solids]()
	{
		lightBeh();
		for (auto& cube : solids.at("cube"))
		{
			glm::mat4& model = cube.GetModelMatrixAddr();
			lgl.SetShaderUniformValue("model", model);
			lgl.SetShaderUniformValue("inv", glm::inverse(model));

			if (SolidSim::CheckForCollision(camera, cube))
			{
				camera.SetLastPosition();
			}
		}
	};

	auto coilBeh = [&lgl, &lightBeh, &solids]()
	{
		lightBeh();

		for (auto& coil : solids.at("coilHead"))
		{
			lgl.SetShaderUniformValue("model", coil.GetModelMatrixAddr());
			lgl.SetShaderUniformValue("inv", glm::inverse(coil.GetModelMatrixAddr()));
		}
	};

	auto lampBeh = [&lgl, &camera, &solids, &lightColor]()
	{
		lgl.SetShaderUniformValue("lightColor", lightColor);

		lgl.SetShaderUniformValue("view", camera.GetViewMatrixAddr());
		lgl.SetShaderUniformValue("proj", camera.GetProjectionMatrixAddr());
		for (auto& light : solids.at("light"))
		{
			light.SetPosition(SolidSim::Direction::Nowhere);
			light.Rotate({ 0.0f, 0.005f, 0.0f });
			light.SetPosition(SolidSim::Direction::Forward);
			lgl.SetShaderUniformValue("model", light.GetModelMatrixAddr());;
		}
	};

	std::vector<LGLStructs::Vertex> cubeV = GeneralHelpers::ConvertAVerySpecificFloatPointerToVertexVector(vertNT, sizeof(vertNT));
	std::vector<unsigned int> cubeInd;

	LGLStructs::ModelInfo cubeModel;

	cubeModel.behaviour = cubeBeh;
	cubeModel.shaderProgram = "lightComb";
	cubeModel.AddMesh({ cubeV, cubeInd });
	cubeModel.meshes.back().mesh.textures.push_back({});
	cubeModel.meshes.back().mesh.textures.back().type = LGLStructs::Texture::TextureType::Diffuse;
	fileLoader.LoadTexture("textures\\box.png", cubeModel.meshes.back().mesh.textures.back());
	cubeModel.meshes.back().mesh.textures.push_back({});
	cubeModel.meshes.back().mesh.textures.back().type = LGLStructs::Texture::TextureType::Specular;
	fileLoader.LoadTexture("textures\\boxEdge.png", cubeModel.meshes.back().mesh.textures.back());
	lgl.CreateModel(cubeModel);

	LGLStructs::ModelInfo coil;
	fileLoader.LoadModel("extraStuff\\n2.glb", coil);
	coil.shaderProgram = "lightComb";
	coil.behaviour = coilBeh;
	lgl.CreateModel(coil);
	coil.render = true;
	fileLoader.FreeTextureData();

	lgl.SetStaticBackgroundColor({ 0.0f, 0.0f, 0.0f, 0.0f });

	lgl.SetCursorPositionCallback(
		[&camera](double xpos, double ypos) { camera.Rotate(static_cast<float>(xpos), static_cast<float>(ypos));}
	);
	lgl.SetScrollCallback(
		[&camera](double xpos, double ypos) { camera.Zoom(static_cast<float>(xpos), static_cast<float>(ypos)); }
	);

	auto SpawnSolidCommand = [&camera, &solids](const std::string& arg)
	{
		glm::vec3 posToPlace = camera.GetPositionVectorAddr() + camera.GetFrontVectorAddr();
		try
		{
			std::vector<SolidSim>& exactSolids = solids.at(arg);
			exactSolids.push_back(posToPlace);
		}
		catch (std::out_of_range&)
		{
			std::cerr << "No solid named " + arg + '\n';
		}
	};

	auto GhostModeToggleCommand = [&camera](const std::string& arg)
	{
		camera.SetGhostMode(arg == "1");
	};

	CommandHandler commandHandler;
	commandHandler.AddCommandLambda("spawnSolid", SpawnSolidCommand);
	commandHandler.AddCommandLambda("ghostMode", GhostModeToggleCommand);

	std::string walkingDirections = "wsad";
	std::vector<SoundSim> walkingSounds;
	walkingSounds.reserve(walkingDirections.size());
	for (size_t i = 0; i < walkingDirections.size(); ++i)
	{
		walkingSounds.emplace_back(SoundSim("sounds//walk" + std::to_string(i) + ".wav", {0.0f, 0.0f, 0.0f}));
	}


	auto PlayWalkingSound = [&walkingSounds]()
	{
		bool isPlaying;

		for (size_t i = 0; i < walkingSounds.size(); ++i)
		{
			isPlaying = walkingSounds[i].IsPlaying();
			if (isPlaying) return;
		}

		walkingSounds[rand() % walkingSounds.size()].Play();
	};

	for (size_t i = 0; i < walkingDirections.size(); ++i)
	{
		lgl.SetInteractable(
			walkingDirections[i],
			[&camera, &PlayWalkingSound, i]() { PlayWalkingSound(); camera.SetPosition(static_cast<CameraSim::Direction>(i)); }
		);
	}

	lgl.SetInteractable('r', [&camera]() { camera.SetPosition(CameraSim::Direction::Up); });
	lgl.SetInteractable('c', [&commandHandler]() { commandHandler.RunCommandLine(); });

	lgl.GetMaxAmountOfVertexAttr();
	lgl.CaptureMouse();

	SoundSim coilSound("sounds//static.wav", solids.at("coilHead").back().GetPositionVectorAddr());

	lgl.RunRenderingCycle(
		[&lgl, &camera, &coilSound]() {
			coilSound.UpdatePositions();
			camera.SetPosition(CameraSim::Direction::Nowhere);
		}
	);
	
	LGL::TerminateOpenGL();
}