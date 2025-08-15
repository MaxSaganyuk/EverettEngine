#pragma once
#include "afxdialogex.h"

#include <array>
#include <vector>

#include "EverettEngine.h"

#include "DLLLoaderCommon.h"
#include "CObjectMoveDialog.h"

// CObjectEditDialog dialog

class CObjectEditDialog : public DLLLoaderCommon
{
	DECLARE_DYNAMIC(CObjectEditDialog)

public:
	CObjectEditDialog(
		EverettEngine& engine, 
		EverettEngine::ObjectTypes objectType,
		std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
		const std::vector<std::pair<AdString, AdString>>& selectedNodes = {},
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
	void SetPlayerButtons(bool play, bool pause, bool stop);

	void UpdateParams();
	void StartObjectMoveDlg(CObjectMoveDialog::ObjectTransformType transType);

	EverettEngine& engineRef;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine::ObjectTypes objectType;
	AdString subtypeName;
	AdString objectName;

	AdString chosenObjectName;
	IObjectSim& currentObjectInterface;
	ISolidSim* castedSolidInterface;
	ISoundSim* castedSoundInterface;

	// Model property objects
	CStatic meshText;
	CStatic modelPropText;
	CComboBox meshComboBox;
	CButton meshVisCheck;

	CStatic playerText;
	CComboBox playerComboBox;
	CButton playerPlayButton;
	CButton playerPauseButton;
	CButton playerStopButton;
	CButton playerLoopCheck;
	CStatic playerSpeedText;
	CEdit playerSpeedEdit;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateParamsButtonClick();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnMeshCBSelChange();
	afx_msg void OnPlayPlayerButtonClick();
	afx_msg void OnPausePlayerButtonClick();
	afx_msg void OnStopPlayerButtonClick();
	afx_msg void OnAnimCBSelChange();
	afx_msg void OnPosEditButtonClick();
	afx_msg void OnScaEditButtonClick();
	afx_msg void OnRotEditButtonClick();
};
