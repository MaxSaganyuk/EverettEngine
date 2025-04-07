#pragma once
#include "afxdialogex.h"

#include <array>
#include <vector>

#include "EverettEngine.h"

#include "DLLLoaderCommon.h"

// CObjectEditDialog dialog

class CObjectEditDialog : public DLLLoaderCommon
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
	CString GenerateTitle();

	EverettEngine& engineRef;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine::ObjectTypes objectType;
	std::string subtypeName;
	std::string objectName;

	std::string chosenObjectName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateParamsButtonClick();
};
