#pragma once

#include "interfaces/IObjectSim.h"

#include <unordered_map>
#include <type_traits>

#include "ScriptFuncStorage.h"
#include "SimSerializer.h"

class ObjectSim : virtual public IObjectSim
{
protected:
	std::string GetSimInfoToSaveImpl();
	void SetSimInfoToLoad(std::string& line);
	void CheckRotationLimits();
	
	constexpr static size_t realDirectionAmount = 6;

	glm::vec3 scale;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 pos;
	glm::vec3 lastPos;
	Rotation rotate;
	bool lastBlocker;
	float speed;

	bool ghostMode;

	Direction lastDir;

	std::unordered_map<Direction, bool> disabledDirs;
	std::pair<Rotation, Rotation> rotationLimits;

	ScriptFuncStorage scriptFuncStorage;

public:
	ObjectSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 0.006f
	);

	static std::string GetObjectTypeNameStr();

	void InvertMovement() override;
	bool IsMovementInverted() override;

	glm::vec3& GetFrontVectorAddr() override;
	glm::vec3& GetPositionVectorAddr() override;
	glm::vec3& GetUpVectorAddr() override;
	glm::vec3& GetScaleVectorAddr() override;

	void SetGhostMode(bool val) override;
	bool IsGhostMode() const override;

	void DisableDirection(Direction dir) override;
	void EnableDirection(Direction dir) override;
	void EnableAllDirections() override;
	size_t GetAmountOfDisabledDirs() override;
	Direction GetLastDirection() override;

	void SetLastPosition() override;
	void SetPosition(Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }) override;

	void LimitRotations(const Rotation& min, const Rotation& max) override;
	void Rotate(const Rotation& toRotate) override;

	void AddScriptFunc(const std::string& dllName, ScriptFuncStorage::ScriptFuncWeakPtr& scriptFunc);
	void ExecuteScriptFunc(const std::string& dllName = "") override;
	void ExecuteAllScriptFuncs() override;
	bool IsScriptFuncAdded(const std::string& dllName = "") override;
	bool IsScriptFuncRunnable(const std::string& dllName = "") override;
};