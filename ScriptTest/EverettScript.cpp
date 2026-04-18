#include "EverettScript.h"

#include <iostream>

IEverettEngine* engineInter = nullptr;
ICameraSim* cameraSim = nullptr;

class TestCharHolder
{
	ISolidSim* testCharSolid;
	IColliderSim* testCharCollider;
	bool moving{};
	bool linkedToCamera{};

	void CheckIfPtrValid(IObjectSim* ptr)
	{
		if (!ptr)
		{
			std::cerr << "Invalid ptr. Cannot continue\n";
			std::terminate();
		}
	}

public:
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
	cameraSim->EnableAutoModelUpdates();

	testChar.SetSolidSim(engine.GetSolidInterface("TestChar"));
	testChar.SetColliderSim(engine.GetColliderInterface("TestCharBox"));
	testChar.SetupBlockCollision(engine.GetColliderInterface("BlockBox"));
	testChar.SetupWorldSwitchCollision(engine.GetColliderInterface("WorldSwitchBox"));

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

	engine.AddMouseScrollCallback([](double value) { cameraSim->Zoom(static_cast<float>(value)); });

	engineInter = &engine;
}

ScriptCleanUp()
{
	engineInter = nullptr;
	cameraSim = nullptr;
	testChar.Reset();
}