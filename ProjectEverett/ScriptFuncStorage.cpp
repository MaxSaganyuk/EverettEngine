#include "ScriptFuncStorage.h"

ScriptFuncStorage::ScriptFuncStorage()
{
	lastExecutedScriptDll = "";
}

void ScriptFuncStorage::AddScriptFunc(
	const std::string& dllPath, 
	const std::string& dllName, 
	ScriptFuncWeakPtr scriptFunc
)
{
	scriptFuncMap.try_emplace(dllName, dllPath, scriptFunc);
	lastExecutedScriptDll = dllName;
}

void ScriptFuncStorage::ExecuteScriptFunc(IObjectSim* object, const std::string& dllName)
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

void ScriptFuncStorage::ExecuteAllScriptFuncs(IObjectSim* object)
{
	for (auto& scriptFuncPair : scriptFuncMap)
	{
		if (scriptFuncPair.second.second.lock() && *scriptFuncPair.second.second.lock().get())
		{
			((*scriptFuncPair.second.second.lock())(object));
		}
	}
}

bool ScriptFuncStorage::IsScriptFuncAdded(const std::string& dllName)
{
	return scriptFuncMap.find(dllName == "" ? lastExecutedScriptDll : dllName) != scriptFuncMap.end();
}

bool ScriptFuncStorage::IsScriptFuncRunnable(const std::string& dllName)
{
	return IsScriptFuncAdded(dllName) && *scriptFuncMap[dllName].second.lock().get();
}

void ScriptFuncStorage::ClearScriptFuncMap()
{
	scriptFuncMap.clear();
}