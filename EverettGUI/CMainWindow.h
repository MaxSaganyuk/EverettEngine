#pragma once

#include <vector>
#include <string>
#include <array>
#include <map>
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
	bool SetSelectedScriptDLLInfo(const std::vector<std::pair<std::string, std::string>>& scriptDLLInfo);
private:

	void OnInitialUpdate() override;
	HTREEITEM ForceNodeSelection();

	EverettEngine* engineP;
	MFCTreeManager objectTree;

	std::vector<std::pair<AdString, AdString>> selectedScriptDllInfo;

	static inline std::map<EverettEngine::ObjectTypes, int> validSubnodeAmount
	{
		{EverettEngine::ObjectTypes::Solid, 2 },
		{EverettEngine::ObjectTypes::Light, 2 },
		{EverettEngine::ObjectTypes::Sound, 1 }
	};
public:
	afx_msg void OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeRightClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditAmbientLightButton();
};


