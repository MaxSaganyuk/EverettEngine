#include "RenderLogger.h"

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
	counter(0),
	isRenderEnabled(true)
{
	startTextPos = CalcFirstTextPos(windowWidth, windowHeight);
}

glm::vec3 RenderLogger::CalcFirstTextPos(float windowsWidth, float windowHeight)
{
	return { 20.0f, windowHeight - 25.0f, 1.0f };
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
	if (renderMessageCollection.GetCurrentSize() == maxAmountOfMessages)
	{
		ScrollMessages();
	}

	renderMessageCollection.PushBack(
		{ counter++, { str, GetCurrentTextPosition(), isRenderEnabled, shader, glyphs, behaviourToUse} }
	);
	createFunc(std::to_string(renderMessageCollection.GetBack().first), renderMessageCollection.GetBack().second);
}

void RenderLogger::ScrollMessages()
{
	deleteFunc(std::to_string(renderMessageCollection.GetFront().first));
	renderMessageCollection.PopFront();

	for (size_t i = 0; i < renderMessageCollection.GetCurrentSize(); ++i)
	{
		renderMessageCollection[i].second.position.y += 15.0f;
	}
}

glm::vec3 RenderLogger::GetCurrentTextPosition()
{
	glm::vec3 currentTextPos = startTextPos;
	currentTextPos.y -= renderMessageCollection.GetCurrentSize() * 15.0f;

	return currentTextPos;
}

void RenderLogger::EnableRender(bool value)
{
	isRenderEnabled = value;

	for (size_t i = 0; i < renderMessageCollection.GetCurrentSize(); ++i)
	{
		renderMessageCollection[i].second.render = isRenderEnabled;
	}
}

void RenderLogger::UpdateTextPos(float windowWidth, float windowHeight)
{
	startTextPos = CalcFirstTextPos(windowWidth, windowHeight);

	for (size_t i = 0; i < renderMessageCollection.GetCurrentSize(); ++i)
	{
		renderMessageCollection[i].second.position.y = startTextPos.y - i * 15.0f;
	}
}
