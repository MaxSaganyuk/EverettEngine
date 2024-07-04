#pragma once

#include <type_traits>
#include <mutex>
#include <functional>


template<typename Context>
class ContextManager
{
	static std::recursive_mutex rMutex;
	std::function<void(Context*)> contextSetter;
public:
	ContextManager(Context* context, std::function<void(Context*)> contextSetter)
	{
		this->contextSetter = contextSetter;

		rMutex.lock();
		contextSetter(context);
	}

	~ContextManager()
	{
		contextSetter(nullptr);
		rMutex.unlock();
	}
};
