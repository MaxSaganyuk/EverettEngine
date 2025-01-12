// CMainWindow.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "CMainWindow.h"
#include "CBrowseDialog.h"


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

	
	ClearObjectParams();

	objectTree.SetObjectTypes(engineP->GetObjectTypes(), engineP->GetLightTypeList());
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
	DDX_Control(pDX, IDC_EDIT1, objectInfoEdits[0][0]);
	DDX_Control(pDX, IDC_EDIT2, objectInfoEdits[0][1]);
	DDX_Control(pDX, IDC_EDIT3, objectInfoEdits[0][2]);
	DDX_Control(pDX, IDC_EDIT4, objectInfoEdits[1][0]);
	DDX_Control(pDX, IDC_EDIT5, objectInfoEdits[1][1]);
	DDX_Control(pDX, IDC_EDIT6, objectInfoEdits[1][2]);
	DDX_Control(pDX, IDC_EDIT7, objectInfoEdits[2][0]);
	DDX_Control(pDX, IDC_EDIT8, objectInfoEdits[2][1]);
	DDX_Control(pDX, IDC_EDIT9, objectInfoEdits[2][2]);
	DDX_Control(pDX, IDC_TREE1, objectTree.GetTreeCtrl());
	DDX_Control(pDX, IDC_EDIT10, scriptEdit);
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
}

BEGIN_MESSAGE_MAP(CMainWindow, CFormView)
	ON_BN_CLICKED(IDC_BUTTON1, &CMainWindow::OnUpdateParamsClick)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMainWindow::OnTreeSelectionChanged)
	ON_BN_CLICKED(IDC_BUTTON2, &CMainWindow::OnBrowseButtonClick)
	ON_BN_CLICKED(IDC_BUTTON3, &CMainWindow::OnLoadScriptButton)
	ON_BN_CLICKED(IDC_BUTTON4, &CMainWindow::OnUnloadScriptButton)
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


// CMainWindow message handlers
void CMainWindow::UpdateModelChoice()
{
}

void CMainWindow::OnUpdateParamsClick()
{
	if (engineP)
	{
		std::vector<glm::vec3> valuesToSet;
		auto nodeStrings = objectTree.GetAllOfRootsSelectedNode();

		for (int i = 0; i < objectInfoEdits.size(); ++i)
		{
			valuesToSet.push_back({});
			for (int j = 0; j < objectInfoEdits.front().size(); ++j)
			{
				CString value;
				objectInfoEdits[i][j].GetWindowTextW(value);
				valuesToSet[i][j] = _tstof(value);
			}
		}

		engineP->SetSolidParamsByName(nodeStrings["Model"], nodeStrings["Solid"], valuesToSet);
	}
}

void CMainWindow::SetEverettEngineRef(EverettEngine& engineRef)
{
	engineP = &engineRef;
}

void CMainWindow::ClearObjectParams()
{
	for (auto& objectInfoEdit : objectInfoEdits)
	{
		for (auto& objectInfo : objectInfoEdit)
		{
			objectInfo.SetWindowTextW(_T(""));
			objectInfo.EnableWindow(false);
			objectInfo.RedrawWindow();
		}
	}
	UpdateData(false);
}

void CMainWindow::SetObjectParams(const std::vector<glm::vec3>& params)
{
	for (int i = 0; i < objectInfoEdits.size(); ++i)
	{
		for (int j = 0; j < objectInfoEdits.front().size(); ++j)
		{
			CString value;
			value.Format(_T("%.4f"), params[i][j]);
			objectInfoEdits[i][j].SetWindowTextW(value);
			objectInfoEdits[i][j].EnableWindow(true);
			objectInfoEdits[i][j].RedrawWindow();
		}
	}
}

void CMainWindow::OnTreeSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto nodeStrings = objectTree.GetAllOfRootsSelectedNode();

	if (nodeStrings.find("Solid") != nodeStrings.end())
	{
		if (nodeStrings.find("Model") != nodeStrings.end())
		{
			chosenObjectName = nodeStrings["Solid"];
			SetObjectParams(engineP->GetSolidParamsByName(nodeStrings["Model"], nodeStrings["Solid"]));
		}
	}
	else
	{
		ClearObjectParams();
	}

	*pResult = 0;
}


void CMainWindow::OnBrowseButtonClick()
{
	CString pathStr;

	if (CBrowseDialog::OpenAndGetFilePath(pathStr))
	{
		scriptEdit.SetWindowTextW(pathStr);
		dllScriptPath = CT2A(pathStr);
	}
}


void CMainWindow::OnLoadScriptButton()
{
	if (engineP)
	{
		engineP->SetScriptToObject(chosenObjectName, dllScriptPath);
	}
}


void CMainWindow::OnUnloadScriptButton()
{

}
