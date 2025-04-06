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
		interState.lightInter->ForceModelUpdate();
	}
}

ScriptObjectInit(Rise, ISolidSim)
{
	interState.charInter = &objectRise;
}

ScriptObjectInit(Spot0, ILightSim)
{
	interState.lightInter = &objectSpot0;
}

ScriptKeybindPressed(T)
{
	if (interState.charInter)
	{
		interState.charInter->SetPosition(ISolidSim::Direction::Forward);
	}
}