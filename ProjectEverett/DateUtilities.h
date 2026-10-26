#pragma once

#include <array>
#include <string>
#include <optional>

#include "EverettExceptionInternal.h"

#include "StringCast.h"

// std::chrono is disappointing
namespace DateUtilities
{
	constexpr size_t monthsInAYear = 12;

	// Better use case could be C++26 contracts, but we are not seeing those for a while.
	// Instead, bounds check is provided at construction of day/month/year abstraction
	// It is ok to pass by value, Day, Month and Year abstractions because their sizes are equivalent to underlying 
	// values sizes. 
	// Mutating these values is not needed for now and could invalidate bounds check, so only observation is allowed
	class Year
	{
		size_t year{};

	public:
		constexpr Year(size_t year)
			: year(year)
		{
			CheckAndThrowExceptionWMessage(year, "Year cannot be zero");
		}
		constexpr Year(std::string_view str)
			: Year(StringCast::FromString<size_t>(str)) {}

		constexpr Year(const Year&) noexcept = default;
		constexpr Year& operator=(const Year&) noexcept = default;

		constexpr operator size_t() const noexcept
		{
			return year;
		}
	};

	class Month
	{
		size_t month{};

		constexpr size_t GetMonthIndexByStr(std::string_view str)
		{
			// std::map is not constexpr
			constexpr static std::array<const char[4], monthsInAYear> monthNames{
				"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
			};

			for (size_t i = 0; i < monthNames.size(); ++i)
			{
				if (str == monthNames[i]) return i + 1;
			}

			ThrowExceptionWMessage("No such month name exists");
		}

	public:
		constexpr Month(size_t month)
			: month(month)
		{
			CheckAndThrowExceptionWMessage((month > 0 && month <= monthsInAYear), "Month must be from 1 to 12");
		}
		constexpr Month(std::string_view str)
			: month(GetMonthIndexByStr(str)) {}

		constexpr Month(const Month&) noexcept = default;
		constexpr Month& operator=(const Month&) noexcept = default;

		constexpr operator size_t() const noexcept
		{
			return month;
		}
	};

	constexpr size_t GetDaysInMonth(Month month, std::optional<Year> year = std::nullopt) noexcept
	{
		constexpr static std::array<size_t, monthsInAYear> daysInMonths{
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		};

		bool isLeapYear = year && (*year % 4 == 0 && *year % 100 != 0 || *year % 400 == 0);

		return month == 2 && isLeapYear ? 29 : daysInMonths[month - 1];
	}

	class Day
	{
		size_t day{};

	public:
		constexpr Day(size_t day, Month month, std::optional<Year> year = std::nullopt)
			: day(day)
		{
			CheckAndThrowExceptionWMessage(
				(day > 0 && day <= GetDaysInMonth(month, year)), "Invalid amount of days for month and year"
			);
		}
		constexpr Day(std::string_view str, Month month, std::optional<Year> year = std::nullopt)
			: Day(StringCast::FromString<size_t>(str), month, year) {}

		constexpr Day(const Day&) noexcept = default;
		constexpr Day& operator=(const Day&) noexcept = default;

		constexpr operator size_t() const noexcept
		{
			return day;
		}
	};

	constexpr auto GetDaysPassedInMonthsArr(std::optional<Year> year = std::nullopt) noexcept
	{
		std::array<size_t, monthsInAYear> daysPassed{};
		size_t sum{};

		for (size_t i = 0; i < monthsInAYear; ++i)
		{
			daysPassed[i] += sum;
			sum += GetDaysInMonth(i + 1, year);
		}

		return daysPassed;
	}

	constexpr size_t GetDaysPassedSinceAnnoDomini(Day day, Month month, Year year) noexcept
	{
		auto daysPassed = GetDaysPassedInMonthsArr(year);

		size_t fullYearsPassed = year - 1;
		size_t leapDaysPassed = (year / 4) - (year / 100) + (year / 400);

		return 365 * fullYearsPassed + leapDaysPassed + daysPassed[month - 1] + day;
	}

	constexpr size_t GetDaysPassedFromDateString(std::string_view str)
	{
		// "May 11 1999"
		//  0123456789AB

		Year year{ str.substr(7, 4) };
		Month month{ str.substr(0, 3) };
		Day day{ str.substr(4, 2), month, year };

		return GetDaysPassedSinceAnnoDomini(day, month, year);
	}
}