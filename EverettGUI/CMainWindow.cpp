// CMainWindow.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "CMainWindow.h"
#include "CObjectEditDialog.h"
#include "CCopyObjDlg.h"
#include "CRenameObjDlg.h"
#include "MFCUtilities.h"

#include "EverettException.h"
#include "CommonStrEdits.h"

#include <iostream>

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

	showGizmoCheck.SetCheck(true);
	showDebugTextCheck.SetCheck(true);
	defaultControlsCheck.SetCheck(true);
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
	DDX_Control(pDX, IDC_CHECK1, showGizmoCheck);
	DDX_Control(pDX, IDC_CHECK2, showDebugTextCheck);
	DDX_Control(pDX, IDC_CHECK3, defaultControlsCheck);
}

BEGIN_MESSAGE_MAP(CMainWindow, CFormView)
	ON_COMMAND(CopyNode, &CMainWindow::OnCopyNode)
	ON_COMMAND(RenameNode, &CMainWindow::OnRenameNode)
	ON_COMMAND(DeleteNode, &CMainWindow::OnDeleteNode)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMainWindow::OnTreeSelectionChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CMainWindow::OnNodeDoubleClick)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CMainWindow::OnNodeRightClick)
	ON_BN_CLICKED(IDC_BUTTON1, &CMainWindow::OnEditAmbientLightButton)
	ON_BN_CLICKED(IDC_CHECK1, &CMainWindow::OnShowGizmoCheckClick)
	ON_BN_CLICKED(IDC_CHECK2, &CMainWindow::OnShowDebugTextCheckClick)
	ON_BN_CLICKED(IDC_BUTTON2, &CMainWindow::OnEditBackgroundColorButtonClick)
	ON_BN_CLICKED(IDC_CHECK3, &CMainWindow::OnDefaultControlsCheckClick)
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

	unsigned int flags{};
	HTREEITEM hItem = nullptr;
	hItem = treeCtrl.HitTest(point, &flags);

	if (hItem)
	{
		treeCtrl.SelectItem(hItem);
	}

	return hItem;
}

// C string is used instead of std::string to reduce amount of issues possible on panic state
void CMainWindow::Panic(const char* errorMessage)
{
	engineP->CreateLogReport();
	std::cerr << errorMessage << '\n';
	throw std::runtime_error{ errorMessage };
}

void CMainWindow::OnNodeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto selectedNodes = objectTree.GetAllOfRootsSelectedNode();

	if (!selectedNodes.empty())
	{
		std::optional<ObjectTypes> currentType;

		currentType = EverettEngine::GetObjectTypeToName(selectedNodes.back().second);

		if (!currentType.has_value()) return;

		selectedNodes.pop_back();

		if (selectedNodes.size() == validSubnodeAmount[currentType.value()])
		{
			CObjectEditDialog objEditDlg(
				*engineP,
				currentType.value(),
				selectedScriptDllInfo,
				selectedNodes
			);
			objEditDlg.DoModal();
		}
	}

	*pResult = 0;
}

