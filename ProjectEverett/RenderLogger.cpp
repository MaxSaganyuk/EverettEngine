#include "RenderLogger.h"

#include <thread>

RenderLogger::RenderLogger(
	const float windowWidth,
	const float windowHeight,
	const LGLStructs::GlyphInfo& glyphs,
	const std::string& shader,
	const ShaderBehaviourLog shaderBehaviourLog,
	const ShaderBehaviourError shaderBehaviourError,
	const RenderTextCreateFunc createFunc,
	const RenderTextDeleteFunc deleteFunc
)
	: 
	glyphs(glyphs), 
	shader(shader),
	shaderBehaviourLog(shaderBehaviourLog),
	shaderBehaviourError(shaderBehaviourError),
	createFunc(createFunc), 
	deleteFunc(deleteFunc),
	counter(0)
{
	startTextPos = { 20.0f, windowHeight - 25.0f, 1.0f };
	maxAmountOfMessages = static_cast<int>(windowHeight / 60.0f);
}


void RenderLogger::CreateLogMessage(const std::string& str)
{
	CreateMessage(str, shaderBehaviourLog);
}

void RenderLogger::CreateErrorMessage(const std::string& str)
{
	CreateMessage(str, shaderBehaviourError);
}

void RenderLogger::CreateMessage(const std::string& str, const std::function<void()>& behaviourToUse)
{
	if (renderMessageCollection.size() == maxAmountOfMessages)
	{
		ScrollMessages();
	}

	renderMessageCollection.push_back(
		{ counter++, { str, GetCurrentTextPosition(), true, shader, glyphs, behaviourToUse} }
	);
	createFunc(std::to_string(renderMessageCollection.back().first), renderMessageCollection.back().second);
}

void RenderLogger::ScrollMessages()
{
	deleteFunc(std::to_string(renderMessageCollection.front().first));
	renderMessageCollection.pop_front();

	for (auto& message : renderMessageCollection)
	{
		message.second.position.y += 15.0f;
	}
}

glm::vec3 RenderLogger::GetCurrentTextPosition()
{
	glm::vec3 currentTextPos = startTextPos;
	currentTextPos.y -= renderMessageCollection.size() * 15.0f;

	return currentTextPos;
}
