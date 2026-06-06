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
		return nameInfo.contains(name);
	}

	void Add(const std::string& name)
	{
		int number{};
		std::string namePure = CommonStrEdits::RemoveDigitsFromStringEnd(name, number);

		if (auto iter = nameInfo.find(namePure); iter != nameInfo.end())
		{
			auto& [taken, counter] = iter->second;

			taken ? ++counter : taken = true;
		}
		else
		{
			nameInfo.emplace(namePure, NameInfo{});
		}
	}

	std::string GetAvailibleName(const std::string& name) const
	{
		if (auto iter = nameInfo.find(name); iter != nameInfo.end())
		{
			auto& [taken, number] = iter->second;
			
			if (taken) return (name + std::to_string(number));
		}

		return name;
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
