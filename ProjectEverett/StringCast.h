#pragma once

#include <string>

#include "stdEx/stringEx.h"

#include "EverettExceptionInternal.h"
#include "ConceptUtils.h"

namespace StringCast
{
	using CastedString = stdEx::FixedString<32>;
}

namespace _StringCast
{
	using namespace StringCast;

	constexpr void ThrowExc(std::errc errorCode)
	{
		using UnderlyingErrc = std::underlying_type_t<std::errc>;

		constexpr size_t maxIntSizeDigitAmount = std::numeric_limits<UnderlyingErrc>::digits10 + 2; // plus max and plus "-" sign
		constexpr const char errorMesBase[] = "Failed to get from string during parsing, error: ";

		stdEx::FixedString<sizeof(errorMesBase) + maxIntSizeDigitAmount> errorMes{ errorMesBase };
		stdEx::FixedString<maxIntSizeDigitAmount + 1> str;

		auto [lineEndPtr, _] = std::to_chars(str.Begin(), str.Begin() + str.GetBufferSize(), std::to_underlying(errorCode));
		str.SetSize(lineEndPtr - str.Begin());

		errorMes += str;

		throw EverettException(std::source_location::current(), errorMes.View());
	}

	template<typename Type>
	constexpr Type FromString(const std::string_view str)
	{
		Type value{};

		auto [_, errorCode] = std::from_chars(str.data(), str.data() + str.size(), value);
		
		if (errorCode != std::errc{})
		{
			ThrowExc(errorCode);
		}

		CheckAndThrowExceptionWMessage(
			(errorCode == std::errc()),
			"Failed to get from string during deserialization, error: "
		);

		return value;
	}

	template<typename Type>
	constexpr CastedString ToString(const Type value)
	{
		CastedString castedStr;

		auto [lineEndPtr, errorCode] = std::to_chars(
			castedStr.Begin(), castedStr.Begin() + castedStr.GetBufferSize(), value
		);
		castedStr.SetSize(lineEndPtr - castedStr.Begin());

		if (errorCode != std::errc{})
		{
			ThrowExc(errorCode);
		}

		return castedStr;
	}
}

namespace StringCast
{
	// As for C++23, floating point types cannot be parsed by from_chars or to_chars at compile time
	template<OnlyIntegralNotBool Type>
	constexpr Type FromString(const std::string_view str)
	{
		return _StringCast::FromString<Type>(str);
	}

	template<std::floating_point Type>
	Type FromString(const std::string_view str)
	{
		return _StringCast::FromString<Type>(str);
	}

	template<typename>
	constexpr bool FromString(const std::string_view str)
	{
		return _StringCast::FromString<int>(str);
	}

	template<OnlyIntegralNotBool Type>
	constexpr CastedString ToString(const Type value)
	{
		return _StringCast::ToString(value);
	}

	template<std::floating_point Type>
	CastedString ToString(const Type value)
	{
		return _StringCast::ToString(value);
	}

	template<typename>
	constexpr CastedString ToString(const bool value)
	{
		return _StringCast::ToString(static_cast<int>(value));
	}
};