void CMainWindow::OnNodeRightClick(NMHDR* pNMHDR, LRESULT* pResult)
{ 
	currentNodeInfo.Clean();
	currentNodeInfo.rawNode = ForceNodeSelection();
	
	if (!currentNodeInfo.rawNode)
	{
		currentNodeInfo.Clean();
		return;
	}

	currentNodeInfo.selectedSubnodes = objectTree.GetAllOfRootsSelectedNode();

	AdString& objectType = currentNodeInfo.selectedSubnodes.front().first;

	if (objectType != "Model")
	{
		currentNodeInfo.selectedType = EverettEngine::GetObjectTypeToName(objectType);

		if (!currentNodeInfo.selectedType.has_value())
		{
			currentNodeInfo.Clean();
			return;
		}

	}
	currentNodeInfo.selectedSubnodes.pop_back();

	CMenu menu;
	CPoint point;

	if (GetCursorPos(&point))
	{
		menu.CreatePopupMenu();

		if (currentNodeInfo.selectedType)
		{
			menu.AppendMenuW(MF_STRING, CopyNode, _T("Copy object"));
		}

		menu.AppendMenuW(MF_STRING, RenameNode, _T("Rename object"));
		menu.AppendMenuW(MF_STRING, DeleteNode, _T("Delete object"));

		if (engineP->CheckIfScriptsRunning(false))
		{
			constexpr UINT disableBitVal = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;

			menu.EnableMenuItem(RenameNode, disableBitVal);
			menu.EnableMenuItem(DeleteNode, disableBitVal);
		}

		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void CMainWindow::OnCopyNode()
{
	auto& [_, name] = currentNodeInfo.selectedSubnodes.front();

	CCopyObjDlg copyObjDlg;

	if (copyObjDlg.DoModal() == IDOK)
	{
		size_t pickedAmount = copyObjDlg.GetPickedAmount();

		if (!pickedAmount) return;

		size_t firstCount = engineP->GetObjectNameCount(name);

		if (engineP->CopyObject(name, pickedAmount, currentNodeInfo.selectedType))
		{
			AdString newNameBase = CommonStrEdits::RemoveDigitsFromStringEnd(name);

			for (size_t i = firstCount; i < firstCount + pickedAmount; ++i)
			{
				AdString newName = newNameBase + std::to_wstring(i).c_str();
				
				switch (*currentNodeInfo.selectedType)
				{
				case EverettEngine::ObjectTypes::Solid:
					objectTree.AddSolidToTree(currentNodeInfo.selectedSubnodes[1].second, newName);
					break;
				case EverettEngine::ObjectTypes::Light:
					objectTree.AddLightToTree(currentNodeInfo.selectedSubnodes[1].second, newName);
					break;
				case EverettEngine::ObjectTypes::Sound:
					objectTree.AddSoundToTree(newName);
					break;
				case EverettEngine::ObjectTypes::Collider:
					objectTree.AddColliderToTree(newName);
					break;
				}
			}
		}
		else
		{
			Panic("[CRITICAL ERROR] Copy step failed. Terminating\n");
		}
	}
}

void CMainWindow::OnRenameNode()
{
	AdString& oldName = currentNodeInfo.selectedSubnodes.front().second;

	CRenameObjDlg renameObjDlg(oldName);

	if (renameObjDlg.DoModal() == IDOK)
	{
		AdString newName = renameObjDlg.GetNewName();

		if (!(engineP->RenameObject(oldName, newName, currentNodeInfo.selectedType) &&
			  objectTree.RenameNodeByItem(currentNodeInfo.rawNode, newName))
		)
		{
			Panic("[CRITICAL ERROR] Rename step failed. Terminating\n");
		}
	}
}

void CMainWindow::OnDeleteNode()
{
	auto& [typeName, objectName] = currentNodeInfo.selectedSubnodes.front();

	int res = AfxMessageBox(
		L"Are you sure you want to delete " + typeName + L" : " + objectName + L'?',
		MB_YESNO | MB_ICONQUESTION
	);

	if (res == IDYES)
	{
		if (!(engineP->DeleteObject(objectName, currentNodeInfo.selectedType) && 
			objectTree.DeleteNodeByItem(currentNodeInfo.rawNode, true))
		)
		{
			Panic("[CRITICAL ERROR] Delete step failed. Terminating\n");
		}
	}
}

std::vector<std::pair<AdString, AdString>>& CMainWindow::GetSelectedScriptDllInfo()
{
	return selectedScriptDllInfo;
}

bool CMainWindow::SetSelectedScriptDLLInfo(std::generator<EverettStructs::BasicFileInfo>&& scriptDLLInfo)
{
	for (auto scriptDLLPair : scriptDLLInfo)
	{
		selectedScriptDllInfo.push_back({ scriptDLLPair.path, scriptDLLPair.name });
	}

	return true;
}

void CMainWindow::OnEditAmbientLightButton()
{
	MFCUtilities::OpenColorSelection([this]() -> glm::vec3& { return engineP->GetAmbientLightVectorAddr(); });
}

void CMainWindow::OnEditBackgroundColorButtonClick()
{
	MFCUtilities::OpenColorSelection([this]() -> glm::vec3& { return engineP->GetBackgroundColorVectorAddr(); });
}

void CMainWindow::OnShowGizmoCheckClick()
{
	engineP->SetGizmoVisible(showGizmoCheck.GetCheck());
}

void CMainWindow::OnShowDebugTextCheckClick()
{
	engineP->SetDebugLogVisible(showDebugTextCheck.GetCheck());
}

void CMainWindow::OnDefaultControlsCheckClick()
{
	engineP->SetDefaultControls(defaultControlsCheck.GetCheck());
}
