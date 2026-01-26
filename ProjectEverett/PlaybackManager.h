#pragma once

#include <chrono>
#include <functional>

class PlaybackManager
{
public:
	using StateChangeCallback = std::function<void(bool, bool, bool)>;
private:
	bool playing;
	bool paused;
	bool looped;

	std::chrono::system_clock::time_point startPlaybackTime;
	std::chrono::system_clock::time_point currentPlaybackTime;

	StateChangeCallback stateChangeCallback;

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

		if (stateChangeCallback)
		{
			stateChangeCallback(playing, paused, looped);
		}
	}

	void NullifyValues()
	{
		playing = false;
		paused = false;
		looped = false;

		if (stateChangeCallback)
		{
			stateChangeCallback(playing, paused, looped);
		}
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

		if (stateChangeCallback)
		{
			stateChangeCallback(playing, paused, looped);
		}
	}

	void Pause()
	{
		paused = true;

		if (stateChangeCallback)
		{
			stateChangeCallback(playing, paused, looped);
		}
	}

	void Stop()
	{
		ResetValues();
	}

	bool IsPlaying() const
	{
		return playing;
	}

	bool IsPaused() const
	{
		return paused;
	}

	bool IsLooped() const
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

	void SetStateChangeCallback(StateChangeCallback callback)
	{
		stateChangeCallback = callback;
	}
};
