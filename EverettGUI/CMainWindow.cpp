// CMainWindow.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "CMainWindow.h"
#include "CBrowseDialog.h"
#include "CObjectEditDialog.h"

#include "EverettException.h"

using ObjectTypes = EverettEngine::ObjectTypes;

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

MFCTreeManager& CMainWindow::GetObjectTree()
{
	return objectTree;
}

void CMainWindow::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	objectTree.SetObjectTypes();
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
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CMainWindow::OnNodeRightClick)
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

HTREEITEM CMainWindow::ForceNodeSelection()
{
	CPoint point;
	GetCursorPos(&point);

	CTreeCtrl& treeCtrl = objectTree.GetTreeCtrl();
	treeCtrl.ScreenToClient(&point);

	unsigned int flags;
	HTREEITEM hItem = nullptr;
	hItem = treeCtrl.HitTest(point, &flags);

	if (hItem)
	{
		treeCtrl.SelectItem(hItem);
	}

	return hItem;
}

void CMainWindow::OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto selectedNodes = objectTree.GetAllOfRootsSelectedNode();
	ObjectTypes currentType;

	try
	{
		currentType = EverettEngine::GetObjectTypeToName(selectedNodes.back().second);
	}
	catch (const EverettException&)
	{
		// Invalid value here is unexpected, but not fatal - log, ignore and do not crash
		return;
	}
	selectedNodes.pop_back();

	if (selectedNodes.size() == validSubnodeAmount[currentType])
	{
		CObjectEditDialog objEditDlg(
			*engineP,
			currentType,
			selectedScriptDllInfo,
			selectedNodes
		);
		objEditDlg.DoModal();
	}

	*pResult = 0;
}

void CMainWindow::OnNodeRightClick(NMHDR* pNMHDR, LRESULT* pResult)
{ 
	HTREEITEM selectedNodeRaw = ForceNodeSelection();
	if (!selectedNodeRaw)
	{
		return;
	}

	auto selectedNodes = objectTree.GetAllOfRootsSelectedNode();

	ObjectTypes currentType;

	try
	{
		currentType = EverettEngine::GetObjectTypeToName(selectedNodes.back().second);
	}
	catch (const EverettException&)
	{
		// Invalid value here is unexpected, but not fatal - log, ignore and do not crash
		return;
	}
	selectedNodes.pop_back();

	std::function<bool(const std::string&)> deleterFunc = nullptr;

	if (selectedNodes.size() == validSubnodeAmount[currentType])
	{
		switch (currentType)
		{
		case ObjectTypes::Solid:
			deleterFunc = [this](const std::string& solidName) { return engineP->DeleteSolid(solidName); };
			break;
		case ObjectTypes::Light:
			deleterFunc = [this](const std::string& lightName) { return engineP->DeleteLight(lightName); };
			break;
		case ObjectTypes::Sound:
			deleterFunc = [this](const std::string& soundName) { return engineP->DeleteSound(soundName); };
			break;
		default:
			assert(false && "unreachable");
		}
	}
	else if (currentType == ObjectTypes::Solid && selectedNodes.size() == validSubnodeAmount[currentType] - 1)
	{
		deleterFunc = [this](const std::string& modelName) { return engineP->DeleteModel(modelName); };
	}
	
	if (deleterFunc)
	{
		int res = AfxMessageBox(
			L"Are you sure you want to delete " + selectedNodes.front().first + L" : " + selectedNodes.front().second + L'?',
			MB_YESNO | MB_ICONQUESTION
		);

		if (res == IDYES && deleterFunc(selectedNodes.front().second))
		{
			objectTree.DeleteNodeByItem(selectedNodeRaw, true);
		}
	}
}

std::vector<std::pair<AdString, AdString>>& CMainWindow::GetSelectedScriptDllInfo()
{
	return selectedScriptDllInfo;
}

bool CMainWindow::SetSelectedScriptDLLInfo(const std::vector<std::pair<std::string, std::string>>& scriptDLLInfo)
{
	for (auto& scriptDLLPair : scriptDLLInfo)
	{
		selectedScriptDllInfo.push_back({ scriptDLLPair.first, scriptDLLPair.second });
	}

	return true;
}