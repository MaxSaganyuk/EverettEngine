#pragma once

#include <unordered_map>
#include <string>

// The map will never change during runtime, no insertion or erasure will happen
// therefore no need for a more complex implementation of bidirectional map here
// This class is very limited deliberately, so that it's filled only during init
// The list for keys will not grow significantly, so double the size of a list
// is not a critical for memory
template<typename Key1, typename Key2>
class NaiveBidirMap
{
	static_assert(!std::is_same_v<Key1, Key2>, "Types can't be the same");

public:
	NaiveBidirMap(std::initializer_list<std::pair<Key1, Key2>> initList)
	{
		auto pairIter = initList.begin();
		
		for (int i = 0; i < initList.size(); ++i)
		{
			firstMap.emplace(pairIter[i].first, pairIter[i].second);
			secondMap.emplace(pairIter[i].second, pairIter[i].first);
		}
	}

	auto Get(const Key1& value)
	{
		return firstMap.find(value);
	}

	auto Get(const Key2& value)
	{
		return secondMap.find(value);
	}

	template<typename Key>
	auto End();

	template<>
	auto End<Key1>()
	{
		return firstMap.end();
	}

	template<>
	auto End<Key2>()
	{
		return secondMap.end();
	}

	bool Exists(const Key1& value) 
	{
		return firstMap.find(value) != firstMap.end();
	}

	bool Exists(const Key2& value) 
	{
		return secondMap.find(value) != secondMap.end();
	}

	Key1& operator[](const Key2& value) 
	{
		return secondMap[value];
	}

	Key2& operator[](const Key1& value) 
	{
		return firstMap[value];
	}


private:
	std::unordered_map<Key1, Key2> firstMap;
	std::unordered_map<Key2, Key1> secondMap;
};

class LGLKeyToStringMap
{
public:
	static NaiveBidirMap<int, char> keyToCharMap;
	static NaiveBidirMap<int, std::string> keyToStringMap;
	static NaiveBidirMap<int, std::string> mouseToStringMap;

	static std::string Get(int keyID)
	{
		if (auto iter = keyToCharMap.Get(keyID); iter != keyToCharMap.End<int>()) 
			return std::to_string(iter->second);
		if (auto iter = keyToStringMap.Get(keyID); iter != keyToStringMap.End<int>())
			return iter->second;
		if (auto iter = mouseToStringMap.Get(keyID); iter != mouseToStringMap.End<int>())
			return iter->second;

		return "InvalidKey";
	}

	static int Get(char c)
	{
		auto iter = keyToCharMap.Get(c);
		return iter != keyToCharMap.End<char>() ? iter->second : -1;
	}

	static int Get(const std::string& name)
	{
		if (name.size() == 1)
			return Get(name[0]);
		if (auto iter = keyToStringMap.Get(name); iter != keyToStringMap.End<std::string>())
			return iter->second;
		if (auto iter = mouseToStringMap.Get(name); iter != mouseToStringMap.End<std::string>())
			return iter->second;

		return -1;
	}
};