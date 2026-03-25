#pragma once

#include "afxdialogex.h"

#include "EverettEngine.h"

#include <vector>
#include <string>

#include "AdString.h"

class DLLLoaderCommon : public CDialogEx
{
	DECLARE_DYNAMIC(DLLLoaderCommon)
public:
	// Function wrappers to be passed will include every other variable except for directly
	// dll related info in lambda capture

	// Should accept dll path
	using IsDLLLoadedFunc = std::function<bool(const std::string&)>;

	// Should accept dll name
	using IsScriptSetFunc = std::function<bool(const std::string&)>;
	
	// Should accept dll name and path
	using SetScriptFunc   = std::function<void(const std::string&, const std::string&)>;
	
	// Should accept dll name
	using UnsetScriptFunc = std::function<void(const std::string&)>;

	DLLLoaderCommon(
		int dialogID,
		std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
		IsDLLLoadedFunc isDllLoadedFunc,
		IsScriptSetFunc isScriptSetFunc,
		SetScriptFunc setScriptFunc,
		UnsetScriptFunc unsetScriptFunc,
		bool blockLoadButton = false,
		CWnd* pParent = nullptr
	);
	virtual ~DLLLoaderCommon();

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL OnInitDialog();
protected:
	DECLARE_MESSAGE_MAP()

	void UpdateScriptButtons();
	void BlockLoadScriptButton(bool value = true);

	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo;

	CButton scriptBrowseButton;
	CButton loadScriptButton;
	CButton unloadScriptButton;
	CComboBox dllComboBox;
	CButton scriptRunIndicator;
private:
	IsDLLLoadedFunc isDllLoadedFunc;
	IsScriptSetFunc isScriptSetFunc;
	SetScriptFunc setScriptFunc;
	UnsetScriptFunc unsetScriptFunc;

	bool blockLoadButton;

	constexpr static char LoadDLLStr[] = "Load DLL";
	constexpr static char BindFuncStr[] = "Bind Func";

	void FillComboBoxWithScriptInfo();

	afx_msg void OnBrowseScriptButton();
	afx_msg void OnLoadScriptButtonClick();
	afx_msg void OnUnloadDllButtonClick();
	afx_msg void OnScriptSelectionChangeOk();
};