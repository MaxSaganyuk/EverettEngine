#pragma once

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "interfaces/ISolidSim.h"

class ScriptFuncStorage
{
public:
	using InterfaceScriptFunc = std::function<void(IObjectSim*)>;
	using ScriptFuncSharedPtr = std::shared_ptr<InterfaceScriptFunc>;
	using ScriptFuncWeakPtr = std::weak_ptr<InterfaceScriptFunc>;
	using ScriptFuncMainMap = std::map<std::string, ScriptFuncSharedPtr>;
	using ScriptFuncMap = std::map<std::string, std::pair<std::string, ScriptFuncWeakPtr>>;

	ScriptFuncStorage();

	void AddScriptFunc(const std::string& dllPath, const std::string& dllName, ScriptFuncWeakPtr scriptFunc);
	void ExecuteScriptFunc(IObjectSim* object, const std::string& dllName = "");
	void ExecuteAllScriptFuncs(IObjectSim* object);
	bool IsScriptFuncAdded(const std::string& dllName = "");
	bool IsScriptFuncRunnable(const std::string& dllName = "");
	void ClearScriptFuncMap();
	std::vector<std::pair<std::string, std::string>> GetAddedScriptDLLs() const;
	std::vector<std::pair<std::string, std::string>> GetTempScriptDllNameVect();

private:
	friend class ObjectSim;

	std::string lastExecutedScriptDll;
	ScriptFuncMap scriptFuncMap;
	std::vector<std::pair<std::string, std::string>> tempScriptDllNameVect;
};