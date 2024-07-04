#pragma once

#include <AL\al.h>
#include <AL\alc.h>

#include <string>

class SoundSim
{
	static ALCdevice* device;
	ALCcontext* context;

	struct SoundInfo
	{
		unsigned int channels;
		unsigned int sampleRate;
		unsigned long long totalPCMFrameCount;
		float* data = nullptr;

		ALuint buffer;
		ALuint source;
	};

	SoundInfo sound;

	bool LoadFile(const std::string& file);
	bool CreateContext();
	bool CreateBufferAndSource();
public:
	static void InitOpenAL();
	SoundSim(const std::string& file);
	void Play();
	bool IsPlaying();
	void Stop();
	~SoundSim();
};