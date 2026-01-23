#pragma once

#include <chrono>

class PlaybackManager
{
	bool playing;
	bool paused;
	bool looped;

	std::chrono::system_clock::time_point startPlaybackTime;
	std::chrono::system_clock::time_point currentPlaybackTime;

	friend class SolidSim;

public:
	PlaybackManager()
	{
		ResetValues();
	}

	void ResetValues()
	{
		playing = false;
		paused = true;
		looped = false;
		ResetPlaybackTime();
	}

	void NullifyValues()
	{
		playing = false;
		paused = false;
		looped = false;
	}

	void ResetPlaybackTime()
	{
		startPlaybackTime = currentPlaybackTime = std::chrono::system_clock::now();
	}

	void Play(bool loop = false)
	{
		if (!playing)
		{
			startPlaybackTime = std::chrono::system_clock::now();
		}
		else if (paused)
		{
			auto requiredDiff = currentPlaybackTime - startPlaybackTime;
			currentPlaybackTime = std::chrono::system_clock::now();
			startPlaybackTime = currentPlaybackTime - requiredDiff;
		}

		playing = true;
		paused = false;
		looped = loop;
	}

	void Pause()
	{
		paused = true;
	}

	void Stop()
	{
		ResetValues();
	}

	bool IsPlaying()
	{
		return playing;
	}

	bool IsPaused()
	{
		return paused;
	}

	bool IsLooped()
	{
		return looped;
	}

	double GetCurrentTime()
	{
		if (!paused)
		{
			currentPlaybackTime = std::chrono::system_clock::now();
		}

		return std::chrono::duration<double>(currentPlaybackTime - startPlaybackTime).count();
	}
};
