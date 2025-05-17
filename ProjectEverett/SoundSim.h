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

		std::string fileName;
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
	std::string GetSimInfoForSaveImpl();
public:
	std::string GetSimInfoToSave(const std::string& soundName);
	bool SetSimInfoToLoad(std::string& line);

	static void InitOpenAL();
	static void SetCamera(CameraSim& camera);
	SoundSim() = default;
	SoundSim(const std::string& file, glm::vec3& pos);
	SoundSim(const std::string& file, glm::vec3&& pos);

	static std::string GetObjectTypeNameStr();

	void Play() override;
	bool IsPlaying() override;
	void Stop() override;
	void UpdatePositions() override;
	
	~SoundSim();
};