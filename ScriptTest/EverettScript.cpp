#include "EverettScript.h"

class InternalState
{
public:
	bool charInterRunning = false;
	ISolidSim* charInter = nullptr;
	ILightSim* lightInter = nullptr;
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

ScriptObjectInit(Spot0, ILightSim)
{
	interState.lightInter = &objectSpot0;
}

ScriptKeybindReleased(T)
{
	if (interState.charInter)
	{
		interState.charInter->SetPosition(ISolidSim::Direction::Forward);
	}
}