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

#include "CommonStructs.h"

class SoundSim : public ObjectSim, public ISoundSim
{
	static inline ALCdevice* device = nullptr;
	static inline CameraSim* camera = nullptr;

	static inline bool freeDRWav = false;

	ALCcontext* context = nullptr;

	struct SoundInfo
	{
		stdEx::ValWithBackup<glm::vec3> pos;

		PlayerStates playStates;

		std::string fileName;
		float playbackSpeed{};
		unsigned int channels{};
		unsigned int sampleRate{};
		unsigned long long totalPCMFrameCount{};
		float* data = nullptr;

		ALuint buffer{};
		ALuint source{};

		SoundInfo()
		{
			playbackSpeed = 1.0f;
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
	bool SetSimInfoToLoad(std::string_view& line);

	static void InitOpenAL();
	static void TerminateOpenAL();
	static void TriggerFreeDrWav(bool value = true);
	static void SetCamera(CameraSim& camera);
	SoundSim() = default;
	SoundSim(const std::string& file, glm::vec3& pos);
	SoundSim(const std::string& file, glm::vec3&& pos);

	static std::string GetObjectTypeNameStr();

	void Play(bool loop = false) override;
	bool IsPlaying() override;

	void Pause() override;
	bool IsPaused() override;

	bool IsLooped() override;

	void Stop() override;
	void UpdatePositions() override;
	void SetPlaybackSpeed(float speed) override;
	float GetPlaybackSpeed() override;
	
	~SoundSim();
};