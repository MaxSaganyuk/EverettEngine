#include "ScriptFuncStorage.h"

void _ScriptFuncStorageImpl::AddScriptFunc(
	const std::string& dllPath, const std::string& dllName, ScriptFuncWeakPtr scriptFunc
)
{
	scriptFuncMap.try_emplace(dllName, dllPath, scriptFunc);
	lastExecutedScriptDll = dllName;
}

void _ScriptFuncStorageImpl::ExecuteScriptFuncImpl(void* object, const std::string& dllName)
{
	if (!scriptFuncMap.empty())
	{
		const std::string& dllToExecute = dllName.empty() ? lastExecutedScriptDll : dllName;

		if (scriptFuncMap[dllToExecute].second.lock() && *scriptFuncMap[dllToExecute].second.lock().get())
		{
			((*scriptFuncMap[dllToExecute].second.lock())(object));
			lastExecutedScriptDll = dllToExecute;
		}
	}
}

void _ScriptFuncStorageImpl::ExecuteAllScriptFuncsImpl(void* object)
{
	for (auto& scriptFuncPair : scriptFuncMap)
	{
		if (scriptFuncPair.second.second.lock() && *scriptFuncPair.second.second.lock().get())
		{
			((*scriptFuncPair.second.second.lock())(object));
		}
	}
}

bool _ScriptFuncStorageImpl::IsScriptFuncAdded(const std::string& dllName)
{
	return scriptFuncMap.find(dllName == "" ? lastExecutedScriptDll : dllName) != scriptFuncMap.end();
}

bool _ScriptFuncStorageImpl::IsScriptFuncRunnable(const std::string& dllName)
{
	return IsScriptFuncAdded(dllName) && *scriptFuncMap[dllName].second.lock().get();
}

void _ScriptFuncStorageImpl::ClearScriptFuncMap()
{
	scriptFuncMap.clear();
}