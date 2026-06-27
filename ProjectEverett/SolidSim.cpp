#include "SolidSim.h"

void SolidSim::ResetModelMatrix()
{
	model = glm::mat4(1.0f);

	const glm::vec3& posRef = pos;
	const glm::vec3& scaleRef = scale;
	const glm::quat& orientRef = orient;

	model = glm::translate(model, posRef);
	model *= glm::mat4_cast(orientRef);
	model = glm::scale(model, scaleRef);

	recalcInv = true;
}

SolidSim::SolidSim(
	const glm::vec3& pos,
	const glm::vec3& scale,
	const float speed
)
	: ObjectSim(pos, scale, speed)
{
	ResetModelMatrix();
}

void SolidSim::SetPositionVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	ObjectSim::SetPositionVector(vect, executeLinkedObjects);
	ResetModelMatrix();
}

void SolidSim::SetScaleVector(const glm::vec3& vect, bool executeLinkedObjects)
{
	ObjectSim::SetScaleVector(vect, executeLinkedObjects);
	ResetModelMatrix();
}

void SolidSim::SetOrientation(const glm::quat& quat, bool executeLinkedObjects)
{
	ObjectSim::SetOrientation(quat, executeLinkedObjects);
	ResetModelMatrix();
}

std::string SolidSim::GetThisObjectTypeNameStr()
{
	return TypeName;
}

std::string SolidSim::GetObjectTypeNameStr()
{
	return TypeName;
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
		res += SimSerializer::GetValueToSaveFrom(STMM.modelDefaultColor);
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
		res = res && SimSerializer::SetValueToLoadFrom(line, STMM.modelDefaultColor,              8);

		STMM.CheckIfModelVisible();
	}

	return res;
}

std::string SolidSim::GetSimInfoToSaveImpl()
{
	std::string res = ObjectSim::GetSimInfoToSaveImpl();

	res += SimSerializer::GetValueToSaveFrom(model);

	res += CollectInfoToSaveFromSTMM();

	recalcInv = true;

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
	int legacyInt;

	bool res = ObjectSim::SetSimInfoToLoad(line);

	res = res && SimSerializer::SetValueToLoadFrom(line, model,      1);
	res = res && SimSerializer::SetValueToLoadFrom(line, legacyInt,  1, 15);

	res = res && CollectInfoToLoadToSTMM(line);

	ResetModelMatrix();

	return res;
}

const glm::mat4& SolidSim::GetModelMatrixAddr()
{
	return model;
}

glm::mat4 SolidSim::GetInverseModelMatrix()
{
	if (recalcInv)
	{
		recalcInv = false;
		invModel = glm::inverse(model);
	}

	return invModel;
}

bool SolidSim::UpdateTransform()
{
	if (ObjectSim::UpdateTransform())
	{
		ResetModelMatrix();

		return true;
	}

	return false;
}

size_t SolidSim::GetMeshAmount()
{
	return STMM.GetMeshAmount();
}

void SolidSim::SetModelVisibility(bool value)
{
	STMM.SetModelVisibility(value);
}

bool SolidSim::GetModelVisibility()
{
	return STMM.GetModelVisibility();
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

void SolidSim::InvokeAutoScale()
{
	scale = STMM.GetAutoScaleVector();
}

std::vector<std::string> SolidSim::GetModelAnimationNames()
{
	return STMM.GetAnimationNames();
}

const std::string& SolidSim::GetModelName()
{
	return STMM.GetModelName();
}

glm::vec4 SolidSim::GetModelDefaultColor()
{
	return STMM.GetModelDefaultColor();
}

void SolidSim::SetModelDefaultColor(const glm::vec4& color)
{
	STMM.SetModelDefaultColor(color);
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
	STMM.SetAnimationPlaybackCallback(std::move(callback));
}

double SolidSim::GetModelCurrentAnimationTime()
{
	return STMM.GetCurrentAnimationTime();
}

void SolidSim::SetModelStartBoneIndexRef(const size_t& startingBoneIndexRef)
{
	STMM.SetStartBoneIndexRef(startingBoneIndexRef);
}

size_t SolidSim::GetModelCurrentStartingBoneIndex()
{
	return STMM.GetCurrentStartingBoneIndex();
}

size_t SolidSim::GetModelBoneAmount()
{
	return STMM.GetModelBoneAmount();
}

bool SolidSim::IsModelAnimationResetRequired()
{
	return STMM.IsAnimationResetRequired();
}