#pragma once

#include <vector>
#include <map>
#include <generator>

#include "EverettEngine.h"
#include "MFCTreeManager.h"
#include "AdString.h"

// CMainWindow form view

class CMainWindow : public CFormView
{
	DECLARE_DYNCREATE(CMainWindow)

	CMainWindow();           // protected constructor used by dynamic creation
	virtual ~CMainWindow();
protected:

public:
	BOOL Create(
		LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, 
		DWORD dwStype, 
		const RECT& rect, 
		CWnd*, 
		UINT, 
		CCreateContext*
	) override;
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetEverettEngineRef(EverettEngine& engineRef);
	MFCTreeManager& GetObjectTree();
	std::vector<std::pair<AdString, AdString>>& GetSelectedScriptDllInfo();
	bool SetSelectedScriptDLLInfo(std::generator<EverettStructs::BasicFileInfo>&& scriptDLLInfo);
private:

	void OnInitialUpdate() override;
	HTREEITEM ForceNodeSelection();
	void Panic(const char* errorMessage);

	EverettEngine* engineP;
	MFCTreeManager objectTree;

	struct NodeInfo
	{
		HTREEITEM rawNode;
		std::optional<EverettEngine::ObjectTypes> selectedType;
		std::vector<std::pair<AdString, AdString>> selectedSubnodes;

		void Clean()
		{
			rawNode = nullptr;
			selectedType.reset();
			selectedSubnodes.clear();
		}
	};

	NodeInfo currentNodeInfo;
	std::vector<std::pair<AdString, AdString>> selectedScriptDllInfo;

	static inline std::map<EverettEngine::ObjectTypes, int> validSubnodeAmount
	{
		{EverettEngine::ObjectTypes::Solid,    2},
		{EverettEngine::ObjectTypes::Light,    2},
		{EverettEngine::ObjectTypes::Sound,    1},
		{EverettEngine::ObjectTypes::Collider, 1}
	};

	CButton showGizmoCheck;
	CButton showDebugTextCheck;
	CButton wasdControlsCheck;

	constexpr static int RenameNode = WM_USER + 4;
	constexpr static int DeleteNode = WM_USER + 5;
public:
	afx_msg void OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeRightClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRenameNode();
	afx_msg void OnDeleteNode();
	afx_msg void OnEditAmbientLightButton();
	afx_msg void OnEditBackgroundColorButtonClick();
	afx_msg void OnShowGizmoCheckClick();
	afx_msg void OnShowDebugTextCheckClick();
	afx_msg void OnWASDControlsCheckClick();
};


