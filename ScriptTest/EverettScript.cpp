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
	void SetSolidSim(ISolidSim* testCharSolid)
	{
		this->testCharSolid = testCharSolid;
		this->testCharSolid->EnableAutoModelUpdates();

		if (testCharCollider)
		{
			testCharSolid->LinkObject(*testCharCollider);
		}
	}

	void SetColliderSim(IColliderSim* testCharCollider)
	{
		this->testCharCollider = testCharCollider;

		if (testCharSolid)
		{
			testCharSolid->LinkObject(*testCharCollider);
		}
	}

	void SetupBlockCollision(IColliderSim* blockCollider)
	{
		if (testCharSolid && testCharCollider)
		{
			testCharCollider->AddCollisionCallback(
				{ [this]() { testCharSolid->SetLastPosition(); }, nullptr, blockCollider, true }
			);
		}
	}

	void LinkCharToCamera(ICameraSim* camera)
	{
		if (!testCharSolid) return;

		std::cout << "Camera link to char";
		if (!linkedToCamera)
		{
			testCharSolid->LinkObject(*camera);
		}

		linkedToCamera = !linkedToCamera;
		std::cout << linkedToCamera ? " on\n" : " off\n";
		testCharSolid->EnableObjectLinking(linkedToCamera);
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
	testChar.SetSolidSim(&objectTestChar);
}

ScriptObjectInit(TestCharBox, IColliderSim)
{
	testChar.SetColliderSim(&objectTestCharBox);
}

ScriptObjectInit(BlockBox, IColliderSim)
{
	blockCollider = &objectBlockBox;
}

ScriptKeybindPressed(I)
{
	testChar.Go(0.0f);
}

ScriptKeybindPressed(J)
{
	testChar.Go(glm::pi<float>() / 2.0f);
}

ScriptKeybindPressed(K)
{
	testChar.Go(glm::pi<float>());
}

ScriptKeybindPressed(L)
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

ScriptKeybindPressed(C)
{
	if (cameraSim)
	{
		testChar.LinkCharToCamera(cameraSim);
	}
}

// TODO: This is a temp workaround. Rewrite loading system to have expected sequencing of Init calls 
ScriptKeybindPressed(V)
{
	testChar.SetupBlockCollision(blockCollider);
}

ScriptCleanUp()
{
	cameraSim = nullptr;
	testChar.Reset();
}