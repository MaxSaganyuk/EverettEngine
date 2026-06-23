#include "EverettScript.h"
#include "ColorManager.h"

#include <iostream>
#include <random>

IEverettEngine* engineInter = nullptr;
ICameraSim* cameraSim = nullptr;

bool stopSwitchColors = false;
size_t amountOfTimesColorChanged{};
ILightSim* light = nullptr;

void CheckIfPtrValid(IObjectSim* ptr)
{
	if (!ptr)
	{
		std::cerr << "Invalid ptr. Cannot continue\n";
		std::terminate();
	}
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
	bool moving{};
	bool linkedToCamera{};

public:
	void Rotate(const IObjectSim::RotationDegrees& rotate)
	{
		testCharSolid->Rotate(rotate, false);
	}

	void SetSolidSim(ISolidSim* testCharSolid)
	{
		CheckIfPtrValid(testCharSolid);

		this->testCharSolid = testCharSolid;
		this->testCharSolid->EnableAutoModelUpdates();
	}

	void SetColliderSim(IColliderSim* testCharCollider)
	{
		CheckIfPtrValid(testCharCollider);

		this->testCharCollider = testCharCollider;
		testCharSolid->LinkObject(*testCharCollider);
	}

	void SetupBlockCollision(IColliderSim* blockCollider)
	{
		CheckIfPtrValid(blockCollider);

		testCharCollider->AddCollisionCallback(
			{ [this]() { testCharSolid->SetLastPosition(); }, nullptr, blockCollider, true }
		);
	}

	void SetupWorldSwitchCollision(IColliderSim* worldSwitchCollider)
	{
		CheckIfPtrValid(worldSwitchCollider);

		testCharCollider->AddCollisionCallback(
			{ []() { engineInter->RequestWorldLoad("E:\\WorldSwitchTest.esav"); }, nullptr, worldSwitchCollider, false }
		);
	}

	void LinkCharToCamera(ICameraSim& camera)
	{
		std::cout << "Camera link to char";

		linkedToCamera ? testCharSolid->UnlinkObject(camera) : testCharSolid->LinkObject(camera);

		linkedToCamera = !linkedToCamera;
		std::cout << linkedToCamera ? " on\n" : " off\n";
	}

	void Go(float rotation)
	{
		if (testCharSolid)
		{
			if (!moving)
			{
				testCharSolid->SetOrientation(glm::angleAxis(rotation, IObjectSim::worldUp), false);
				testCharSolid->PlayModelAnimation(true);
			}

			moving = true;
			testCharSolid->MoveByAxis({ glm::sin(rotation), 0.0f, glm::cos(rotation) });
		}
	}

	void Stop()
	{
		if (testCharSolid)
		{
			if (moving)
			{
				testCharSolid->StopModelAnimation();
			}

			moving = false;
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

	testChar.SetSolidSim(engine.GetSolidInterface("TestChar"));
	testChar.SetColliderSim(engine.GetColliderInterface("TestCharBox"));
	testChar.SetupBlockCollision(engine.GetColliderInterface("BlockBox"));

	// Delibirate hintless interface get test
	testChar.SetupWorldSwitchCollision(dynamic_cast<IColliderSim*>(engine.GetObjectInterface("WorldSwitchBox")));

	CheckIfPtrValid(light = engine.GetLightInterface("Spot"));

	auto testCharStopFunc = []() { testChar.Stop(); };

	engine.AddInteractable(
		engine.ConvertKeyTo('I'), true, 
		[]() { testChar.Go(0.0f); }, testCharStopFunc
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('J'), true,
		[]() { testChar.Go(glm::pi<float>() / 2.0f); }, testCharStopFunc
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('K'), true,
		[]() { testChar.Go(glm::pi<float>()); }, testCharStopFunc
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('L'), true,
		[]() { testChar.Go(-glm::pi<float>() / 2.0f); }, testCharStopFunc
	);
	engine.AddInteractable(
		engine.ConvertKeyTo('C'), false,
		[]() { testChar.LinkCharToCamera(*cameraSim); }, nullptr
	);

	engine.AddInteractable(
		engine.ConvertKeyTo('R'), true, []() { testChar.Rotate({ 0.0f, 1.0f, 0.0f }); }
	);

	engine.AddMouseScrollCallback([](double value) { cameraSim->Zoom(static_cast<float>(value)); });

	// Counted timed callback test
	engine.AddTimedCallback(
		{ std::chrono::seconds(1), [](size_t count) { std::cout << "TimerTest " << count << '\n'; }, 3 }
	);
	
	// End triggered timed callback test
	engine.AddInteractable(
		engine.ConvertKeyTo('M'), false, 
		[]() { 
			if (!stopSwitchColors)
			{
				stopSwitchColors = true;
				std::cout << "Stopped color switch. Switches happened " << amountOfTimesColorChanged << " times\n";
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

ScriptCleanUp()
{
	engineInter = nullptr;
	cameraSim = nullptr;
	testChar.Reset();
}