#pragma once
#include "afxdialogex.h"

#include <array>
#include <vector>

#include "EverettEngine.h"


// CObjectEditDialog dialog

class CObjectEditDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CObjectEditDialog)

public:
	CObjectEditDialog(
		EverettEngine& engine, 
		EverettEngine::ObjectTypes objectType,
		std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
		const std::vector<std::pair<std::string, std::string>>& selectedNodes = {},
		CWnd* pParent = nullptr
	);
	virtual ~CObjectEditDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

private:
	BOOL OnInitDialog() override;

	void SetObjectParams(const std::vector<glm::vec3>& params);
	void FillComboBoxWithScriptInfo();
	void UpdateScriptButtons();

	EverettEngine& engineRef;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine::ObjectTypes objectType;
	std::string subtypeName;
	std::string objectName;

	CButton scriptBrowseButton;
	CButton loadScriptButton;
	CButton unloadScriptButton;
	CComboBox dllComboBox;
	CButton scriptRunIndicator;

	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo;
	std::string chosenObjectName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateParamsButtonClick();
	afx_msg void OnBrowseScriptButton();
	afx_msg void OnLoadScriptButtonClick();
	afx_msg void OnUnloadDllButtonClick();
	afx_msg void OnScriptSelectionChange();
	afx_msg void OnScriptSelectionChangeOk();
};
