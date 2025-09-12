#pragma once

#include <memory>

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

struct WavDataRaw
{
	unsigned int channels{};
	unsigned int sampleRate{};
	unsigned long long totalPCMFrameCount{};
};

struct WavDataOwner : WavDataRaw
{
	std::shared_ptr<float> data;
};

struct WavData : WavDataRaw
{
	std::weak_ptr<float> data;
	std::string fileName;

	WavData() = default;
	WavData(const WavDataOwner& wavDataOwner)
	{
		data = wavDataOwner.data;
		channels = wavDataOwner.channels;
		sampleRate = wavDataOwner.sampleRate;
		totalPCMFrameCount = wavDataOwner.totalPCMFrameCount;
	}

	operator bool()
	{
		return data.lock().get();
	}
};
