#include "AutoReindexer.h"

AutoIndex::AutoIndex(const size_t* ptr) noexcept
	: indexPtr(ptr) {
}

AutoIndex::operator bool() const noexcept
{
	return indexPtr;
}

AutoIndex::operator const size_t() const
{
	if (!indexPtr) throw std::exception("AutoIndexer was not initialized");

	return *indexPtr;
}

void AutoReindexer::PushIncrementBy(size_t value)
{
	list.push_back(list.empty() ? 0 : list.back() + value);
}

void AutoReindexer::PushValueBack(size_t value)
{
	if (!list.empty() && list.back() >= value)
	{
		throw std::exception("Pushed value must be evergrowing");
	}
	else
	{
		list.push_back(value);
	}
}

AutoIndex AutoReindexer::GetBackIndex() const noexcept
{
	return AutoIndex{ &list.back() };
}

void AutoReindexer::RemoveIndex(size_t index, size_t reduction)
{
	if (list.empty()) return;

	auto endIter = list.end();
	auto beforeCurrentIter = endIter;

	bool indexFound{};

	for (auto currentIter = list.begin(); currentIter != endIter;)
	{
		if (indexFound)
		{
			(*currentIter) -= reduction;
		}

		if (!indexFound && *currentIter == index)
		{
			currentIter = list.erase(currentIter);

			if (currentIter == endIter) break;

			bool beforeCurrentSet = beforeCurrentIter != endIter;

			if (reduction > *currentIter || (beforeCurrentSet && reduction >= *currentIter - *beforeCurrentIter))
				throw std::exception("Reduction value is too large");

			indexFound = true;
		}
		else
		{
			beforeCurrentIter = currentIter++;
		}
	}
}

bool AutoReindexer::IsEmpty() const noexcept
{
	return list.empty();
}

void AutoReindexer::Clean() noexcept
{
	list.clear();
}