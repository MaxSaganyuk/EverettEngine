#pragma once

#include <type_traits>
#include <mutex>
#include <functional>

template<typename Context>
class ContextManager
{
	static inline std::recursive_mutex rMutex;
	static inline size_t counter{};
	static inline std::function<void(Context*)> contextSetter;
public:
	ContextManager(Context* context)
	{
		if (contextSetter)
		{
			rMutex.lock();
			if (!counter)
			{
				contextSetter(context);
			}
			++counter;
		}
	}

	~ContextManager()
	{
		if (contextSetter)
		{
			--counter;
			if (!counter)
			{
				contextSetter(nullptr);
			}
			rMutex.unlock();
		}
	}

	static void SetContextSetter(std::function<void(Context*)> contextSetter)
	{
		ContextManager<Context>::contextSetter = contextSetter;
	}
};
