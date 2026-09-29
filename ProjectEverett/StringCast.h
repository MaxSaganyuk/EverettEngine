#pragma once

#include <string>

#include "EverettExceptionInternal.h"
#include "ConceptUtils.h"

namespace StringCast
{
	template<OnlyFundamentalNotBool Type>
	Type FromString(const std::string_view str)
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
	std::string ToString(const Type value)
	{
		constexpr size_t ConverterBufferSize = 32;
		char ConverterBuffer[ConverterBufferSize];

		auto [lineEndPtr, errorCode] = std::to_chars(ConverterBuffer, ConverterBuffer + ConverterBufferSize, value);

		CheckAndThrowExceptionWMessage(
			static_cast<bool>(errorCode == std::errc()),
			"Failed to get from value during serialization, error: " + std::to_string(static_cast<int>(errorCode))
		);

		return std::string(ConverterBuffer, lineEndPtr);
	}

	template<typename>
	bool FromString(const std::string_view str)
	{
		return FromString<int>(str);
	}

	template<typename>
	std::string ToString(const bool value)
	{
		return ToString(static_cast<int>(value));
	}
};