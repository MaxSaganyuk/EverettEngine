#pragma once

#include <list>

// Abstraction encapsulating ptr get to ensure it was produced only in AutoReindexer
class AutoIndex
{
	friend class AutoReindexer;

	const size_t* indexPtr{};
	
	AutoIndex(const size_t* ptr) noexcept;
public:
	AutoIndex() noexcept = default;

	AutoIndex(const AutoIndex&) = default;
	AutoIndex& operator=(const AutoIndex&) = default;
	AutoIndex(AutoIndex&& value) noexcept = delete;
	AutoIndex& operator=(AutoIndex&&) noexcept = delete;

	operator bool() const noexcept;
	operator const size_t() const;
};

class AutoReindexer
{
	std::list<size_t> list;
public:
	AutoReindexer() = default;

	AutoReindexer(const AutoReindexer&) = delete;
	AutoReindexer& operator=(const AutoReindexer&) = delete;
	AutoReindexer(AutoReindexer&& value) noexcept = delete;
	AutoReindexer& operator=(AutoReindexer&&) noexcept = delete;

	void PushIncrementBy(size_t value);
	void PushValueBack(size_t value);

	AutoIndex GetBackIndex() const noexcept;

	void RemoveIndex(size_t index, size_t reduction);

	bool IsEmpty() const noexcept;
	void Clean() noexcept;
};