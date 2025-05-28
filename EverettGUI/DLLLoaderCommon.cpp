#include "pch.h"

#include "EverettGUI.h"

#include "DLLLoaderCommon.h"
#include "CBrowseDialog.h"

void DLLLoaderCommonCommon::InitializeDLLLoaderCommon(
	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfoTS,
	EverettEngine& engine,
	EverettEngine::ObjectTypes objectType,
	AdString subtypeName,
	AdString objectName
)
{
	selectedScriptDllInfo = &selectedScriptDllInfoTS;

	isScriptSetFunc = [=, &engine](const std::string& dllName) 
		{ return engine.IsObjectScriptSet(objectType, subtypeName, objectName, dllName); };
	setScriptFunc = [=, &engine](const std::string& dllPath, const std::string& dllName)
		{ return engine.SetScriptToObject(objectType, subtypeName, objectName, dllPath, dllName); };
	unsetScriptFunc = [=, &engine](const std::string& dllName)
		{ engine.UnsetScript(dllName); };
}

void DLLLoaderCommonCommon::FillComboBoxWithScriptInfo()
{
	for (auto& scriptDllInfo : *selectedScriptDllInfo)
	{
		dllComboBox.AddString(scriptDllInfo.second);
	}
}

void DLLLoaderCommonCommon::UpdateScriptButtons()
{
	bool isObjectScriptSet = isScriptSetFunc((*selectedScriptDllInfo)[dllComboBox.GetCurSel()].second);

	scriptRunIndicator.SetCheck(isObjectScriptSet);
	loadScriptButton.EnableWindow(!isObjectScriptSet);
	unloadScriptButton.EnableWindow(isObjectScriptSet);
}

BOOL DLLLoaderCommonCommon::OnInitDialog()
{
	if (selectedScriptDllInfo->size() > 0)
	{
		FillComboBoxWithScriptInfo();
		dllComboBox.SetCurSel(0);
		loadScriptButton.EnableWindow(true);
		scriptRunIndicator.SetCheck(isScriptSetFunc((*selectedScriptDllInfo)[0].second));
	}

	return true;
}

void DLLLoaderCommonCommon::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
	DDX_Control(pDX, IDC_BUTTON4, unloadScriptButton);
	DDX_Control(pDX, IDC_COMBO1, dllComboBox);
	DDX_Control(pDX, IDC_CHECK2, scriptRunIndicator);
}

void DLLLoaderCommonCommon::OnBrowseScriptButton()
{
	AdString pathStr;
	AdString fileStr;

	if (CBrowseDialog::OpenAndGetFilePath(pathStr, fileStr, {{"DLL files", "*.dll"}}));
	{
		selectedScriptDllInfo->push_back(std::pair<std::string, std::string>{ pathStr, fileStr });
		dllComboBox.AddString(fileStr);
		dllComboBox.SetCurSel(selectedScriptDllInfo->size() - 1);
		loadScriptButton.EnableWindow(true);
	}
}

void DLLLoaderCommonCommon::OnLoadScriptButtonClick()
{
	int curSel = dllComboBox.GetCurSel();

	setScriptFunc((*selectedScriptDllInfo)[curSel].first, (*selectedScriptDllInfo)[curSel].second);

	UpdateScriptButtons();
}

void DLLLoaderCommonCommon::OnUnloadDllButtonClick()
{
	unsetScriptFunc((*selectedScriptDllInfo)[dllComboBox.GetCurSel()].first);

	UpdateScriptButtons();
}

void DLLLoaderCommonCommon::OnScriptSelectionChangeOk()
{
	UpdateScriptButtons();
}