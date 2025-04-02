#include "EverettScript.h"

class InternalState
{
public:
	bool charInterRunning = false;
	ISolidSim* charInter = nullptr;
};

InternalState interState;

CameraScriptLoop()
{
	if (interState.charInter)
	{
		if (!interState.charInterRunning)
		{
			interState.charInterRunning = true;
			interState.charInter->ExecuteScriptFunc(); // Call of other script func from script func test
		}
		interState.charInter->Rotate({ 0.0f, 0.001f, 0.0f });
		interState.charInter->ForceModelUpdate();
	}
}

ScriptObjectInit(Rise, Solid)
{
	interState.charInter = &objectRise;
}