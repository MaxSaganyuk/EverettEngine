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
		const LGLStructs::GlyphInfo& glyphs,
		const std::string& shader,
		const ShaderBehaviourLog shaderBehaviourLog,
		const ShaderBehaviourError shaderBehaviourError,
		const RenderTextCreateFunc createFunc, 
		const RenderTextDeleteFunc deleteFunc
	);

	void CreateLogMessage(const std::string& str);
	void CreateErrorMessage(const std::string& str);

	std::streambuf* GetCustomLogOutputBuffer();
	std::streambuf* GetCustomErrorOutputBuffer();
private:
	void CreateMessage(const std::string& str, const std::function<void()>& behaviourToUse);

	glm::vec3 GetCurrentTextPosition();
	void ScrollMessages();

	class CustomOutput
	{
	private:
		class StreambufSubstitute : public std::streambuf
		{
		public:
			using EndlineCallbackFunc = std::function<void(const std::string&)>;

			void SetEndlineCallback(const EndlineCallbackFunc endlineCallback)
			{
				this->endlineCallback = endlineCallback;
			}

		private:
			EndlineCallbackFunc endlineCallback;
			std::string lineToSend;

			int overflow(int c) override
			{
				if (c == '\n')
				{
					endlineCallback(lineToSend);
					lineToSend = "";
				}
				else
				{
					lineToSend += static_cast<char>(c);
				}

				return c;
			}
		};

	public:
		CustomOutput()
		{
			ostream = std::make_unique<std::ostream>(&sSub);
		}

		std::streambuf* GetStreamBuffer()
		{
			return ostream->rdbuf();
		}

		void SetEndlineCallback(const StreambufSubstitute::EndlineCallbackFunc endlineCallback)
		{
			sSub.SetEndlineCallback(endlineCallback);
		}
	private:
		StreambufSubstitute sSub;
		std::unique_ptr<std::ostream> ostream;
	};

	// later should be made to be dynamic
	glm::vec3 startTextPos = { 20.0f, 575.0f, 1.0f };
	constexpr static int maxAmountOfMessages = 10; 

	CustomOutput logOutput;
	CustomOutput errorOutput;

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