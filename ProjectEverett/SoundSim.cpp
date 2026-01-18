#include "SoundSim.h"

#include <OpenAL\alext.h>

#include <iostream>
#include <cassert>

#include "ContextManager.h"
#define ContextLock ContextManager<ALCcontext> mux(context);

#include "EverettException.h"

std::string SoundSim::GetSimInfoForSaveImpl()
{
	return ObjectSim::GetSimInfoToSaveImpl();
}

std::string SoundSim::GetSimInfoToSave(const std::string& soundName)
{
	std::string info = GetObjectTypeNameStr() + "**" + soundName + '*' + sound.fileName + '*';

	info += GetSimInfoForSaveImpl();

	return info + '\n';
}

bool SoundSim::SetSimInfoToLoad(std::string_view& line)
{
	return ObjectSim::SetSimInfoToLoad(line);
}

void SoundSim::InitOpenAL()
{
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cerr << "Cannot get the device\n";
	}

	bool res = alcIsExtensionPresent(nullptr, "AL_EXT_float32");

	ContextManager<ALCcontext>::SetContextSetter([](ALCcontext* context){ alcMakeContextCurrent(context); });
}

void SoundSim::TerminateOpenAL()
{
	alcCloseDevice(device);
}

void SoundSim::SetCamera(CameraSim& camera)
{
	SoundSim::camera = &camera;
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
		sound.data.lock().get(),
		static_cast<int>(sound.totalPCMFrameCount * sizeof(float)), 
		sound.sampleRate
	);

	return true;
}

void SoundSim::Play(bool loop)
{
	if (!SoundSim::camera)
	{
		ThrowExceptionWMessage("Cannot play sound without listener (camera)");
	}

	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);
	alSourcei(sound.source, AL_LOOPING, loop);

	UpdatePositions();

	alSourcePlay(sound.source);

	sound.playStates.playing = true;
	sound.playStates.paused = false;
	sound.playStates.looped = loop;
}

bool SoundSim::IsPlaying()
{
	return sound.playStates.playing;
}

void SoundSim::Pause()
{
	ContextLock

	alSourcePause(sound.source);

	sound.playStates.paused = true;
}

bool SoundSim::IsPaused()
{
	return sound.playStates.paused;
}

bool SoundSim::IsLooped()
{
	return sound.playStates.looped;
}

void SoundSim::Stop()
{
	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);
	alSourceStop(sound.source);

	sound.playStates.ResetValues();
}

void SoundSim::SetupSound(WavData&& wavData)
{
	sound = std::move(wavData);

	CreateContext();
	CreateBufferAndSource();
}

void SoundSim::SetPlaybackSpeed(float speed)
{
	ContextLock

	sound.playbackSpeed = speed;
	alSourcef(sound.source, AL_PITCH, sound.playbackSpeed);
}

float SoundSim::GetPlaybackSpeed()
{
	return sound.playbackSpeed;
}

void SoundSim::UpdatePositions()
{
	ContextLock

	glm::vec3 pos = sound.pos;
	alSource3f(sound.source, AL_POSITION, pos.x, pos.y, pos.z);
	alSource3f(sound.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

	glm::vec3& listenerPos = SoundSim::camera->GetPositionVectorAddr();
	alListener3f(AL_POSITION, listenerPos.x, listenerPos.y, listenerPos.z);
	alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

	const glm::vec3& listenerFront = SoundSim::camera->GetFrontVectorAddr();
	const glm::vec3& listenerUp = SoundSim::camera->GetUpVectorAddr();
	std::vector<float> cameraOrientation {
		listenerFront.x, listenerFront.y, listenerFront.z,
		listenerUp.x,    listenerUp.y,    listenerUp.z
	};
	alListenerfv(AL_ORIENTATION, cameraOrientation.data());
}

SoundSim::SoundSim(WavData&& wavData)
{
	SetupSound(std::move(wavData));
}

SoundSim::SoundSim(SoundSim&& otherSoundSim) noexcept
	: context(otherSoundSim.context), sound(otherSoundSim.sound)
{
	otherSoundSim.context = nullptr;
}

std::string SoundSim::GetObjectTypeNameStr()
{
	return "Sound";
}

SoundSim::~SoundSim()
{
	ContextLock

	if(context)
	{ 
		alcDestroyContext(context);
	}
}