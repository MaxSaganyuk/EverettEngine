#pragma once

#include <vector>
#include <string>
#include <array>
#include <map>
#include "EverettEngine.h"
#include "TreeManager.h"

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
	afx_msg void OnUpdateParamsClick();

	void SetEverettEngineRef(EverettEngine& engineRef);
	TreeManager& GetObjectTree();
private:
	void UpdateModelChoice();
	void SetObjectParams(const std::vector<glm::vec3>& params);
	void ClearObjectParams();

	void OnInitialUpdate() override;

	std::array<std::array<CEdit, 3>, 3> objectInfoEdits;

	EverettEngine* engineP;
	TreeManager objectTree;
public:
	afx_msg void OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
};


