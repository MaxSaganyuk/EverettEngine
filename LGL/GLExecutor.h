#pragma once

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <functional>
#include <cassert>
#include <unordered_map>

#define GLSafeExecute(glFunc, ...) GLExecutor::SafeExecute(#glFunc, glFunc, __VA_ARGS__)

class GLExecutor
{
	static std::unordered_map<unsigned int, std::string> errorMessages;
	static bool assertOnFailure;

public:
	template<typename GLFunc, typename... Types>
	static bool SafeExecute(const std::string& annotation, GLFunc glFunc, Types... values)
	{
		unsigned int error;

		glFunc(values...);

		while((error = glGetError()) != GL_NO_ERROR)
		{
			std::cerr << "OpenGL ERROR:" + errorMessages[error] + (annotation.size() ? " comment: " + annotation : "") + '\n';
			assert(!assertOnFailure && "OpenGL ERROR: check cmd");
		}

		return !error;
	}

	static void SetAssertOnFailure(bool value)
	{
		assertOnFailure = value;
	}
};

std::unordered_map<unsigned int, std::string> GLExecutor::errorMessages
{
	{GL_INVALID_ENUM, "An invalid enum value was used in a function."},
	{GL_INVALID_VALUE, "A numeric argument is out of range"},
	{GL_INVALID_OPERATION, "The operation is not allowed in the current state"},
	//{GL_STACK_OVERFLOW, "An attempt to push onto a stack caused it to overflow."},
	//{GL_STACK_UNDERFLOW, "An attempt to pop off a stack caused it to underflow"},
	{GL_OUT_OF_MEMORY, "Not enough memory to execute the command"}

};

bool GLExecutor::assertOnFailure = false;