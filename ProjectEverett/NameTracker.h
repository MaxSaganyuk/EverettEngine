#pragma once

#include "CommonStrEdits.h"

#include <string>
#include <unordered_map>

class NameTracker
{
private:
	struct NameInfo
	{
		bool taken = true;
		size_t counter = 1;
	};

	std::unordered_map<std::string, NameInfo> nameInfo;

public:
	bool Contains(const std::string& name) const
	{
		return nameInfo.contains(CommonStrEdits::RemoveDigitsFromStringEnd(name));
	}

	void Add(const std::string& name)
	{
		std::string namePure = CommonStrEdits::RemoveDigitsFromStringEnd(name);

		if (auto iter = nameInfo.find(namePure); iter != nameInfo.end())
		{
			auto& [taken, counter] = iter->second;

			taken ? ++counter : taken = true;
		}
		else
		{
			nameInfo.try_emplace(namePure);
		}
	}

	std::string GetAvailibleName(const std::string& name) const
	{
		std::string namePure = CommonStrEdits::RemoveDigitsFromStringEnd(name);

		if (auto iter = nameInfo.find(namePure); iter != nameInfo.end())
		{
			auto& [taken, number] = iter->second;
			
			if (taken) return (namePure + std::to_string(number));
		}

		return namePure;
	}

	size_t GetCountForName(const std::string& name) const
	{
		if (auto iter = nameInfo.find(CommonStrEdits::RemoveDigitsFromStringEnd(name)); iter != nameInfo.end())
		{
			auto& [_, counter] = iter->second;

			return counter;
		}

		return 0;
	}

	void TryRemove(const std::string& name)
	{
		if (auto iter = nameInfo.find(name); iter != nameInfo.end())
		{
			iter->second.taken = false;
		}
	}

	void Clean() noexcept
	{
		nameInfo.clear();
	}
};
