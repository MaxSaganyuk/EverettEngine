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
	void SetupModelParams();
	void SetAnimButtons(bool play, bool pause, bool stop);

	EverettEngine& engineRef;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine::ObjectTypes objectType;
	std::string subtypeName;
	std::string objectName;

	std::string chosenObjectName;
	IObjectSim& currentObjectInterface;
	ISolidSim* castedCurrentObject;

	// Model property objects
	CStatic meshText;
	CStatic modelPropText;
	CComboBox meshComboBox;
	CButton meshVisCheck;

	CStatic animText;
	CComboBox animComboBox;
	CButton animPlayButton;
	CButton animPauseButton;
	CButton animStopButton;
	CButton animLoopCheck;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateParamsButtonClick();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnMeshCBSelChange();
	afx_msg void OnPlayAnimButtonClick();
	afx_msg void OnPauseAnimButtonClick();
	afx_msg void OnStopAnimButtonClick();
	afx_msg void OnAnimCBSelChange();
};
