#pragma once

#include <iostream>
#include <functional>
#include <list>

class CustomOutput
{
private:
	class StreambufSubstitute final : public std::streambuf
	{
	public:
		using EndlineCallbackFunc = std::function<void(const std::string&)>;
	private:
		static inline thread_local std::string lineToSend;

		std::map<std::string, std::pair<EndlineCallbackFunc, bool>> endlineCallbacks;
		std::list<std::string> delayedOutputList;
		bool manualOutputEnabled = false;
		std::mutex mux;

		int overflow(int c) override
		{
			if (c == '\n')
			{
				ExecuteCallbacks(false);

				if (manualOutputEnabled)
				{
					std::lock_guard lock(mux);
					delayedOutputList.push_back(lineToSend);
				}

				lineToSend.clear();
			}
			else
			{
				lineToSend += static_cast<char>(c);
			}

			return c;
		}
	public:
		using EndlineCallbackFunc = std::function<void(const std::string&)>;

		void SetEndlineCallback(
			const std::string& key, const StreambufSubstitute::EndlineCallbackFunc endlineCallback, bool manualExecution
		)
		{
			if (manualExecution)
			{
				manualOutputEnabled = true;
			}

			endlineCallbacks.emplace(key, std::pair<EndlineCallbackFunc, bool>{ endlineCallback, manualExecution });
		}

		void RemoveEndlineCallback(const std::string& key) noexcept
		{
			endlineCallbacks.erase(key);
		}

		void ExecuteCallbacks(bool manual)
		{
			if (manual && delayedOutputList.empty()) return;

			for (auto& [_, callbackPair] : endlineCallbacks)
			{
				if (manualOutputEnabled && manual)
				{
					if (callbackPair.second)
					{
						std::lock_guard lock(mux);

						for (auto& str : delayedOutputList)
						{
							callbackPair.first(str);
						}

						delayedOutputList.clear();
					}
				}
				else if (!callbackPair.second)
				{
					callbackPair.first(lineToSend);
				}
			}
		}
	};

public:
	CustomOutput()
	{
		ostream = std::make_unique<std::ostream>(&sSub);
	}

	std::streambuf* GetStreamBuffer() const noexcept
	{
		return ostream->rdbuf();
	}

	void SetEndlineCallback(
		const std::string& key, const StreambufSubstitute::EndlineCallbackFunc endlineCallback, bool manualExecution = false
	)
	{
		sSub.SetEndlineCallback(key, endlineCallback, manualExecution);
	}

	void RemoveEndlineCallback(const std::string& key) noexcept
	{
		sSub.RemoveEndlineCallback(key);
	}

	void ExecuteManualCallbacks()
	{
		sSub.ExecuteCallbacks(true);
	}
private:
	StreambufSubstitute sSub;
	std::unique_ptr<std::ostream> ostream;
};