#pragma once

#include <functional>

template<typename Type>
class ValueObserver
{
	Type value{};
	std::function<void()> callback;
public:
	ValueObserver(const Type& value)
		: value(value) {}

	void SetCallback(const std::function<void()>& callback)
	{
		this->callback = callback;
	}

	void ExecuteCallback()
	{
		if (callback)
		{
			callback();
		}
	}

	template<typename IndexerType> requires requires(IndexerType indexer) { value[indexer]; }
	auto& operator[](IndexerType index) 
	{ 
		return value[index]; 
	}

	template<typename IndexerType> requires requires(IndexerType indexer) { value[indexer]; }
	const auto operator[](IndexerType index) const
	{
		return value[index];
	}

	void operator=(const Type& otherValue)
	{
		value = otherValue;
		ExecuteCallback();
	} 
	
	void operator+=(const Type& otherValue) 
	{
		value += otherValue; 
		ExecuteCallback();
	} 
	
	void operator-=(const Type& otherValue) 
	{ 
		value -= otherValue; 
		ExecuteCallback();
	}

	operator Type&()
	{
		return value;
	}

	operator const Type&() const
	{
		return value;
	}
};