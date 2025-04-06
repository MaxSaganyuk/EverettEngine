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
	static NaiveBidirMap<int, std::string> keyToStringMap;
};