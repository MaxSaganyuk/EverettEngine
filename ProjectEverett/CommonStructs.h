#pragma once

#include <memory>
#include <string>

struct WavDataRaw
{
	unsigned int channels{};
	unsigned int sampleRate{};
	unsigned long long totalPCMFrameCount{};
	double duration;
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
		duration = wavDataOwner.duration;
	}

	operator bool()
	{
		return data.lock().get();
	}
};