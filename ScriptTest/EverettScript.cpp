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

ScriptObjectInit(Rise1, ISolidSim)
{
	interState.charInter = &objectRise1;
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