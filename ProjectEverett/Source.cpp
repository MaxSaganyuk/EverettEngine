#include <time.h>
#include <cmath>
#include <cstdlib>

#include "LGL.h"
#include "Verts.h"
#include "MaterialSim.h"
#include "LightSim.h"

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
	srand(time(nullptr));

	LGL lgl(3, 3);

	lgl.CreateWindow(windowHeight, windowWidth, "ProjectEverett");
	lgl.InitGLAD();
	lgl.InitCallbacks();

	/*
	std::vector<glm::vec3> cubePos{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.0f, 2.5f, 3.0f)
	};

	*/

	std::vector<glm::vec3> cubesPos = GenerateRandomCubes(20);
	std::vector<glm::vec3> lightsPos = GenerateRandomCubes(5);

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

	// view matrix
	glm::mat4 view = glm::mat4(1.0f);

	glm::mat4 projection;

	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	glm::vec3 cubePos(0.0f, 0.0f, 0.0f);

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	MaterialSim::Material mat = MaterialSim::GetMaterial(MaterialSim::MaterialID::GOLD);
	LightSim::Attenuation atte = LightSim::GetAttenuation(60);

	auto moveCubeABit = [&lgl, &cubePos]()
	{
		static double lastTime = glfwGetTime();
		if (glfwGetTime() - lastTime > 1)
		{
			++cubePos.x;
			lastTime = glfwGetTime();
		}
	};

	auto lightBeh = [&lgl, &cubesPos, &view, &projection, &lightPos, &lightsPos, &cameraPos, &cameraFront, &mat, &atte]()
	{
		lgl.SetShaderUniformValue("proj", projection);
		lgl.SetShaderUniformValue("view", view);

		lgl.SetShaderUniformValue("material.diffuse", 0);
		lgl.SetShaderUniformValue("material.specular", 1);
		lgl.SetShaderUniformValue("material.shininess", mat.shininess);

		//lgl.SetShaderUniformValue("dirLight.direction", lightPos);
		//lgl.SetShaderUniformValue("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		//lgl.SetShaderUniformValue("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		//lgl.SetShaderUniformValue("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

		lgl.SetShaderUniformValue("lightAmount", static_cast<int>(lightsPos.size()));
		for (int i = 0; i < lightsPos.size(); ++i)
		{
			std::string accessIndex = "pointLight[" + std::to_string(i) + "]";
			lgl.SetShaderUniformValue(accessIndex + ".position", lightsPos[i]);
			lgl.SetShaderUniformValue(accessIndex + ".ambient", glm::vec3(0.05f, 0.05f, 0.05f));
			lgl.SetShaderUniformValue(accessIndex + ".diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
			lgl.SetShaderUniformValue(accessIndex + ".specular", glm::vec3(1.0f, 1.0f, 1.0f));
			lgl.SetShaderUniformValue(accessIndex + ".constant", 1.0f);
			lgl.SetShaderUniformValue(accessIndex + ".linear", atte.linear);
			lgl.SetShaderUniformValue(accessIndex + ".quadratic", atte.quadratic);
		}


		lgl.SetShaderUniformValue("spotLight.position", cameraPos);
		lgl.SetShaderUniformValue("spotLight.direction", cameraFront);
		lgl.SetShaderUniformValue("spotLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		lgl.SetShaderUniformValue("spotLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		lgl.SetShaderUniformValue("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lgl.SetShaderUniformValue("spotLight.constant", 1.0f);
		lgl.SetShaderUniformValue("spotLight.linear", atte.linear);
		lgl.SetShaderUniformValue("spotLight.quadratic", atte.quadratic);
		lgl.SetShaderUniformValue("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		lgl.SetShaderUniformValue("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

		lgl.SetShaderUniformValue("viewPos", cameraPos);

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

	auto lampBeh = [&lgl, &view, &projection, &lightPos, &lightsPos, &lightColor]()
	{
		lgl.SetShaderUniformValue("lightColor", lightColor);

		lgl.SetShaderUniformValue("view", view);
		lgl.SetShaderUniformValue("proj", projection);
		for (int i = 0; i < lightsPos.size(); ++i)
		{
			lightsPos[i].x = sin(glfwGetTime() + i * 230);
			lightsPos[i].y = cos(glfwGetTime() - i * 230);

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

	bool changeRed = true;
	bool changeGreen = true;
	bool changeBlue = true;

	auto smoothColorChange = [&lgl, &changeRed, &changeGreen, &changeBlue]()
	{
		glm::vec4 coords{
			changeRed   ? sin(glfwGetTime())      / 2.0f + 0.5f : 0.0f,
			changeGreen ? sin(glfwGetTime() + 10) / 2.0f + 0.5f : 0.0f,
			changeBlue  ? sin(glfwGetTime() + 20) / 2.0f + 0.5f : 0.0f,
			1.0f
		};

		lgl.SetShaderUniformValue("ourColor", coords);
	};

	glm::vec3 v { 1.0f, 0.0f, 0.0f };
	float fov = 45.0f;

	auto setPos = [&lgl, &cameraPos, &cameraFront, &fov, &projection, &view](int direction)
	{
		// model matrix
		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::rotate(model, glm::radians(50.f), glm::vec3(0.0f, 0.0f, 1.0f));
		const float cameraSpeed = 0.005f;

		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		switch (direction)
		{
		case 0:
			cameraPos += cameraSpeed * cameraFront;
			break;
		case 1:
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case 2:
			cameraPos -= cameraSpeed * glm::cross(cameraFront, cameraUp);
			break;
		case 3:
			cameraPos += cameraSpeed * glm::cross(cameraFront, cameraUp);
			break;
		default:
			break;
		}

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), static_cast<float>(windowHeight / windowWidth), 0.1f, 100.f);
		lgl.SetShaderUniformValue("view", view);
		lgl.SetShaderUniformValue("proj", projection);

		//glfw.SetShaderUniformValue("model", model);

	};

	auto moveNowhere = [&setPos]()
	{
		setPos(-1);
	};

	auto moveForward = [&setPos]()
	{
		setPos(0);
	};

	auto moveBackward = [&setPos]()
	{
		setPos(1);
	};

	auto moveLeft = [&setPos]()
	{
		setPos(2);
	};

	auto moveRight = [&setPos]()
	{
		setPos(3);
	};

	auto mouseMove = [&lgl, &cameraFront](double xpos, double ypos)
	{
		static float lastX = windowHeight;
		static float lastY = windowWidth;
		static float yaw = -90.0f;
		static float pitch = 0.0f;

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		constexpr float sens = 0.05f;
		xoffset *= sens;
		yoffset *= sens;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch >= 90.0f)
		{
			pitch = 90.0f;
		}
		else if (pitch <= -90.0f)
		{
			pitch = -90.0f;
		}

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);
	};

	auto scrollPOV = [&lgl, &fov](double xoffset, double yoffset)
	{
		if (fov + yoffset > 1.0f && fov - yoffset < 45.0f)
		{
			fov -= yoffset;
		}
		else if (fov <= 1.0f)
		{
			fov = 1.0f;
		}
		else if (fov >= 45.0f)
		{
			fov = 45.0f;
		}
	};

	lgl.SetCursorPositionCallback(mouseMove);
	lgl.SetScrollCallback(scrollPOV);

	lgl.SetInteractable(GLFW_KEY_E, moveCubeABit);

	lgl.SetInteractable(GLFW_KEY_W, moveForward);
	lgl.SetInteractable(GLFW_KEY_S, moveBackward);
	lgl.SetInteractable(GLFW_KEY_A, moveLeft);
	lgl.SetInteractable(GLFW_KEY_D, moveRight);

	lgl.GetMaxAmountOfVertexAttr();
	lgl.CaptureMouse();

	lgl.RunRenderingCycle(moveNowhere);

	//glfw.RunRenderingCycle();
}