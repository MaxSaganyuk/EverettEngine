#include "SolidSim.h"

void SolidSim::ForceModelUpdate()
{
	ResetModelMatrix();
}

void SolidSim::ResetModelMatrix()
{
	model = glm::mat4(1.0f);

	const glm::vec3& posRef = pos;
	const glm::vec3& scaleRef = scale;
	const glm::quat& orientRef = orient;

	model = glm::translate(model, posRef);
	model *= glm::mat4_cast(orientRef);
	model = glm::scale(model, scaleRef);
}

void SolidSim::EnableAutoModelUpdates(bool value)
{
	std::function<void()> resetModelMatrixWrapper = 
		value ? [this]() { ResetModelMatrix(); } : std::function<void()>(nullptr);

	pos.SetCallback(resetModelMatrixWrapper);
	scale.SetCallback(resetModelMatrixWrapper);
	orient.SetCallback(resetModelMatrixWrapper);
}

SolidSim::SolidSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const float speed
)
	: ObjectSim(pos, scale, speed)
{
	ResetModelMatrix();
	type = SolidType::Static;
}

std::string SolidSim::GetObjectTypeNameStr()
{
	return "Solid";
}

std::string SolidSim::CollectInfoToSaveFromSTMM()
{
	std::string res = "";

	if (STMM.IsInitialized())
	{
		res += SimSerializer::GetValueToSaveFrom(STMM.animationSpeed);
		res += SimSerializer::GetValueToSaveFrom(STMM.currentAnimationIndex);
		res += SimSerializer::GetValueToSaveFrom(STMM.animStates.startPlaybackTime);
		res += SimSerializer::GetValueToSaveFrom(STMM.animStates.currentPlaybackTime);
		res += SimSerializer::GetValueToSaveFrom(STMM.animStates.playing);
		res += SimSerializer::GetValueToSaveFrom(STMM.animStates.paused);
		res += SimSerializer::GetValueToSaveFrom(STMM.animStates.looped);
		res += SimSerializer::GetValueToSaveFrom(STMM.meshVisibility);
	}

	return res;
}

bool SolidSim::CollectInfoToLoadToSTMM(std::string_view& line)
{
	bool res = true;

	if (STMM.IsInitialized())
	{
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animationSpeed,                 1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.currentAnimationIndex,          1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animStates.startPlaybackTime,   1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animStates.currentPlaybackTime, 1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animStates.playing,             1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animStates.paused,              1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.animStates.looped,              1);
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.meshVisibility,                 1);
	}

	return res;
}

std::string SolidSim::GetSimInfoToSaveImpl()
{
	std::string res = ObjectSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(model);
	res += SimSerializer::GetValueToSaveFrom(type);

	res += CollectInfoToSaveFromSTMM();

	return res;
}

std::string SolidSim::GetSimInfoToSave(const std::string& modelSolidName)
{
	std::string info = GetObjectTypeNameStr() + '*' + modelSolidName + '*';

	info += GetSimInfoToSaveImpl();

	return info + '\n';
}

bool SolidSim::SetSimInfoToLoad(std::string_view& line)
{
	bool res = ObjectSim::SetSimInfoToLoad(line);

	res = res && SimSerializer::SetValueToLoadFrom(line, model, 1);
	res = res && SimSerializer::SetValueToLoadFrom(line, type,  1);

	res = res && CollectInfoToLoadToSTMM(line);

	return res;
}

glm::mat4& SolidSim::GetModelMatrixAddr()
{
	return model;
}

void SolidSim::SetType(SolidType type)
{
	this->type = type;
}

void SolidSim::MoveInDirection(ObjectSim::Direction dir, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	ObjectSim::MoveInDirection(dir, limitAxis, executeLinkedObjects);

	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix();
	}
	else 
	{
		const glm::vec3& posRef = pos;

		model[3].x = posRef.x;
		model[3].y = posRef.y;
		model[3].z = posRef.z;
	}
}

void SolidSim::MoveByAxis(const glm::vec3& axis, const glm::vec3& limitAxis, bool executeLinkedObjects)
{
	ObjectSim::MoveByAxis(axis, limitAxis, executeLinkedObjects);

	if (type == SolidType::Static && lastBlocker)
	{
		ResetModelMatrix();
	}
	else
	{
		const glm::vec3& posRef = pos;

		model[3].x = posRef.x;
		model[3].y = posRef.y;
		model[3].z = posRef.z;
	}
}

