#pragma once

#include "afxdialogex.h"

#include "EverettEngine.h"

#include <vector>

#include "AdString.h"
#include "CTrackedDlg.h"

class CDLLLoaderDlg : public CTrackedDlg
{
	DECLARE_DYNAMIC(CDLLLoaderDlg)
public:
	CDLLLoaderDlg(
		std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
		EverettEngine& engineRef,
		std::function<void()> scriptChangeCallback,
		CWnd* pParent = nullptr
	);
	virtual ~CDLLLoaderDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL OnInitDialog();
protected:
	DECLARE_MESSAGE_MAP()

	void UpdateScriptButtons();

	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo;

	CButton scriptBrowseButton;
	CButton preloadScriptButton;
	CButton loadScriptButton;
	CButton unloadScriptButton;
	CComboBox dllComboBox;
	CButton resetStateCheck;
	CButton applyToAllCheck;
private:
	EverettEngine& engineRef;
	std::function<void()> scriptChangeCallback;

	constexpr static char LoadDLL[] = "Load DLL";
	constexpr static char UnloadDLL[] = "Unload DLL";

	void FillComboBoxWithScriptInfo();
	void ExecuteScriptsAccordingToRule(std::function<bool(int)> func);

	afx_msg void OnBrowseScriptButton();
	afx_msg void OnPreloadDLLButtonClick();
	afx_msg void OnLoadScriptButtonClick();
	afx_msg void OnUnloadDllButtonClick();
	afx_msg void OnScriptSelectionChangeOk();
};