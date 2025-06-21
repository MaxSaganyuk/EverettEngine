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

	void NullifyValues()
	{
		playing = false;
		paused = false;
		looped = false;
	}

	PlayerStates()
	{
		ResetValues();
	}
};