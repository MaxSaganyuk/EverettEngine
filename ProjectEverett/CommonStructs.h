#pragma once

struct PlayerStates
{
	bool playing;
	bool paused;
	bool looped;

	void ResetValues()
	{
		playing = false;
		paused = true;
		looped = false;
	}

	PlayerStates()
	{
		ResetValues();
	}
};