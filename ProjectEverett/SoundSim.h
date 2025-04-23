#pragma once

#include <AL\al.h>
#include <AL\alc.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdEx/utilityEx.h>

#include <string>

#include "interfaces/ISoundSim.h"
#include "CameraSim.h"

class SoundSim : public ObjectSim, public ISoundSim
{
	static ALCdevice* device;
	static CameraSim* camera;
	
	ALCcontext* context;

	struct SoundInfo
	{
		stdEx::ValWithBackup<glm::vec3> pos;

		unsigned int channels;
		unsigned int sampleRate;
		unsigned long long totalPCMFrameCount;
		float* data = nullptr;

		ALuint buffer;
		ALuint source;

		SoundInfo()
		{
			pos.ResetBackup(&camera->GetPositionVectorAddr());
		}
	};

	SoundInfo sound;

	void SetupSound(const std::string& file);
	bool LoadFile(const std::string& file);
	bool CreateContext();
	bool CreateBufferAndSource();
public:
	static void InitOpenAL();
	static void SetCamera(CameraSim& camera);
	SoundSim() = default;
	SoundSim(const std::string& file, glm::vec3& pos);
	SoundSim(const std::string& file, glm::vec3&& pos);

	void Play() override;
	bool IsPlaying() override;
	void Stop() override;
	void UpdatePositions() override;
	
	~SoundSim();
};