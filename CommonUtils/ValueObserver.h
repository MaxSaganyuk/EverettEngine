#pragma once

#include <functional>

template<typename Type>
class ValueObserver
{
	bool updateRequired{};

	Type lastValue{};
	Type currentValue{};
	Type accumulatedValue{};

	std::function<void()> valueUpdateCallback;

	void ExecuteValueUpdateCallback()
	{
		if (valueUpdateCallback)
		{
			valueUpdateCallback();
		}
	}

public:
	ValueObserver() = default;
	ValueObserver(const Type& value)
		: currentValue(value), lastValue(value) {}

	ValueObserver(const ValueObserver&) = delete;
	ValueObserver(ValueObserver&&) = delete;

	// Can't meaningfully reason about comparison
	bool operator<=>(const ValueObserver&) const = delete;

	void SetValueUpdateCallback(std::function<void()> callback) noexcept
	{
		valueUpdateCallback = std::move(callback);
	}

	// Accumulated setters for gradual framerate dependant change
	void operator+=(const Type& value)
	{
		updateRequired = true;
		accumulatedValue += value;
	}

	void operator-=(const Type& value)
	{
		updateRequired = true;
		accumulatedValue -= value;
	}

	// Instant setter for on demand framerate independant change
	void operator=(const Type& value)
	{
		lastValue = currentValue;
		currentValue = value;

		ExecuteValueUpdateCallback();
	}

	template<typename Self, typename... IndexerType> requires requires(Self&& self, IndexerType&&... index) {
		std::forward<Self>(self).currentValue[std::forward<IndexerType>(index)...];
	}
	decltype(auto) operator[](this Self&& self, IndexerType&&... index)
		noexcept(noexcept(std::forward<Self>(self).currentValue[std::forward<IndexerType>(index)...]))
	{
		return std::forward<Self>(self).currentValue[std::forward<IndexerType>(index)...];
	}

	bool Update()
	{
		constexpr static Type zeroedOutValue{};

		if (updateRequired)
		{
			updateRequired = false;

			lastValue = currentValue;
			currentValue += accumulatedValue;
			accumulatedValue = zeroedOutValue;

			ExecuteValueUpdateCallback();

			return true;
		}

		return false;
	}

	void SetLastValue()
	{
		currentValue = lastValue;

		ExecuteValueUpdateCallback();
	}

	const Type& GetLastValue() const noexcept
	{
		return lastValue;
	}

	// Underlying current data getter, value change tracking will not apply on non-const
	template<typename Self>
	auto&& GetValue(this Self&& self) noexcept
	{
		return std::forward<Self>(self).currentValue;
	}

	operator Type&() noexcept
	{
		return currentValue;
	}

	operator const Type&() const noexcept
	{
		return currentValue;
	}
};