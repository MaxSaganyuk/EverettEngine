#pragma once

#include "IObjectSim.h"

#include <functional>
#include <vector>

class ISolidSim : virtual public IObjectSim
{
public:
	constexpr static float fullRotation = 360.0f;

	virtual std::vector<std::string> GetModelMeshNames() = 0;
	virtual size_t GetMeshAmount() = 0;
	virtual void SetModelVisibility(bool value) = 0;
	virtual bool GetModelVisibility() = 0;
	virtual void SetModelMeshVisibility(const std::string name, bool value) = 0;
	virtual void SetModelMeshVisibility(size_t index, bool value) = 0;
	virtual bool GetModelMeshVisibility(const std::string name) = 0;
	virtual bool GetModelMeshVisibility(size_t index) = 0;

	virtual void InvokeAutoScale() = 0;

	virtual std::vector<std::string> GetModelAnimationNames() = 0;
	virtual size_t GetModelAnimationAmount() = 0;
	virtual void SetModelAnimation(size_t index) = 0;
	virtual void SetModelAnimation(const std::string& name) = 0;
	virtual size_t GetModelAnimation() = 0;
	virtual double GetModelAnimationSpeed() = 0;
	virtual void SetModelAnimationSpeed(double speed) = 0;
	virtual void PlayModelAnimation(bool loop = false) = 0;
	virtual void PauseModelAnimation() = 0;
	virtual void StopModelAnimation() = 0;
	virtual bool IsModelAnimationPlaying() = 0;
	virtual bool IsModelAnimationPaused() = 0;
	virtual bool IsModelAnimationLooped() = 0;
	virtual void SetModelAnimationPlaybackCallback(std::function<void(bool, bool, bool)> callback) = 0;
};