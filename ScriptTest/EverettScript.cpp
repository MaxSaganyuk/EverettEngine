#include "EverettScript.h"
#include "ColorManager.h"

#include <iostream>
#include <random>

IEverettEngine* engineInter{};
ICameraSim* cameraSim{};
IColliderSim* hardLinkTest{};

bool stopSwitchColors{};
size_t amountOfTimesColorChanged{};
ILightSim* light{};

static void ShaderSwitch()
{
	constexpr auto ambientShaderCodeSection = IEverettEngine::ShaderCodeSection::AmbientLight;
	constexpr static const char* grayAmbientShader = R"(
		vec3 tex = vec3(texture(material.diffuse, TexCoords));
		const vec3 lum = vec3(0.299, 0.587, 0.114);
		float gray = dot(tex, lum);

		return vec3(gray);
	)";

	engineInter->SetShaderCodeSectionTo(
		ambientShaderCodeSection, 
		engineInter->IsCustomShaderCodeSectionFor(ambientShaderCodeSection) ? "" : grayAmbientShader
	);
}

glm::vec3 GetRandomColor()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> distrib(0, 255);

	return ColorManager::RGBVal{ 
		static_cast<std::uint8_t>(distrib(rd)), 
		static_cast<std::uint8_t>(distrib(rd)), 
		static_cast<std::uint8_t>(distrib(rd)) 
	};
}

class TestCharHolder
{
	ISolidSim* testCharSolid{};
	IColliderSim* testCharCollider{};
	ISolidSim* satelliteBox{};
	ILightSim* satelliteBoxLight{};
	bool linkedToCamera{};
	bool toRotate{};
	size_t directionAmount{}; // Interactables run on single thread

	void SwitchLinkedMovementFor(IObjectSim& obj, bool value)
	{
		obj.EnableLinkedExecutionForFunc(IObjectSim::LinkableFuncNames::MoveByAxis, value);
		obj.EnableLinkedExecutionForFunc(IObjectSim::LinkableFuncNames::SetLastPosition, value);
	}

	void TriggerSwitchLinkedMovement(bool value)
	{
		SwitchLinkedMovementFor(*satelliteBox, value);
		SwitchLinkedMovementFor(*cameraSim, value);
	}

public:
	void SetSatelliteBox(ISolidSim* satelliteBox, ILightSim* satelliteBoxLight)
	{
		this->satelliteBox = satelliteBox;
		this->satelliteBoxLight = satelliteBoxLight;

		satelliteBoxLight->SetPositionVector(*satelliteBox);
		satelliteBoxLight->GetColorVectorAddr() = ColorManager::GetColorVec3(ColorManager::Colors::MAGENTA);

		testCharSolid->LinkObject(*satelliteBox).LinkObject(*satelliteBoxLight);
	}

	void RevolveAroundChar()
	{
		satelliteBox->RevolveAround(IObjectSim::RotationDegrees{ 0.0f, 1.0f, 0.0f }, *testCharSolid);
		satelliteBox->LookAt(*testCharSolid);
	}

	void SetSolidSim(ISolidSim* testCharSolid)
	{
		this->testCharSolid = testCharSolid;
	}

	void SetColliderSim(IColliderSim* testCharCollider)
	{
		this->testCharCollider = testCharCollider;
		testCharSolid->LinkObject(*testCharCollider);
	}

	void SetupBlockCollision(IColliderSim* blockCollider)
	{
		testCharCollider->AddCollisionCallback({{
			.startFunc   = [this]() { TriggerSwitchLinkedMovement(false); },
			.processFunc = [this]() { testCharSolid->SetLastPosition(); },
			.stopFunc    = [this]() {
				TriggerSwitchLinkedMovement(true);
				testCharSolid->SetLastPosition();
			},
		}, blockCollider
		});
	}

	void SetupWorldSwitchCollision(IColliderSim* worldSwitchCollider)
	{
		testCharCollider->AddCollisionCallback(
			{ { []() { engineInter->RequestWorldLoad("E:\\WorldSwitchTest.esav"); } }, worldSwitchCollider }
		);
	}

