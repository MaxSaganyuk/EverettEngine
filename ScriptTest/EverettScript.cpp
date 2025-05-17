#include "EverettScript.h"

class InternalState
{
public:
	bool charInterRunning = false;
	ISolidSim* charInter = nullptr;
	ILightSim* lightInter = nullptr;
	ISolidSim* animCharInter = nullptr;
};

InternalState interState;

CameraScriptLoop()
{
	if (interState.lightInter)
	{
		interState.lightInter->Rotate({ 0.0f, 0.01f, 0.0f });
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

ScriptKeybindReleased(T)
{
	interState.animCharInter->SetModelAnimationSpeed(interState.animCharInter->GetModelAnimationSpeed() + 0.1);
}

ScriptKeybindPressed(P)
{
	if (interState.animCharInter)
	{
		interState.animCharInter->IsModelAnimationPaused() ?
			interState.animCharInter->PlayModelAnimation(true) : interState.animCharInter->PauseModelAnimation();
	}
}