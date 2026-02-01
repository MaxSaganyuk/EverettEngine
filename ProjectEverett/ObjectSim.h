#pragma once

#include "interfaces/IObjectSim.h"

#include "stdEx/utilityEx.h"

#include <unordered_map>
#include <type_traits>

#include "ScriptFuncStorage.h"
#include "SimSerializer.h"
#include "ValueObserver.h"

class ObjectSim : virtual public IObjectSim
{
private:
	template<typename MemberFuncType, typename... ParamTypes>
	void ExecuteLinkedObjects(MemberFuncType memberFunc, ParamTypes&&... values);

	void UpdateFrontVector();
protected:
	static inline float renderDeltaTime = 1.0f;

	std::string GetSimInfoToSaveImpl();
	bool SetSimInfoToLoad(std::string_view& line);
	void CheckRotationLimits();
	
	constexpr static size_t realDirectionAmount = 6;

	ValueObserver<glm::vec3> pos;
	ValueObserver<glm::vec3> scale;
	ValueObserver<Rotation>  rotate;

	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 lastPos;
	bool lastBlocker;
	float speed;

	bool ghostMode;

	Direction lastDir;

	std::unordered_map<Direction, bool> disabledDirs;
	std::pair<Rotation, Rotation> rotationLimits;

	ScriptFuncStorage scriptFuncStorage;

	// Callbacks
	std::function<void()> positionChangeCallback;
	std::function<void()> rotationChangeCallback;

	static inline stdEx::RelationGraph<ObjectSim*> objectGraph;
	bool visited; // Utility bool to prevent infinite loops of linked object traversal

public:
	ObjectSim(
		const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& front = glm::vec3(0.0f, 0.0f, 1.0f),
		const float speed = 1.0f
	);
	~ObjectSim();

	static void InitializeObjectGraph();
	static std::string GetObjectTypeNameStr();
	static void SetRenderDeltaTime(float deltaTime);

	void InvertMovement(bool value = true) override;
	bool IsMovementInverted() override;

	void SetPositionVector(const glm::vec3& vect) override;
	void SetScaleVector(const glm::vec3& vect) override;
	void SetRotationVector(const Rotation& vect) override;
		
	const glm::vec3& GetFrontVectorAddr() override;
	const glm::vec3& GetUpVectorAddr() override;

	glm::vec3& GetPositionVectorAddr() override;
	glm::vec3& GetScaleVectorAddr() override;

	void SetMovementSpeed(float speed) override;
	float GetMovementSpeed() override;

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

	void AddScriptFunc(const std::string& dllPath, const std::string& dllName, ScriptFuncStorage::ScriptFuncWeakPtr& scriptFunc);
	void ClearScriptFuncMap();
	std::vector<std::pair<std::string, std::string>> GetTempScriptDLLInfo();

	// Callback setter
	void SetPositionChangeCallback(std::function<void()> callback) override;
	void SetRotationChangeCallback(std::function<void()> callback) override;

	void ExecuteScriptFunc(const std::string& dllName = "") override;
	void ExecuteAllScriptFuncs() override;
	bool IsScriptFuncAdded(const std::string& dllName = "") override;
	bool IsScriptFuncRunnable(const std::string& dllName = "") override;

	void LinkObject(IObjectSim& otherObject) override;
};