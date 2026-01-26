#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdEx/utilityEx.h>

#include <string>

#include "interfaces/ISoundSim.h"
#include "CameraSim.h"

#include "CommonStructs.h"
#include "PlaybackManager.h"

struct ALCdevice;
struct ALCcontext;

class SoundSim : public ObjectSim, public ISoundSim
{
private:
	static inline ALCdevice* device = nullptr;
	static inline ALCcontext* context = nullptr;
	static inline std::weak_ptr<CameraSim> camera;
	static inline int soundsCurrentlyPlaying = 0;

	static bool CreateContext();

	struct SoundInfo : WavData
	{
		PlaybackManager playStates;

		float playbackSpeed{};

		unsigned int buffer{};
		unsigned int source{};

		SoundInfo() = default;
		SoundInfo(WavData wavData)
			: WavData(wavData)
		{
			playbackSpeed = 1.0f;
		}
	};

	SoundInfo sound;
	bool currentSoundPlaying;

	void SetupSound(WavData&& wavData);
	bool CreateBufferAndSource();
	std::string GetSimInfoForSaveImpl();
	void UpdateCurrentPlaybackTime();
public:
	std::string GetSimInfoToSave(const std::string& soundName);
	bool SetSimInfoToLoad(std::string_view& line);

	void UpdateSoundPosition();

	static void InitOpenAL();
	static void TerminateOpenAL();
	static void SetCamera(std::weak_ptr<CameraSim> camera);
	static void UpdateCameraPosition();
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
	void SetPlaybackSpeed(float speed) override;
	float GetPlaybackSpeed() override;

	void SetPlaybackCallback(std::function<void(bool, bool, bool)> callback) override;
};