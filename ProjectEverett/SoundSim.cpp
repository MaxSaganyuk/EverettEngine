#include "SoundSim.h"

#include <OpenAL\alc.h>
#include <OpenAL\al.h>
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

	if (!CreateContext())
	{
		std::cerr << "Failed to create context\n";
	}

	bool res = alcIsExtensionPresent(nullptr, "AL_EXT_float32");

	ContextManager<ALCcontext>::SetContextSetter([](ALCcontext* context){ alcMakeContextCurrent(context); });
}

void SoundSim::TerminateOpenAL()
{
	ContextLock

	if (context)
	{
		alcDestroyContext(context);
	}
	
	if (device)
	{
		alcCloseDevice(device);
	}
}

void SoundSim::SetCamera(std::weak_ptr<CameraSim> camera)
{
	SoundSim::camera = camera;
}

void SoundSim::UpdateCameraPosition()
{
	auto cameraPtr = camera.lock();

	if (cameraPtr && soundsCurrentlyPlaying)
	{
		ContextLock

		const glm::vec3& listenerPos = cameraPtr->GetPositionVectorAddr();
		alListener3f(AL_POSITION, listenerPos.x, listenerPos.y, listenerPos.z);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

		const glm::vec3& listenerFront = cameraPtr->GetFrontVectorAddr();
		const glm::vec3& listenerUp = cameraPtr->GetUpVectorAddr();
		float cameraOrientation[]{
			listenerFront.x, listenerFront.y, listenerFront.z,
			listenerUp.x,    listenerUp.y,    listenerUp.z
		};
		alListenerfv(AL_ORIENTATION, cameraOrientation);
	}
}

bool SoundSim::CreateContext()
{
	context = alcCreateContext(device, nullptr);

	return context;
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
	if (!SoundSim::camera.lock())
	{
		ThrowExceptionWMessage("Cannot play sound without listener (camera)");
	}

	if (!currentSoundPlaying)
	{
		currentSoundPlaying = true;
		++soundsCurrentlyPlaying;
	}

	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);
	alSourcei(sound.source, AL_LOOPING, loop);

	alSourcePlay(sound.source);

	sound.playStates.Play(loop);
}

bool SoundSim::IsPlaying()
{
	return sound.playStates.IsPlaying();
}

void SoundSim::Pause()
{
	ContextLock

	alSourcePause(sound.source);

	sound.playStates.Pause();
}

bool SoundSim::IsPaused()
{
	return sound.playStates.IsPaused();
}

bool SoundSim::IsLooped()
{
	return sound.playStates.IsLooped();
}

void SoundSim::Stop()
{
	if (currentSoundPlaying)
	{
		currentSoundPlaying = false;
		--soundsCurrentlyPlaying;

		ContextLock

		alSourcei(sound.source, AL_BUFFER, sound.buffer);
		alSourceStop(sound.source);

		sound.playStates.Stop();
	}
}

void SoundSim::SetupSound(WavData&& wavData)
{
	sound = std::move(wavData);

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

void SoundSim::UpdateCurrentPlaybackTime()
{
	if (sound.duration < sound.playStates.GetCurrentTime())
	{
		Stop();
	}
}

void SoundSim::UpdateSoundPosition()
{
	UpdateCurrentPlaybackTime();

	if (sound.playStates.IsPlaying())
	{
		ContextLock

		const glm::vec3& currentPos = pos;
		alSource3f(sound.source, AL_POSITION, currentPos.x, currentPos.y, currentPos.z);
		alSource3f(sound.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	}
}

void SoundSim::SetPlaybackCallback(std::function<void(bool, bool, bool)> callback)
{
	sound.playStates.SetStateChangeCallback(callback);
}

SoundSim::SoundSim(WavData&& wavData)
{
	currentSoundPlaying = false;
	SetupSound(std::move(wavData));
}

SoundSim::SoundSim(SoundSim&& otherSoundSim) noexcept
	: sound(otherSoundSim.sound), currentSoundPlaying(otherSoundSim.currentSoundPlaying)
{}

std::string SoundSim::GetObjectTypeNameStr()
{
	return "Sound";
}