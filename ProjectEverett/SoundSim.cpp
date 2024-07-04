#include "SoundSim.h"

#include <AL\alext.h>
ALCdevice* SoundSim::device = nullptr;

#define DR_WAV_IMPLEMENTATION
#include <DrWav/dr_wav.h>

#include <iostream>

#include "ContextManager.h"
#define ContextLock ContextManager<ALCcontext> mux(context, [this](ALCcontext* context){ alcMakeContextCurrent(context); });
std::recursive_mutex ContextManager<ALCcontext>::rMutex;


void SoundSim::InitOpenAL()
{
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cerr << "Cannot get the device\n";
	}

	bool res = alcIsExtensionPresent(nullptr, "AL_EXT_float32");
}

bool SoundSim::LoadFile(const std::string& file)
{
	sound.data = drwav_open_file_and_read_pcm_frames_f32
		(file.c_str(), &sound.channels, &sound.sampleRate, &sound.totalPCMFrameCount, nullptr);

	if (!sound.data)
	{
		std::cerr << "Cannot get data from file " + file + '\n';
		return false;
	}

	return true;
}

bool SoundSim::CreateContext()
{
	context = alcCreateContext(device, nullptr);

	return true;
}

bool SoundSim::CreateBufferAndSource()
{
	ContextLock

	alGenBuffers(1, &sound.buffer);
	alGenSources(1, &sound.source);

	alBufferData(
		sound.buffer, 
		(sound.channels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32, 
		sound.data, 
		sound.totalPCMFrameCount * sizeof(float), 
		sound.sampleRate
	);

	return true;
}

void SoundSim::Play()
{
	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);
	alSourcePlay(sound.source);
}

bool SoundSim::IsPlaying()
{
	ContextLock

	ALint state;
	alGetSourcei(sound.source, AL_SOURCE_STATE, &state);

	return state == AL_PLAYING;
}

void SoundSim::Stop()
{
	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);
	alSourceStop(sound.source);
}

SoundSim::SoundSim(const std::string& file)
{
	CreateContext();
	LoadFile(file);
	CreateBufferAndSource();
}

SoundSim::~SoundSim()
{
	drwav_free(sound.data, nullptr);
}