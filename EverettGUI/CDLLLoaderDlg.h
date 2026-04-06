#pragma once

#include "afxdialogex.h"

#include "EverettEngine.h"

#include <vector>
#include <string>

#include "AdString.h"

class CDLLLoaderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDLLLoaderDlg)
public:
	CDLLLoaderDlg(
		std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
		EverettEngine& engineRef,
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
	CButton loadScriptButton;
	CButton unloadScriptButton;
	CComboBox dllComboBox;
private:
	EverettEngine& engineRef;

	constexpr static char LoadDLL[] = "Load DLL";
	constexpr static char UnloadDLL[] = "Unload DLL";

	void FillComboBoxWithScriptInfo();

	afx_msg void OnBrowseScriptButton();
	afx_msg void OnLoadScriptButtonClick();
	afx_msg void OnUnloadDllButtonClick();
	afx_msg void OnScriptSelectionChangeOk();
};