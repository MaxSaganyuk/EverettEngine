#pragma once

#include <source_location>
#include <string>
#include <functional>

// Outside the engine dll, for catching only
class EverettException
{
	void CreateTheMessage(const std::source_location& currentSourceLocation, const std::string& additionalMessage);

	static inline std::function<void()> logReportCreatorFunc;
public:
	EverettException(const std::source_location& currentSourceLocation, std::string_view additionalMessage = "");

	static void SetLogReportCreator(std::function<void()> logReportCreatorFuncToSet);
};