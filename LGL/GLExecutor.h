#pragma once

#ifndef LGL_EXPORT
#error "GLExecutor is LGL only"
#endif

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <functional>
#include <cassert>
#include <unordered_map>

#define GLSafeExecute(glFunc, ...) GLExecutor::SafeExecute(#glFunc, glFunc, __VA_ARGS__)
#define GLSafeExecuteRet(glFunc, ...) GLExecutor::SafeExecuteWithReturn(#glFunc, glFunc, __VA_ARGS__)

class GLExecutor
{
	static std::unordered_map<GLenum, std::string> errorMessages;
	static bool assertOnFailure;

	template<typename FunctionType>
	struct ReturnTypeGetter;

	template<typename ReturnType, typename... Types>
	struct ReturnTypeGetter<ReturnType(*)(Types...)>
	{
		using Type = ReturnType;
	};

	static bool CheckForGLErrors(const std::string& annotation)
	{
#ifdef _DEBUG
		GLenum errorID{};
		bool error = false;

		while ((errorID = glGetError()) != GL_NO_ERROR)
		{
			error = true;
			std::cerr <<
				"OpenGL ERROR:" << errorMessages[errorID] << (annotation.size() ? " comment: " + annotation : "") << '\n';
			assert(!assertOnFailure && "OpenGL ERROR: check log");
		}

		return error;
#else
		return false;
#endif
	}

public:
	template<typename GLFunc, typename... Types>
	static bool SafeExecute(const std::string& annotation, GLFunc glFunc, Types&&... values)
	{
		glFunc(std::forward<Types>(values)...);

		return !CheckForGLErrors(annotation);
	}

	template<typename GLFunc, typename... Types>
	static typename ReturnTypeGetter<GLFunc>::Type SafeExecuteWithReturn(
		const std::string& annotation, 
		GLFunc glFunc, 
		Types&&... values
	)
	{
		typename ReturnTypeGetter<GLFunc>::Type res = glFunc(std::forward<Types>(values)...);

		CheckForGLErrors(annotation);

		return res;
	}

	static void SetAssertOnFailure(bool value)
	{
		assertOnFailure = value;
	}
};

std::unordered_map<GLenum, std::string> GLExecutor::errorMessages
{
	{GL_INVALID_ENUM, "An invalid enum value was used in a function."},
	{GL_INVALID_VALUE, "A numeric argument is out of range"},
	{GL_INVALID_OPERATION, "The operation is not allowed in the current state"},
	//{GL_STACK_OVERFLOW, "An attempt to push onto a stack caused it to overflow."},
	//{GL_STACK_UNDERFLOW, "An attempt to pop off a stack caused it to underflow"},
	{GL_OUT_OF_MEMORY, "Not enough memory to execute the command"}

};

bool GLExecutor::assertOnFailure = false;