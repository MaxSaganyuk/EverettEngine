#pragma once

#include <functional>
#include <string>
#include <memory>
#include <map>

using InterfaceScriptFunc = std::function<void(void*)>;
using ScriptFuncSharedPtr = std::shared_ptr<InterfaceScriptFunc>;
using ScriptFuncWeakPtr = std::weak_ptr<InterfaceScriptFunc>;
using ScriptFuncMainMap = std::map<std::string, ScriptFuncSharedPtr>;
using ScriptFuncMap = std::map<std::string, std::pair<std::string, ScriptFuncWeakPtr>>;

class _ScriptFuncStorageImpl
{
public:
	void AddScriptFunc(const std::string& dllPath, const std::string& dllName, ScriptFuncWeakPtr scriptFunc);
	bool IsScriptFuncAdded(const std::string& dllName = "");
	bool IsScriptFuncRunnable(const std::string& dllName = "");
	void ClearScriptFuncMap();
protected:
	void ExecuteScriptFuncImpl(void* object, const std::string& dllName);
	void ExecuteAllScriptFuncsImpl(void* object);

	friend class ObjectSim;

	std::string lastExecutedScriptDll;
	ScriptFuncMap scriptFuncMap;
};

/*
*  By forcing ScriptFuncStorage class to be a class template instead of only applying template to "Execute" methods
*  we raise a compile time suggestion that each storage instance is 
*  linked to a script func with a specific non-interchangeable parameter type
*  reducing chances of accidental misuse of generalized void* paramed InterfaceScriptFunc 
*/
template<typename ParamType>
class ScriptFuncStorage : public _ScriptFuncStorageImpl
{
public:
	void ExecuteScriptFunc(ParamType* object, const std::string& dllName = "")
	{
		ExecuteScriptFuncImpl(object, dllName);
	}

	void ExecuteAllScriptFuncs(ParamType* object)
	{
		ExecuteAllScriptFuncsImpl(object);
	}
};

template<>
class ScriptFuncStorage<void> : public _ScriptFuncStorageImpl
{
public:
	void ExecuteScriptFunc(const std::string& dllName = "")
	{
		ExecuteScriptFuncImpl(nullptr, dllName);
	}

	void ExecuteAllScriptFuncs()
	{
		ExecuteAllScriptFuncsImpl(nullptr);
	}
};