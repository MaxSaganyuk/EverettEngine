#pragma once

#include <type_traits>
#include <mutex>
#include <functional>

template<typename Context>
class ContextManager
{
	static std::recursive_mutex rMutex;
	static size_t counter;
	std::function<void(Context*)> contextSetter;
public:
	ContextManager(Context* context, std::function<void(Context*)> contextSetter)
	{
		this->contextSetter = contextSetter;

		rMutex.lock();
		if (!counter)
		{
			contextSetter(context);
		}
		++counter;
	}

	~ContextManager()
	{
		--counter;
		if (!counter)
		{
			contextSetter(nullptr);
		}
		rMutex.unlock();
	}
};
