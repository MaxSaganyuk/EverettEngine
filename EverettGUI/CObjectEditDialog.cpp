// CObjectEditDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CObjectEditDialog.h"
#include "CBrowseDialog.h"


// CObjectEditDialog dialog

IMPLEMENT_DYNAMIC(CObjectEditDialog, CDialogEx)

CObjectEditDialog::CObjectEditDialog(
	EverettEngine& engine, 
	EverettEngine::ObjectTypes objectType,
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
	const std::vector<std::pair<std::string, std::string>>& selectedNodes,
	CWnd* pParent 
)
	: 
	CDialogEx(IDD_DIALOG4, pParent), 
	engineRef(engine), 
	objectType(objectType),
	selectedScriptDllInfo(selectedScriptDllInfo)
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
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, objectInfoEdits[0][0]);
	DDX_Control(pDX, IDC_EDIT2, objectInfoEdits[0][1]);
	DDX_Control(pDX, IDC_EDIT3, objectInfoEdits[0][2]);
	DDX_Control(pDX, IDC_EDIT4, objectInfoEdits[1][0]);
	DDX_Control(pDX, IDC_EDIT5, objectInfoEdits[1][1]);
	DDX_Control(pDX, IDC_EDIT6, objectInfoEdits[1][2]);
	DDX_Control(pDX, IDC_EDIT7, objectInfoEdits[2][0]);
	DDX_Control(pDX, IDC_EDIT8, objectInfoEdits[2][1]);
	DDX_Control(pDX, IDC_EDIT9, objectInfoEdits[2][2]);
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
	DDX_Control(pDX, IDC_BUTTON4, unloadScriptButton);
	DDX_Control(pDX, IDC_COMBO1, dllComboBox);
	DDX_Control(pDX, IDC_CHECK2, scriptRunIndicator);
}


BOOL CObjectEditDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetObjectParams(engineRef.GetObjectParamsByName(objectType, subtypeName, objectName));
	
	if (selectedScriptDllInfo.size() > 0)
	{
		FillComboBoxWithScriptInfo();
		dllComboBox.SetCurSel(0);
		loadScriptButton.EnableWindow(true);
		scriptRunIndicator.SetCheck(
			engineRef.IsObjectScriptSet(objectType, subtypeName, objectName, selectedScriptDllInfo[0].second)
		);
	}
	return true;
}

void CObjectEditDialog::FillComboBoxWithScriptInfo()
{
	for (auto& scriptDllInfo : selectedScriptDllInfo)
	{
		dllComboBox.AddString(CA2T(scriptDllInfo.second.c_str()));
	}
}

BEGIN_MESSAGE_MAP(CObjectEditDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CObjectEditDialog::OnUpdateParamsButtonClick)
	ON_BN_CLICKED(IDC_BUTTON2, &CObjectEditDialog::OnBrowseScriptButton)
	ON_BN_CLICKED(IDC_BUTTON3, &CObjectEditDialog::OnLoadScriptButtonClick)
	ON_BN_CLICKED(IDC_BUTTON4, &CObjectEditDialog::OnUnloadDllButtonClick)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CObjectEditDialog::OnScriptSelectionChange)
	ON_CBN_SELENDOK(IDC_COMBO1, &CObjectEditDialog::OnScriptSelectionChangeOk)
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


void CObjectEditDialog::OnBrowseScriptButton()
{
	CString pathStr;
	CString fileStr;

	if (CBrowseDialog::OpenAndGetFilePath(pathStr, fileStr))
	{
		selectedScriptDllInfo.push_back(std::pair<std::string, std::string>{ CT2A(pathStr), CT2A(fileStr) });
		dllComboBox.AddString(fileStr);
		dllComboBox.SetCurSel(selectedScriptDllInfo.size() - 1);
		loadScriptButton.EnableWindow(true);
	}
}

void CObjectEditDialog::UpdateScriptButtons()
{
	bool isObjectScriptSet = engineRef.IsObjectScriptSet(
		objectType, 
		subtypeName, 
		objectName, 
		selectedScriptDllInfo[dllComboBox.GetCurSel()].second
	);

	scriptRunIndicator.SetCheck(isObjectScriptSet);
	loadScriptButton.EnableWindow(!isObjectScriptSet);
	unloadScriptButton.EnableWindow(isObjectScriptSet);
}


void CObjectEditDialog::OnLoadScriptButtonClick()
{
	int curSel = dllComboBox.GetCurSel();

	engineRef.SetScriptToObject(
		objectType, 
		subtypeName,
		objectName,
		selectedScriptDllInfo[curSel].first,
		selectedScriptDllInfo[curSel].second
	);

	UpdateScriptButtons();
}


void CObjectEditDialog::OnUnloadDllButtonClick()
{
	engineRef.UnsetScriptFromObject(selectedScriptDllInfo[dllComboBox.GetCurSel()].second);

	UpdateScriptButtons();
}


void CObjectEditDialog::OnScriptSelectionChange()
{
}


void CObjectEditDialog::OnScriptSelectionChangeOk()
{
	UpdateScriptButtons();
}