void SolidSim::Rotate(const Rotation& toRotate, bool executeLinkedObjects)
{
	ObjectSim::Rotate(toRotate, executeLinkedObjects);

	if (type != SolidType::Static)
	{
		const glm::quat& orientRef = orient;
		model *= glm::mat4_cast(renderDeltaTime * orientRef);
	}
}

bool SolidSim::CheckForCollision(const SolidSim& solid1, const SolidSim& solid2)
{
	if (solid1.IsGhostMode() || solid2.IsGhostMode())
	{
		return false;
	}

	bool res = true;

	for (int i = 0; i <3; ++i)
	{
		res = ((solid1.pos[i] + solid1.scale[i] / 2) - (solid2.pos[i] - solid2.scale[i] / 2)) > 0.0f &&
			  ((solid1.pos[i] - solid1.scale[i] / 2) - (solid2.pos[i] + solid2.scale[i] / 2)) < 0.0f;

		if (!res) break;
	}

	return res;
}

bool SolidSim::CheckForCollision(const ISolidSim& solid1, const ISolidSim& solid2)
{
	return CheckForCollision(solid1, solid2);
}

size_t SolidSim::GetMeshAmount()
{
	return STMM.GetMeshAmount();
}

void SolidSim::SetAllMeshVisibility(bool value)
{
	STMM.SetAllMeshVisibility(value);
}

void SolidSim::SetBackwardsModelAccess(SolidToModelManager::FullModelInfo& model)
{
	STMM.InitializeSTMM(model);
}

std::vector<std::string> SolidSim::GetModelMeshNames()
{
	return STMM.GetMeshNames();
}

void SolidSim::SetModelMeshVisibility(const std::string name, bool value)
{
	STMM.SetMeshVisibility(name, value);
}

void SolidSim::SetModelMeshVisibility(size_t index, bool value)
{
	STMM.SetMeshVisibility(index, value);
}

bool SolidSim::GetModelMeshVisibility(const std::string name)
{
	return STMM.GetMeshVisibility(name);
}

bool SolidSim::GetModelMeshVisibility(size_t index)
{
	return STMM.GetMeshVisibility(index);
}

float SolidSim::GetModelMeshShininess(const std::string& name)
{
	return STMM.GetMeshShininess(name);
}

float SolidSim::GetModelMeshShininess(size_t index)
{
	return STMM.GetMeshShininess(index);
}

std::vector<std::string> SolidSim::GetModelAnimationNames()
{
	return STMM.GetAnimationNames();
}

size_t SolidSim::GetModelAnimationAmount()
{
	return STMM.GetAnimationAmount();
}

void SolidSim::SetModelAnimation(size_t index)
{
	STMM.SetAnimation(index);
}

void SolidSim::SetModelAnimation(const std::string& name)
{
	STMM.SetAnimation(name);
}

size_t SolidSim::GetModelAnimation()
{
	return STMM.GetAnimation();
}

double SolidSim::GetModelAnimationSpeed()
{
	return STMM.GetAnimationSpeed();
}

void SolidSim::SetModelAnimationSpeed(double speed)
{
	STMM.SetAnimationSpeed(speed);
}

void SolidSim::PlayModelAnimation(bool loop)
{
	STMM.PlayAnimation(loop);
}

void SolidSim::PauseModelAnimation()
{
	STMM.PauseAnimation();
}

void SolidSim::StopModelAnimation()
{
	STMM.StopAnimation();
}

bool SolidSim::IsModelAnimationPlaying()
{
	return STMM.IsAnimationPlaying();
}

bool SolidSim::IsModelAnimationPaused()
{
	return STMM.IsAnimationPaused();
}

bool SolidSim::IsModelAnimationLooped()
{
	return STMM.IsAnimationLooped();
}

void SolidSim::SetModelAnimationPlaybackCallback(std::function<void(bool, bool, bool)> callback)
{
	STMM.SetAnimationPlaybackCallback(callback);
}

double SolidSim::GetModelCurrentAnimationTime()
{
	return STMM.GetCurrentAnimationTime();
}

void SolidSim::AppendModelStartingBoneIndex(size_t startingBoneIndex)
{
	STMM.AppendStartingBoneIndex(startingBoneIndex);
}

size_t SolidSim::GetModelCurrentStartingBoneIndex()
{
	return STMM.GetCurrentStartingBoneIndex();
}