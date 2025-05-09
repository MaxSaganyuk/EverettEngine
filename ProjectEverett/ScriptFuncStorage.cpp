#include "ScriptFuncStorage.h"

ScriptFuncStorage::ScriptFuncStorage()
{
	lastExecutedScriptDll = "";
}

void ScriptFuncStorage::AddScriptFunc(const std::string& dllName, ScriptFuncWeakPtr scriptFunc)
{
	scriptFuncMap.emplace(dllName, scriptFunc);
	lastExecutedScriptDll = dllName;
}

void ScriptFuncStorage::ExecuteScriptFunc(IObjectSim* object, const std::string& dllName)
{
	if (!scriptFuncMap.empty())
	{
		const std::string& dllToExecute = dllName.empty() ? lastExecutedScriptDll : dllName;

		if (scriptFuncMap[dllToExecute].lock() && *scriptFuncMap[dllToExecute].lock().get())
		{
			((*scriptFuncMap[dllToExecute].lock())(object));
		}
	}
}

void ScriptFuncStorage::ExecuteAllScriptFuncs(IObjectSim* object)
{
	for (auto& scriptFuncPair : scriptFuncMap)
	{
		if (scriptFuncPair.second.lock() && *scriptFuncPair.second.lock().get())
		{
			((*scriptFuncPair.second.lock())(object));
		}
	}
}

bool ScriptFuncStorage::IsScriptFuncAdded(const std::string& dllName)
{
	return scriptFuncMap.find(dllName == "" ? lastExecutedScriptDll : dllName) != scriptFuncMap.end();
}

bool ScriptFuncStorage::IsScriptFuncRunnable(const std::string& dllName)
{
	return IsScriptFuncAdded(dllName) && *scriptFuncMap[dllName].lock().get();
}

std::vector<std::string> ScriptFuncStorage::GetAddedScriptDLLs() const
{
	std::vector<std::string> res;

	for (auto& scriptFuncPair : scriptFuncMap)
	{
		res.push_back(scriptFuncPair.first);
	}

	return res;
}