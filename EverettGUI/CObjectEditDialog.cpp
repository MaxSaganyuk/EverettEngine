// CObjectEditDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CObjectEditDialog.h"
#include "CBrowseDialog.h"


// CObjectEditDialog dialog

IMPLEMENT_DYNAMIC(CObjectEditDialog, DLLLoaderCommon)

CObjectEditDialog::CObjectEditDialog(
	EverettEngine& engine, 
	EverettEngine::ObjectTypes objectTypeP,
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
	const std::vector<std::pair<std::string, std::string>>& selectedNodes,
	CWnd* pParent 
)
	: 
	DLLLoaderCommon(
		IDD_DIALOG4, 
		selectedScriptDllInfo, 
		[this](const std::string& dllName) { 
			return engineRef.IsObjectScriptSet(objectType, subtypeName, objectName, dllName); 
		},
		[this](const std::string& dllName, const std::string& dllPath) { 
			engineRef.SetScriptToObject(objectType, subtypeName, objectName, dllName, dllPath); 
		},
		[this](const std::string& dllPath) { engineRef.UnsetScriptFromObject(dllPath); },
		pParent
	),
	engineRef(engine), 
	objectType(objectTypeP)
{
	subtypeName = "";
	objectName = "";

	if (selectedNodes.size() == 2)
	{
		subtypeName = selectedNodes[1].second;
		objectName = selectedNodes[0].second;
	}
	else if (selectedNodes.size() == 1)
	{
		objectName = selectedNodes[0].second;
	}
}

CObjectEditDialog::~CObjectEditDialog()
{
}

void CObjectEditDialog::DoDataExchange(CDataExchange* pDX)
{
	DLLLoaderCommon::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, objectInfoEdits[0][0]);
	DDX_Control(pDX, IDC_EDIT2, objectInfoEdits[0][1]);
	DDX_Control(pDX, IDC_EDIT3, objectInfoEdits[0][2]);
	DDX_Control(pDX, IDC_EDIT4, objectInfoEdits[1][0]);
	DDX_Control(pDX, IDC_EDIT5, objectInfoEdits[1][1]);
	DDX_Control(pDX, IDC_EDIT6, objectInfoEdits[1][2]);
	DDX_Control(pDX, IDC_EDIT7, objectInfoEdits[2][0]);
	DDX_Control(pDX, IDC_EDIT8, objectInfoEdits[2][1]);
	DDX_Control(pDX, IDC_EDIT9, objectInfoEdits[2][2]);

}


BOOL CObjectEditDialog::OnInitDialog()
{
	DLLLoaderCommon::OnInitDialog();

	SetObjectParams(engineRef.GetObjectParamsByName(objectType, subtypeName, objectName));

	return true;
}

BEGIN_MESSAGE_MAP(CObjectEditDialog, DLLLoaderCommon)
	ON_BN_CLICKED(IDC_BUTTON1, &CObjectEditDialog::OnUpdateParamsButtonClick)
END_MESSAGE_MAP()


void CObjectEditDialog::SetObjectParams(const std::vector<glm::vec3>& params)
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


void CObjectEditDialog::OnUpdateParamsButtonClick()
{
	std::vector<glm::vec3> valuesToSet;

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

	engineRef.SetObjectParamsByName(
		objectType, 
		subtypeName, 
		objectName, 
		valuesToSet
	);
}
