#pragma once

#include <cassert>
#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <functional>

#include "interfaces/ISolidSim.h"

class SolidSim : virtual public ISolidSim
{
public:
	using ScriptFunc          = std::function<void(ISolidSim&)>;
	using ScriptFuncSharedPtr = std::shared_ptr<ScriptFunc>;
	using ScriptFuncWeakPtr   = std::weak_ptr<ScriptFunc>;
	using ScriptFuncMap       = std::map<std::string, ScriptFuncWeakPtr>;
private:
	constexpr static size_t realDirectionAmount = 6;

	glm::mat4 model;
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

	SolidType type;

	std::unordered_map<Direction, bool> disabledDirs;
	std::pair<Rotation, Rotation> rotationLimits;

	std::string lastExecutedScriptDll;
	ScriptFuncMap scriptFuncMap;

	void CheckRotationLimits();
	void ResetModelMatrix(const Rotation& toRotate = {});
public:
	SolidSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 0.006f
	);

	void InvertMovement() override;
	bool IsMovementInverted() override;

	glm::mat4& GetModelMatrixAddr() override;
	glm::vec3& GetFrontVectorAddr() override;
	glm::vec3& GetPositionVectorAddr() override;
	glm::vec3& GetUpVectorAddr() override;
	glm::vec3& GetScaleVectorAddr() override;
	void ForceModelUpdate() override;

	void SetGhostMode(bool val) override;
	bool IsGhostMode() const override;

	void DisableDirection(Direction dir) override;
	void EnableDirection(Direction dir) override;
	void EnableAllDirections() override;
	size_t GetAmountOfDisabledDirs() override;
	Direction GetLastDirection() override;

	void SetType(SolidType type) override;

	void SetLastPosition() override;
	void SetPosition(Direction dir, const glm::vec3& limitAxis = { 1.0f, 1.0f, 1.0f }) override;

	void LimitRotations(const Rotation& min, const Rotation& max) override;
	void Rotate(const Rotation& toRotate) override;

	static bool CheckForCollision(const SolidSim& solid1, const SolidSim& solid2);
	bool CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2) override;

	void AddScriptFunc(const std::string& dllName, ScriptFuncSharedPtr& scriptFunc);
	void ExecuteScriptFunc(const std::string& dllName = "") override;
	void ExecuteAllScriptFuncs() override;
	bool IsScriptFuncAdded(const std::string& dllName = "") override;
};