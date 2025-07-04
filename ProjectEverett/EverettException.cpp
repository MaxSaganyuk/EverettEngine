#include "EverettException.h"

#include <iostream>
#include <cassert>

std::string EverettException::CreateTheMessage(
	const std::source_location& currentSourceLocation, 
	const std::string& additionalMessage
)
{
	std::cerr << ("Critical Error: " + additionalMessage + '\n');

	std::string message = "FileName: ";
	std::string_view fileName = currentSourceLocation.file_name();
	message += fileName.substr(fileName.rfind('\\') + 1, std::string::npos);

	std::string_view funcName = currentSourceLocation.function_name();
	funcName = funcName.substr(0, funcName.find('('));
	funcName = funcName.substr(funcName.rfind(':') + 1, std::string::npos);
	message += " FuncName: ";
	message += funcName;

	message += " LineNum: " + std::to_string(currentSourceLocation.line());

	std::cerr << (message + '\n');

	message = additionalMessage + " - " + message;

	assert(false && "Critical error, see error message");

	return message;
}

EverettException::EverettException(const std::source_location& currentSourceLocation, const std::string& additionalMessage)
	: std::runtime_error(CreateTheMessage(currentSourceLocation, additionalMessage).c_str()) {}