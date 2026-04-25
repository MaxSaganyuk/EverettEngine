#pragma once

#include <stdexcept>
#include <source_location>
#include <string>
#include <functional>

// Outside the engine dll, for catching only
class EverettException : public std::runtime_error
{
	std::string CreateTheMessage(const std::source_location& currentSourceLocation, const std::string& additionalMessage);

	static inline std::function<void()> logReportCreatorFunc;
public:
	EverettException(const std::source_location& currentSourceLocation, const std::string& additionalMessage = "");

	static void SetLogReportCreator(std::function<void()> logReportCreatorFuncToSet);
};