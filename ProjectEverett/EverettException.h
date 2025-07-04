#pragma once

#include <stdexcept>
#include <source_location>
#include <string>

#define ThrowException                                     throw EverettException(std::source_location::current())
#define ThrowExceptionWMessage(message)                    throw EverettException(std::source_location::current(), message)
#define CheckAndThrowExceptionWMessage(condition, message) if(!condition) throw EverettException(std::source_location::current(), message)

class EverettException : public std::runtime_error
{
	std::string CreateTheMessage(const std::source_location& currentSourceLocation, const std::string& additionalMessage);

public:
	EverettException(const std::source_location& currentSourceLocation, const std::string& additionalMessage = "");
};