#pragma once

#include <vector>
#include <string>
#include <array>
#include <map>
#include "EverettEngine.h"
#include "MFCTreeManager.h"

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
	std::vector<std::pair<std::string, std::string>>& GetSelectedScriptDllInfo();
private:

	void OnInitialUpdate() override;

	EverettEngine* engineP;
	MFCTreeManager objectTree;

	std::vector<std::pair<std::string, std::string>> selectedScriptDllInfo;
public:
	afx_msg void OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
};


