#pragma once

#include "LGLStructs.h"

#include <iostream>
#include <memory>

class RenderLogger
{
public:
	using ShaderBehaviourLog   = std::function<void()>;
	using ShaderBehaviourError = std::function<void()>;
	using RenderTextCreateFunc = std::function<void(const std::string&, LGLStructs::TextInfo&)>;
	using RenderTextDeleteFunc = std::function<void(const std::string&)>;

	RenderLogger(
		const float windowWidth,
		const float windowHeight,
		const LGLStructs::GlyphInfo& glyphs,
		const std::string& shader,
		const ShaderBehaviourLog shaderBehaviourLog,
		const ShaderBehaviourError shaderBehaviourError,
		const RenderTextCreateFunc createFunc, 
		const RenderTextDeleteFunc deleteFunc
	);

	void CreateLogMessage(const std::string& str);
	void CreateErrorMessage(const std::string& str);
private:
	void CreateMessage(const std::string& str, const std::function<void()>& behaviourToUse);

	glm::vec3 GetCurrentTextPosition();
	void ScrollMessages();

	glm::vec3 startTextPos;
	int maxAmountOfMessages; 

	const LGLStructs::GlyphInfo& glyphs;
	std::string shader;
	ShaderBehaviourLog shaderBehaviourLog;
	ShaderBehaviourError shaderBehaviourError;
	RenderTextCreateFunc createFunc;
	RenderTextDeleteFunc deleteFunc;

	size_t counter;
	//std::vector<std::string> pureMessageCollection; Output to file additionally needed as well
	std::list<std::pair<size_t, LGLStructs::TextInfo>> renderMessageCollection;
};