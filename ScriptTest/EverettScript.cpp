#include "EverettScript.h"

#include <memory>

class InternalState
{
public:
	InternalState()
	{
		Reset();
	}

	void Reset()
	{
		charInterRunning = false;
		charInter = nullptr;
		lightInter = nullptr;
		animCharInter = nullptr;
	}

	bool charInterRunning;
	ISolidSim* charInter;
	ILightSim* lightInter;
	ISolidSim* animCharInter;
};

InternalState interState;
ICameraSim* cameraSim = nullptr;

class TestCharHolder
{
	ISolidSim* testCharSolid;
	bool moving{};
	bool linkedToCamera{};

public:
	void SetSolidSim(ISolidSim* testCharSolid)
	{
		this->testCharSolid = testCharSolid;
		this->testCharSolid->EnableAutoModelUpdates();
	}

	void LinkCharToCamera(ICameraSim* camera)
	{
		if (!testCharSolid) return;

		if (!linkedToCamera)
		{
			testCharSolid->LinkObject(*camera);
		}

		linkedToCamera = !linkedToCamera;
		testCharSolid->EnableObjectLinking(linkedToCamera);
	}

	void Go(float rotation, float movementByX, float movementByZ)
	{
		if (testCharSolid)
		{
			if (!moving)
			{
				testCharSolid->SetRotationVector({ 0.0f, rotation, 0.0f }, false);
				testCharSolid->PlayModelAnimation(true);
			}

			moving = true;
			testCharSolid->MoveByAxis({ movementByX, 0.0f, movementByZ });
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


CameraScriptLoop()
{
	if (!cameraSim)
	{
		cameraSim = &camera;
		cameraSim->EnableAutoModelUpdates();
	}
}

ScriptObjectInit(Rise1, ISolidSim)
{
	interState.charInter = &objectRise1;
	interState.charInter->SetAllMeshVisibility(true);

	for (size_t i = 20; i < 30; ++i)
	{
		interState.charInter->SetModelMeshVisibility(i, false);
	}
}

ScriptObjectInit(stickAnimTest, ISolidSim)
{
	interState.animCharInter = &objectstickAnimTest;
}

ScriptObjectInit(stickAnimTest1, ISolidSim)
{
	objectstickAnimTest1.SetModelAnimation(0);
	objectstickAnimTest1.PlayModelAnimation();
}

ScriptObjectInit(Spot0, ILightSim)
{
	interState.lightInter = &objectSpot0;
}

ScriptObjectInit(TestChar, ISolidSim)
{
	testChar.SetSolidSim(&objectTestChar);
}

ScriptKeybindPressed(I)
{
	testChar.Go(0.0f, 0.0f, 1.0f);
}

ScriptKeybindPressed(J)
{
	testChar.Go(glm::pi<float>() / 2.0f, 1.0f, 0.0f);
}

ScriptKeybindPressed(K)
{
	testChar.Go(glm::pi<float>(), 0.0f, -1.0f);
}

ScriptKeybindPressed(L)
{
	testChar.Go(-glm::pi<float>() / 2.0f, -1.0f, 0.0f);
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

ScriptCleanUp()
{
	cameraSim = nullptr;
	interState.Reset();
	testChar.Reset();
}