#pragma once

#include <iostream>
#include <functional>

class CustomOutput
{
private:
	class StreambufSubstitute : public std::streambuf
	{
	public:
		using EndlineCallbackFunc = std::function<void(const std::string&)>;

		void SetEndlineCallback(const std::string& key, const EndlineCallbackFunc endlineCallback)
		{
			endlineCallbacks.emplace(key, endlineCallback);
		}

		void RemoveEndlineCallback(const std::string& key)
		{
			endlineCallbacks.erase(key);
		}

	private:
		std::map<std::string, EndlineCallbackFunc> endlineCallbacks;
		std::string lineToSend;

		int overflow(int c) override
		{
			if (c == '\n')
			{
				for (auto& [key, callback] : endlineCallbacks)
				{
					callback(lineToSend);
				}

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

	void SetEndlineCallback(const std::string& key, const StreambufSubstitute::EndlineCallbackFunc endlineCallback)
	{
		sSub.SetEndlineCallback(key, endlineCallback);
	}

	void RemoveEndlineCallback(const std::string& key)
	{
		sSub.RemoveEndlineCallback(key);
	}
private:
	StreambufSubstitute sSub;
	std::unique_ptr<std::ostream> ostream;
};