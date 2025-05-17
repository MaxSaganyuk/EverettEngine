#include "SoundSim.h"

#include <AL\alext.h>
ALCdevice* SoundSim::device = nullptr;

#define DR_WAV_IMPLEMENTATION
#include <DrWav/dr_wav.h>

#include <iostream>
#include <cassert>

#include "ContextManager.h"
#define ContextLock ContextManager<ALCcontext> mux(context, [this](ALCcontext* context){ alcMakeContextCurrent(context); });
std::recursive_mutex ContextManager<ALCcontext>::rMutex;
size_t ContextManager<ALCcontext>::counter = 0;

CameraSim* SoundSim::camera = nullptr;

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

bool SoundSim::SetSimInfoToLoad(std::string& line)
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
}

void SoundSim::SetCamera(CameraSim& camera)
{
	SoundSim::camera = &camera;
}

bool SoundSim::LoadFile(const std::string& file)
{
	sound.fileName = file;
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
	if (!SoundSim::camera)
	{
		assert(false && "Cannot play sound without listener (camera)");
	}

	ContextLock

	alSourcei(sound.source, AL_BUFFER, sound.buffer);

	UpdatePositions();

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

void SoundSim::SetupSound(const std::string& file)
{
	CreateContext();
	LoadFile(file);
	CreateBufferAndSource();
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

	glm::vec3& listenerFront = SoundSim::camera->GetFrontVectorAddr();
	glm::vec3& listenerUp = SoundSim::camera->GetUpVectorAddr();
	std::vector<float> cameraOrientation {
		listenerFront.x, listenerFront.y, listenerFront.z,
		listenerUp.x,    listenerUp.y,    listenerUp.z
	};
	alListenerfv(AL_ORIENTATION, cameraOrientation.data());
}

SoundSim::SoundSim(const std::string& file, glm::vec3& pos)
{
	sound.pos = std::move(pos);
	SetupSound(file);
}

SoundSim::SoundSim(const std::string& file, glm::vec3&& pos) 
{
	sound.pos = std::move(pos);
	SetupSound(file);
}

std::string SoundSim::GetObjectTypeNameStr()
{
	return "Sound";
}

SoundSim::~SoundSim()
{
	drwav_free(sound.data, nullptr);
}