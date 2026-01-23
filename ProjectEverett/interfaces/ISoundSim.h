#pragma once

#include "IObjectSim.h"

class ISoundSim : virtual public IObjectSim
{
public:
	virtual void Play(bool loop = false) = 0;
	virtual bool IsPlaying() = 0;

	virtual void Pause() = 0;
	virtual bool IsPaused() = 0;

	virtual bool IsLooped() = 0;

	virtual void Stop() = 0;
	virtual void SetPlaybackSpeed(float speed) = 0;
	virtual float GetPlaybackSpeed() = 0;
};