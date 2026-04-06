#include "EverettScript.h"

#include <iostream>

ICameraSim* cameraSim = nullptr;
IColliderSim* blockCollider = nullptr;

class TestCharHolder
{
	ISolidSim* testCharSolid;
	IColliderSim* testCharCollider;
	bool moving{};
	bool linkedToCamera{};

public:
	void SetSolidSim(ISolidSim& testCharSolid)
	{
		this->testCharSolid = &testCharSolid;
		this->testCharSolid->EnableAutoModelUpdates();
	}

	void SetColliderSim(IColliderSim& testCharCollider)
	{
		this->testCharCollider = &testCharCollider;
		testCharSolid->LinkObject(testCharCollider);
	}

	void SetupBlockCollision(IColliderSim& blockCollider)
	{
		testCharCollider->AddCollisionCallback(
			{ [this]() { testCharSolid->SetLastPosition(); }, nullptr, &blockCollider, true }
		);
	}

	void LinkCharToCamera(ICameraSim& camera)
	{
		std::cout << "Camera link to char";
		if (!linkedToCamera)
		{
			testCharCollider->LinkObject(camera);
		}

		linkedToCamera = !linkedToCamera;
		std::cout << linkedToCamera ? " on\n" : " off\n";
		testCharCollider->EnableObjectLinking(linkedToCamera);
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

CameraObjectInit()
{
	cameraSim = &objectCamera;
	cameraSim->EnableAutoModelUpdates();
}

ScriptObjectInit(TestChar, ISolidSim)
{
	testChar.SetSolidSim(objectTestChar);
}

ScriptObjectInit(TestCharBox, IColliderSim)
{
	testChar.SetColliderSim(objectTestCharBox);
}

ScriptObjectInit(BlockBox, IColliderSim)
{
	blockCollider = &objectBlockBox;
	testChar.SetupBlockCollision(objectBlockBox);
}

ScriptKeybindPressed(I, 1)
{
	testChar.Go(0.0f);
}

ScriptKeybindPressed(J, 1)
{
	testChar.Go(glm::pi<float>() / 2.0f);
}

ScriptKeybindPressed(K, 1)
{
	testChar.Go(glm::pi<float>());
}

ScriptKeybindPressed(L, 1)
{
	testChar.Go(-glm::pi<float>() / 2.0f);
}

ScriptKeybindReleased(I)
{
	testChar.Stop();
}

ScriptKeybindReleased(J)
{
	testChar.Stop();
}

ScriptKeybindReleased(K)
{
	testChar.Stop();
}

ScriptKeybindReleased(L)
{
	testChar.Stop();
}

ScriptKeybindPressed(C, 0)
{
	if (cameraSim)
	{
		testChar.LinkCharToCamera(*cameraSim);
	}
}

ScriptCleanUp()
{
	cameraSim = nullptr;
	blockCollider = nullptr;
	testChar.Reset();
}