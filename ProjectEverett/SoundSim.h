#pragma once

#include <OpenAL\al.h>
#include <OpenAL\alc.h>

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
private:
	static inline ALCdevice* device = nullptr;
	static inline CameraSim* camera = nullptr;

	ALCcontext* context = nullptr;

	struct SoundInfo : WavData
	{
		stdEx::ValWithBackup<glm::vec3> pos;

		PlayerStates playStates;

		float playbackSpeed{};

		ALuint buffer{};
		ALuint source{};

		SoundInfo() = default;
		SoundInfo(WavData wavData)
			: WavData(wavData)
		{
			playbackSpeed = 1.0f;
			pos.ResetBackup(&camera->GetPositionVectorAddr());
		}
	};

	SoundInfo sound;

	void SetupSound(WavData&& wavData);
	bool CreateContext();
	bool CreateBufferAndSource();
	std::string GetSimInfoForSaveImpl();
public:
	std::string GetSimInfoToSave(const std::string& soundName);
	bool SetSimInfoToLoad(std::string_view& line);

	static void InitOpenAL();
	static void TerminateOpenAL();
	static void SetCamera(CameraSim& camera);
	SoundSim() = default;
	SoundSim(WavData&& wavData);
	SoundSim(SoundSim&& otherSoundSim) noexcept;

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