#pragma once

#include "afxdialogex.h"

#include "EverettEngine.h"

#include <vector>
#include <string>

#include "AdString.h"

// *sigh*
class DLLLoaderCommonCommon
{
	// Function wrappers to be passed will include every other variable except for directly
// dll related info in lambda capture

// Should accept dll name
	using IsScriptSetFunc = std::function<bool(const std::string&)>;

	// Should accept dll name and path
	using SetScriptFunc = std::function<void(const std::string&, const std::string&)>;

	// Should accept dll name
	using UnsetScriptFunc = std::function<void(const std::string&)>;


protected:
	std::vector<std::pair<AdString, AdString>>* selectedScriptDllInfo;

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

	BOOL OnInitDialog();
	void DoDataExchange(CDataExchange* pDX);

public:
	DLLLoaderCommonCommon() = default;
	void InitializeDLLLoaderCommon(
		std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfoTS,
		EverettEngine& engine,
		EverettEngine::ObjectTypes objectType,
		AdString subtypeName,
		AdString objectName
	);
};

template<typename BaseClass = CDialogEx>
class DLLLoaderCommon : public DLLLoaderCommonCommon, public BaseClass
{
public: 
	DLLLoaderCommon(int dialogID = -1) : BaseClass(dialogID) {}
protected:
	virtual void DoDataExchange(CDataExchange* pDX)
	{
		DLLLoaderCommonCommon::DoDataExchange(pDX);
		BaseClass::DoDataExchange(pDX);
	}

	BOOL OnInitDialog()
	{
		DLLLoaderCommonCommon::OnInitDialog();
		BaseClass::OnInitDialog();

		return true;
	}

	__pragma(warning(push)) __pragma(warning(disable : 4867)) template < typename BaseClass > const AFX_MSGMAP* GetMessageMap() const {
		return GetThisMessageMap();
	} template < typename BaseClass > const AFX_MSGMAP* __stdcall GetThisMessageMap() {
		typedef DLLLoaderCommon< BaseClass > ThisClass; typedef BaseClass TheBaseClass; __pragma(warning(push)) __pragma(warning(disable: 4640)) static const AFX_MSGMAP_ENTRY _messageEntries[] = {
		ON_BN_CLICKED(IDC_BUTTON2, &DLLLoaderCommonCommon::OnBrowseScriptButton)
		ON_BN_CLICKED(IDC_BUTTON3, &DLLLoaderCommonCommon::OnLoadScriptButtonClick)
		ON_BN_CLICKED(IDC_BUTTON4, &DLLLoaderCommonCommon::OnUnloadDllButtonClick)
		ON_CBN_SELENDOK(IDC_COMBO1, &DLLLoaderCommonCommon::OnScriptSelectionChangeOk)
	END_MESSAGE_MAP()
};