	void LinkCharToCamera(ICameraSim& camera)
	{
		linkedToCamera ? testCharSolid->UnlinkObject(camera) : static_cast<void>(testCharSolid->LinkObject(camera));

		linkedToCamera = !linkedToCamera;
		std::cout << "Camera link to char" << linkedToCamera ? " on\n" : " off\n";
	}

	void AddDirection()
	{
		++directionAmount;
		toRotate = true;

		testCharSolid->PlayModelAnimation(true);
	}

	void Go(float rotation)
	{
		if (toRotate)
		{
			toRotate = false;
			testCharSolid->SetOrientation(glm::angleAxis(rotation, IObjectSim::worldUp), false);
		}

		testCharSolid->MoveByAxis({ glm::sin(rotation), 0.0f, glm::cos(rotation) });
	}

	void Stop()
	{
		toRotate = true;

		if (!(--directionAmount))
		{
			testCharSolid->StopModelAnimation();
		}
	}

	void Reset()
	{
		testCharSolid = nullptr;
		testCharCollider = nullptr;
	}
};

TestCharHolder testChar;

ScriptInit()
{
	cameraSim = engine.GetCameraInterface();

	hardLinkTest = engine.GetColliderInterface("HardLinkTest");

	testChar.SetSolidSim(engine.GetSolidInterface("TestChar"));
	testChar.SetSatelliteBox(engine.GetSolidInterface("RevolveBox"), engine.GetLightInterface("RevolveBoxLight"));
	testChar.SetColliderSim(engine.GetColliderInterface("TestCharBox"));
	testChar.SetupBlockCollision(engine.GetColliderInterface("BlockBox"));

	// Delibirate hintless interface get test
	testChar.SetupWorldSwitchCollision(dynamic_cast<IColliderSim*>(engine.GetObjectInterface("WorldSwitchBox")));

	light = engine.GetLightInterface("Spot");

	auto testCharAddDir   = []() { testChar.AddDirection(); };
	auto testCharStopFunc = []() { testChar.Stop(); };

	engine.AddInteractable(
		engine.ConvertKeyTo('I'), { testCharAddDir, []() { testChar.Go(0.0f); }, testCharStopFunc }
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('J'), { testCharAddDir, []() { testChar.Go(glm::pi<float>() / 2.0f); }, testCharStopFunc }
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('K'), { testCharAddDir, []() { testChar.Go(glm::pi<float>()); }, testCharStopFunc }
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('L'), { testCharAddDir, []() { testChar.Go(-glm::pi<float>() / 2.0f); }, testCharStopFunc }
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('C'), { []() { testChar.LinkCharToCamera(*cameraSim); } }
	);

	engine.AddInteractable(
		engine.ConvertKeyTo('R'), 
		{ .processFunc = []() { hardLinkTest->Rotate(IObjectSim::RotationDegrees{ 0.0f, 1.0f, 0.0f }, false); } }
	);

	engine.AddInteractable(engine.ConvertKeyTo("Space"), { ShaderSwitch });

	engine.AddMouseScrollCallback([](double value) { cameraSim->Zoom(static_cast<float>(value)); });

	// Counted timed callback test
	engine.AddTimedCallback(
		{ std::chrono::seconds(1), [](size_t count) { std::cout << "TimerTest " << count << '\n'; }, 3 }
	);
	
	// End triggered timed callback test
	engine.AddInteractable(
		engine.ConvertKeyTo('M'), {
			[]() {
				if (!stopSwitchColors)
				{
					stopSwitchColors = true;
					std::cout << "Stopped color switch. Switches happened " << amountOfTimesColorChanged << " times\n";
				}
			}
		}
	);
	engine.AddTimedCallback(
		{ 
			std::chrono::seconds(1), 
			[](size_t count) { light->GetColorVectorAddr() = GetRandomColor(); amountOfTimesColorChanged = count; }, 
			std::nullopt, 
			stopSwitchColors 
		}
	);

	engineInter = &engine;
}

MainScriptLoop()
{
	testChar.RevolveAroundChar();
}

ScriptCleanUp()
{
	engineInter = nullptr;
	cameraSim = nullptr;
	testChar.Reset();
}