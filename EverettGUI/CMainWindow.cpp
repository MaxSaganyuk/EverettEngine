// CMainWindow.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "CMainWindow.h"
#include "CBrowseDialog.h"
#include "CObjectEditDialog.h"


// CMainWindow

IMPLEMENT_DYNCREATE(CMainWindow, CFormView)

CMainWindow::CMainWindow()
	: CFormView(IDD_FORMVIEW)
{
	engineP = nullptr;
}

CMainWindow::~CMainWindow()
{
	engineP = nullptr;
}

TreeManager& CMainWindow::GetObjectTree()
{
	return objectTree;
}

void CMainWindow::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	objectTree.SetObjectTypes(EverettEngine::GetAllObjectTypeNames(), engineP->GetLightTypeList());
	objectTree.GetTreeCtrl().ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
}

BOOL CMainWindow::Create(
	LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName,
	DWORD dwStype,
	const RECT& rect,
	CWnd* pParentWnd,
	UINT nID,
	CCreateContext* pContext
)
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStype, rect, pParentWnd, nID, pContext);
}

void CMainWindow::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, objectTree.GetTreeCtrl());
}

BEGIN_MESSAGE_MAP(CMainWindow, CFormView)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMainWindow::OnTreeSelectionChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CMainWindow::OnNodeDoubleClick)
END_MESSAGE_MAP()

// CMainWindow diagnostics

#ifdef _DEBUG
void CMainWindow::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CMainWindow::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


void CMainWindow::SetEverettEngineRef(EverettEngine& engineRef)
{
	engineP = &engineRef;
}

void CMainWindow::OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
}

void CMainWindow::OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto selectedNodes = objectTree.GetAllOfRootsSelectedNode();

	if (selectedNodes.size() == 2)
	{
		CObjectEditDialog objEditDlg(
			*engineP, 
			EverettEngine::GetObjectTypeToName(selectedNodes[0].first), 
			selectedScriptDllInfo, 
			selectedNodes
		);
		objEditDlg.DoModal();
	}

	*pResult = 0;
}

std::vector<std::pair<std::string, std::string>>& CMainWindow::GetSelectedScriptDllInfo()
{
	return selectedScriptDllInfo;
}