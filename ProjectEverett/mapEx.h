#ifndef MAPEX
#define MAPEX

#include <map>

#include <stdexcept>

namespace stdEx
{
	template<typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
	class map : public std::map<Key, Value, Compare, Allocator>
	{
		Value defaultValue;
		bool  defaultValueSet = false;

	public:
		Value& at(const Key& key)
		{
			// To reduce amount of catches, if default value is not set
			// will call std::map::at unhandled as usual

			if (defaultValueSet)
			{
				try
				{
					return std::map<Key, Value>::at(key);
				}
				catch (std::out_of_range&)
				{
					return defaultValue;
				}
			}
			else
			{
				return std::map<Key, Value>::at(key);
			}
		}

		void SetDefaultValue(const Value& value)
		{
			defaultValue = value;
			defaultValueSet = true;
		}

	};

	template<class Key, class Value, class Compare = std::less<Key>>
	class multimap : public std::multimap<Key, Value, Compare>
	{
	public:
		std::vector<Value> operator[](const Key&& index)
		{
			std::vector<Value> res{};
			bool found = false;

			for (auto iter = this->begin(); iter != this->end(); ++iter)
			{
				if (iter->first == index)
				{
					res.push_back(iter->second);
					found = true;
				}
				else if (found)
				{
					break;
				}
			}

			return res;
		}
	};
}

#endif