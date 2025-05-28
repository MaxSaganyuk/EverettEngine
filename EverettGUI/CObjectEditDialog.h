#pragma once
#include "afxdialogex.h"

#include <array>
#include <vector>

#include "EverettEngine.h"

#include "DLLLoaderCommon.h"

// CObjectEditDialog dialog

class ObjectEditDialogCommon
{
protected:
	void SetObjectParams(const std::vector<glm::vec3>& params);
	CString GenerateTitle();
	void SetupModelParams();
	void SetAnimButtons(bool play, bool pause, bool stop);

	EverettEngine* engineRef;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine::ObjectTypes objectType;
	AdString subtypeName;
	AdString objectName;

	AdString chosenObjectName;
	IObjectSim* currentObjectInterface;
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
	CStatic animSpeedText;
	CEdit animSpeedEdit;

public:
	ObjectEditDialogCommon() = default;
	void InitializeObjectEditDialog(
		EverettEngine& engine,
		EverettEngine::ObjectTypes objectType
	);

	afx_msg void OnUpdateParamsButtonClick();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnMeshCBSelChange();
	afx_msg void OnPlayAnimButtonClick();
	afx_msg void OnPauseAnimButtonClick();
	afx_msg void OnStopAnimButtonClick();
	afx_msg void OnAnimCBSelChange();
protected:

	BOOL OnInitDialog();
	void DoDataExchange(CDataExchange* pDX);
};

template<typename BaseClass = CDialogEx>
class CObjectEditDialog : public ObjectEditDialogCommon, public BaseClass
{
public:
	CObjectEditDialog(int dialogID = IDD_DIALOG4) : BaseClass(dialogID) {};
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

protected:

	__pragma(warning(push)) __pragma(warning(disable : 4867)) template < typename BaseClass > const AFX_MSGMAP* GetMessageMap() const {
		return GetThisMessageMap();
	} template < typename BaseClass > const AFX_MSGMAP* __stdcall GetThisMessageMap() {
		typedef CObjectEditDialog< BaseClass > ThisClass; typedef BaseClass TheBaseClass; __pragma(warning(push)) __pragma(warning(disable: 4640)) static const AFX_MSGMAP_ENTRY _messageEntries[] = {
		ON_BN_CLICKED(IDC_BUTTON1, &ObjectEditDialogCommon::OnUpdateParamsButtonClick)
		ON_BN_CLICKED(IDC_CHECK3, &ObjectEditDialogCommon::OnBnClickedCheck3)
		ON_CBN_SELCHANGE(IDC_COMBO2, &ObjectEditDialogCommon::OnMeshCBSelChange)
		ON_BN_CLICKED(IDC_BUTTON5, &ObjectEditDialogCommon::OnPlayAnimButtonClick)
		ON_BN_CLICKED(IDC_BUTTON6, &ObjectEditDialogCommon::OnPauseAnimButtonClick)
		ON_BN_CLICKED(IDC_BUTTON7, &ObjectEditDialogCommon::OnStopAnimButtonClick)
		ON_CBN_SELCHANGE(IDC_COMBO3, &ObjectEditDialogCommon::OnAnimCBSelChange)
	END_MESSAGE_MAP()


	virtual void DoDataExchange(CDataExchange* pDX)
	{
		ObjectEditDialogCommon::DoDataExchange(pDX);
		BaseClass::DoDataExchange(pDX);
	}
private:
	BOOL OnInitDialog()
	{
		ObjectEditDialogCommon::OnInitDialog();
		BaseClass::OnInitDialog();

		//SetWindowText(GenerateTitle());
		
		return true;
	}
};
