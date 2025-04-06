#pragma once

#include "afxdialogex.h"

#include "EverettEngine.h"

#include <vector>
#include <string>

class DLLLoaderCommon : public CDialogEx
{
	DECLARE_DYNAMIC(DLLLoaderCommon)
public:
	// Function wrappers to be passed will include every other variable except for directly
	// dll related info in lambda capture

	// Should accept dll name
	using IsScriptSetFunc = std::function<bool(const std::string&)>;
	
	// Should accept dll name and path
	using SetScriptFunc   = std::function<void(const std::string&, const std::string&)>;
	
	// Should accept dll name
	using UnsetScriptFunc = std::function<void(const std::string&)>;

	DLLLoaderCommon(
		int dialogID,
		std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo, 
		IsScriptSetFunc isScriptSetFunc,
		SetScriptFunc setScriptFunc,
		UnsetScriptFunc unsetScriptFunc,
		CWnd* pParent = nullptr
	);
	virtual ~DLLLoaderCommon();

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL OnInitDialog();
protected:
	DECLARE_MESSAGE_MAP()
private:
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo;

	IsScriptSetFunc isScriptSetFunc;
	SetScriptFunc setScriptFunc;
	UnsetScriptFunc unsetScriptFunc;

	CButton scriptBrowseButton;
	CButton loadScriptButton;
	CButton unloadScriptButton;
	CComboBox dllComboBox;
	CButton scriptRunIndicator;

	void FillComboBoxWithScriptInfo();
	void UpdateScriptButtons();

	afx_msg void OnBrowseScriptButton();
	afx_msg void OnLoadScriptButtonClick();
	afx_msg void OnUnloadDllButtonClick();
	afx_msg void OnScriptSelectionChangeOk();
};