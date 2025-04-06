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

void ScriptFuncStorage::ExecuteScriptFunc(ISolidSim* object, const std::string& dllName)
{
	if (!scriptFuncMap.empty())
	{
		const std::string& dllToExecute = dllName.empty() ? lastExecutedScriptDll : dllName;

		if (scriptFuncMap[dllToExecute].lock())
		{
			((*scriptFuncMap[dllToExecute].lock())(object));
		}
		else
		{
			scriptFuncMap.erase(dllToExecute);
		}

	}
}

void ScriptFuncStorage::ExecuteAllScriptFuncs(ISolidSim* object)
{
	for (auto& scriptFuncPair : scriptFuncMap)
	{
		if (scriptFuncPair.second.lock())
		{
			((*scriptFuncPair.second.lock())(object));
		}
	}
}

bool ScriptFuncStorage::IsScriptFuncAdded(const std::string& dllName)
{
	return scriptFuncMap.find(dllName == "" ? lastExecutedScriptDll : dllName) != scriptFuncMap.end();
}
