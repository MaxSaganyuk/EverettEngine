#pragma once

#ifndef LGL_EXPORT
#error "LGLUniformLocator is LGL only" // No inline needed
#endif

#include <span>

#include <glad/glad.h>

#include "GLExecutor.h"

template<typename T>
struct UnwrapGLMType
{
	using Type = T;
	constexpr static bool Value = false;
};

template<size_t N, typename ValueType, glm::qualifier Q>
struct UnwrapGLMType<glm::vec<N, ValueType, Q>>
{
	using Type = ValueType;
	constexpr static bool Value = true;
};

template<size_t N, size_t K, typename ValueType, glm::qualifier Q>
struct UnwrapGLMType<glm::mat<N, K, ValueType, Q>>
{
	using Type = ValueType;
	constexpr static bool Value = true;
};

template<typename T>
struct UnwrapVector
{
	using Type = T;
	constexpr static bool Value = false;
};

template<typename T, typename Allocator>
struct UnwrapVector<std::vector<T, Allocator>>
{
	using Type = T;
	constexpr static bool Value = true;
};

template<typename ReturnType, typename... ParamTypes>
using FuncPtr = ReturnType(*)(ParamTypes...);

namespace _UniformLocatorUtils
{
	template<typename Type>
	using NonMatWrappedLocType = FuncPtr<void, int, int, const typename UnwrapGLMType<Type>::Type*>;

	template<typename Type>
	using MatWrappedLocType = FuncPtr<void, int, int, unsigned char, const typename UnwrapGLMType<Type>::Type*>;

	template<typename Type>
	using UnwrappedSpan = std::span<const typename UnwrapGLMType<Type>::Type>;

	template<typename Type>
	std::string wrappedLocatorName{};

	template<typename Type>
	NonMatWrappedLocType<Type> wrappedLocator{};

	template<typename Type>
	MatWrappedLocType<Type> matWrappedLocator{};

	bool initCheck{};

	template<typename Type>
	auto ProduceLambda(std::string name, NonMatWrappedLocType<Type> func)
	{
		wrappedLocatorName<Type> = std::move(name);
		wrappedLocator<Type> = func;

		return [](int uniformValueLocation, UnwrappedSpan<Type> value) -> bool {
			return GLExecutor::SafeExecute(
				wrappedLocatorName<Type>, wrappedLocator<Type>, uniformValueLocation, static_cast<int>(value.size()), value.data()
			);
		};
	}

	template<typename Type>
	auto ProduceLambda(std::string name, MatWrappedLocType<Type> func)
	{
		wrappedLocatorName<Type> = std::move(name);
		matWrappedLocator<Type> = func;

		return [](int uniformValueLocation, UnwrappedSpan<Type> value) -> bool {
			return GLExecutor::SafeExecute(
				wrappedLocatorName<Type>, matWrappedLocator<Type>, uniformValueLocation,
				static_cast<int>(value.size()), static_cast<unsigned char>(false), value.data()
			);
		};
	}

	template<typename Type> requires std::is_fundamental_v<Type>
	const typename UnwrapGLMType<Type>::Type* GetPtr(const Type& value)
	{
		return &value;
	}

	template<typename Type>
	const typename UnwrapGLMType<Type>::Type* GetPtr(const Type& value)
	{
		return glm::value_ptr(value);
	}
}

namespace UniformLocatorUtils
{
	using namespace _UniformLocatorUtils;

#define UniformLocatorInit(Type, Func) uniformLocatorFunc<Type> = ProduceLambda<Type>(#Func, Func);

	template<typename Type>
	FuncPtr<bool, int, UnwrappedSpan<Type>> uniformLocatorFunc{};

	void InitializeUniformLocators()
	{
		if (!initCheck)
		{
			initCheck = true;

			UniformLocatorInit(int,          glUniform1iv)
			UniformLocatorInit(unsigned int, glUniform1uiv)
			UniformLocatorInit(float,        glUniform1fv)
			UniformLocatorInit(glm::ivec2,   glUniform2iv)
			UniformLocatorInit(glm::ivec3,   glUniform3iv)
			UniformLocatorInit(glm::ivec4,   glUniform4iv)
			UniformLocatorInit(glm::uvec2,   glUniform2uiv)
			UniformLocatorInit(glm::uvec3,   glUniform3uiv)
			UniformLocatorInit(glm::uvec4,   glUniform4uiv)
			UniformLocatorInit(glm::vec2,    glUniform2fv)
			UniformLocatorInit(glm::vec3,    glUniform3fv)
			UniformLocatorInit(glm::vec4,    glUniform4fv)
			UniformLocatorInit(glm::mat2,    glUniformMatrix2fv)
			UniformLocatorInit(glm::mat3,    glUniformMatrix3fv)
			UniformLocatorInit(glm::mat4,    glUniformMatrix4fv)
		}
	}

	template<typename Type>
	UnwrappedSpan<Type> GetSpanForValue(const std::vector<Type>& value)
	{
		return { _UniformLocatorUtils::GetPtr(*value.data()), value.size() };
	}

	template<typename Type>
	UnwrappedSpan<Type> GetSpanForValue(const Type& value)
	{
		return { _UniformLocatorUtils::GetPtr(value), 1 };
	}

#undef UniformLocatorInit
}