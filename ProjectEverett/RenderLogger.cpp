#include "RenderLogger.h"

RenderLogger::RenderLogger(
	const float windowWidth,
	const float windowHeight,
	const LGLStructs::GlyphInfo& glyphs,
	const std::string& shader,
	ShaderBehaviourLog&& shaderBehaviourLog,
	ShaderBehaviourError&& shaderBehaviourError,
	RenderTextCreateFunc&& createFunc
)
	: 
	glyphs(glyphs), 
	shader(shader),
	shaderBehaviourLog(std::move(shaderBehaviourLog)),
	shaderBehaviourError(std::move(shaderBehaviourError)),
	createFunc(std::move(createFunc)),
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
	size_t textInfoCurrentSize = textInfoCollection.GetCurrentSize();
	
	if (textInfoCurrentSize == maxAmountOfMessages)
	{
		ScrollMessages();

		auto& lastTextInfo = textInfoCollection.GetBack();

		lastTextInfo.text = str;
		lastTextInfo.behaviour = behaviourToUse;
	}
	else
	{
		textInfoCollection.PushBack(
			LGLStructs::TextInfo{ str, GetCurrentTextPosition(), isRenderEnabled, shader, glyphs, behaviourToUse }
		);
		auto& newTextInfo = textInfoCollection.GetBack();

		renderMessageContentCollection.PushBack(
			std::pair<std::string*, std::function<void()>*>{ &newTextInfo.text, &newTextInfo.behaviour }
		);

		createFunc(std::to_string(textInfoCurrentSize), textInfoCollection.GetBack());
	}
}

void RenderLogger::ScrollMessages()
{
	for (size_t i = 0; i < renderMessageContentCollection.GetMaxSize() - 1; ++i)
	{
		*renderMessageContentCollection[i].first = std::move(*renderMessageContentCollection[i + 1].first);
		*renderMessageContentCollection[i].second = std::move(*renderMessageContentCollection[i + 1].second);
	}
}

glm::vec3 RenderLogger::GetCurrentTextPosition()
{
	glm::vec3 currentTextPos = startTextPos;
	currentTextPos.y -= textInfoCollection.GetCurrentSize() * 15.0f;

	return currentTextPos;
}

void RenderLogger::EnableRender(bool value)
{
	isRenderEnabled = value;

	for (auto& textInfo : textInfoCollection)
	{
		textInfo.render = isRenderEnabled;
	}
}

void RenderLogger::UpdateTextPos(float windowWidth, float windowHeight)
{
	startTextPos = CalcFirstTextPos(windowWidth, windowHeight);

	for (size_t i = 0; i < textInfoCollection.GetCurrentSize(); ++i)
	{
		textInfoCollection[i].position.y = startTextPos.y - i * 15.0f;
	}
}
