#pragma once

#include <array>
#include <string>

#include "EverettExceptionInternal.h"

#include "StringCast.h"

// std::chrono is disappointing
namespace DateUtilities
{
	consteval auto GetDaysPassedInMonthsArr()
	{
		constexpr static std::array<size_t, 12> daysInMonths{ // excluding leap
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		};

		std::array<size_t, daysInMonths.size()> daysPassed{};
		size_t sum{};

		for (size_t i = 0; i < daysInMonths.size(); ++i)
		{
			daysPassed[i] += sum;
			sum += daysInMonths[i];
		}

		return daysPassed;
	}

	constexpr size_t GetMonthIndexByStr(std::string_view str)
	{
		// std::map is not constexpr
		constexpr static std::array<const char[4], 12> monthNames{
			"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};

		for (size_t i = 0; i < monthNames.size(); ++i)
		{
			if (str == monthNames[i]) return i + 1;
		}

		ThrowExceptionWMessage("No such month name exists");
	}

	constexpr size_t GetDaysPassedInMonths(size_t month, bool isLeapYear)
	{
		constexpr static auto daysPassedByMonth = GetDaysPassedInMonthsArr();

		return daysPassedByMonth[month] + (month > 1 && isLeapYear);
	}

	constexpr size_t GetDaysPassedSinceAnnoDomini(size_t day, size_t month, size_t year)
	{
		size_t fullYearsPassed = year - 1;
		size_t leapDaysPassed = (year / 4) - (year / 100) + (year / 400);
		bool isLeapYear = year % 4 == 0 && year % 100 != 0 && year % 400 == 0;

		return 365 * fullYearsPassed + leapDaysPassed + GetDaysPassedInMonths(month - 1, isLeapYear) + day;
	}

	constexpr size_t GetDaysPassedFromDateString(std::string_view str)
	{
		// "May 11 1999"
		//  0123456789AB

		size_t month = GetMonthIndexByStr(str.substr(0, 3));
		size_t day = StringCast::FromString<size_t>(str.substr(4, 2));
		size_t year = StringCast::FromString<size_t>(str.substr(7, 4));

		return GetDaysPassedSinceAnnoDomini(day, month, year);
	}
}