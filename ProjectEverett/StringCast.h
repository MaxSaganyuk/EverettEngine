#pragma once

#include <string>

#include "EverettException.h"
#include "ConceptUtils.h"

class StringCast
{
	constexpr static size_t ConverterBufferSize = 1024;
	static inline char ConverterBuffer[ConverterBufferSize];
public:
	template<OnlyFundamentalNotBool Type>
	static Type FromString(const std::string_view str)
	{
		Type value{};

		auto [_, errorCode] = std::from_chars(str.data(), str.data() + str.size(), value);

		CheckAndThrowExceptionWMessage(
			static_cast<bool>(errorCode == std::errc()),
			"Failed to get from string during deserialization, error: " + std::to_string(static_cast<int>(errorCode))
		);

		return value;
	}

	template<OnlyFundamentalNotBool Type>
	static std::string ToString(const Type value)
	{
		auto [lineEndPtr, errorCode] = std::to_chars(ConverterBuffer, ConverterBuffer + ConverterBufferSize, value);

		CheckAndThrowExceptionWMessage(
			static_cast<bool>(errorCode == std::errc()),
			"Failed to get from value during serialization, error: " + std::to_string(static_cast<int>(errorCode))
		);

		return std::string(ConverterBuffer, lineEndPtr);
	}

	template<typename>
	static bool FromString(const std::string_view str)
	{
		return FromString<int>(str);
	}

	template<typename>
	static std::string ToString(const bool value)
	{
		return ToString(static_cast<int>(value));
	}
};