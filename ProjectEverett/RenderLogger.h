#pragma once

#include "LGLStructs.h"
#include "stdEx/utilityEx.h"

class RenderLogger
{
public:
	using ShaderBehaviourLog   = std::function<void()>;
	using ShaderBehaviourError = std::function<void()>;
	using RenderTextCreateFunc = std::function<void(const std::string&, LGLStructs::TextInfo&)>;

	RenderLogger(
		const float windowWidth,
		const float windowHeight,
		const LGLStructs::GlyphInfo& glyphs,
		const std::string& shader,
		ShaderBehaviourLog&& shaderBehaviourLog,
		ShaderBehaviourError&& shaderBehaviourError,
		RenderTextCreateFunc&& createFunc
	);

	void CreateLogMessage(const std::string& str);
	void CreateErrorMessage(const std::string& str);

	void EnableRender(bool value = true);
	void UpdateTextPos(float windowWidth, float windowHeight);
private:
	void CreateMessage(const std::string& str, const std::function<void()>& behaviourToUse);

	glm::vec3 CalcFirstTextPos(float windowWidth, float windowHeight);
	glm::vec3 GetCurrentTextPosition();
	void ScrollMessages();

	glm::vec3 startTextPos;
	constexpr static int maxAmountOfMessages = 10; 

	const LGLStructs::GlyphInfo& glyphs;
	std::string shader;
	ShaderBehaviourLog shaderBehaviourLog;
	ShaderBehaviourError shaderBehaviourError;
	RenderTextCreateFunc createFunc;

	bool isRenderEnabled;

	// std::inplace_vector would be better here, but that is only in C++26
	stdEx::RingBuffer<LGLStructs::TextInfo, maxAmountOfMessages> textInfoCollection;
	stdEx::RingBuffer<std::pair<std::string*, std::function<void()>*>, maxAmountOfMessages> renderMessageContentCollection;
};