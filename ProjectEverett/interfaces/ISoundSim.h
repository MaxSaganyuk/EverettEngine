#pragma once

#include "ISolidSim.h"

class ISoundSim : virtual public IObjectSim
{
	virtual void Play() = 0;
	virtual bool IsPlaying() = 0;
	virtual void Stop() = 0;
	virtual void UpdatePositions() = 0;
};