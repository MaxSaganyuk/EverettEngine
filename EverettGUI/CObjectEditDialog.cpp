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
	std::vector<std::pair<std::string, std::string>>& selectedNodes,
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
	CWnd* pParent 
)
	: 
	CDialogEx(IDD_DIALOG4, pParent), 
	engineRef(engine), 
	selectedNodes(selectedNodes), 
	selectedScriptDllInfo(selectedScriptDllInfo)
{}

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

	SetObjectParams(engineRef.GetSolidParamsByName(selectedNodes[0].second, selectedNodes[1].second));
	
	if (selectedScriptDllInfo.size() > 0)
	{
		FillComboBoxWithScriptInfo();
		loadScriptButton.EnableWindow(true);
		scriptRunIndicator.SetCheck(engineRef.IsObjectScriptSet(selectedNodes[1].second));
	}
	return true;
}

void CObjectEditDialog::FillComboBoxWithScriptInfo()
{
	for (auto& scriptDllInfo : selectedScriptDllInfo)
	{
		dllComboBox.AddString(CA2T(scriptDllInfo.first.c_str()));
	}
}

BEGIN_MESSAGE_MAP(CObjectEditDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CObjectEditDialog::OnUpdateParamsButtonClick)
	ON_BN_CLICKED(IDC_BUTTON2, &CObjectEditDialog::OnBrowseScriptButton)
	ON_BN_CLICKED(IDC_BUTTON3, &CObjectEditDialog::OnLoadScriptButtonClick)
	ON_BN_CLICKED(IDC_BUTTON4, &CObjectEditDialog::OnUnloadDllButtonClick)
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

	if (selectedNodes[0].first == "Solid")
	{
		engineRef.SetSolidParamsByName(selectedNodes[0].second, selectedNodes[1].second, valuesToSet);
	}
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
	bool isObjectScriptSet = engineRef.IsObjectScriptSet(selectedNodes[1].second);

	scriptRunIndicator.SetCheck(isObjectScriptSet);
	loadScriptButton.EnableWindow(!isObjectScriptSet);
	unloadScriptButton.EnableWindow(isObjectScriptSet);
}


void CObjectEditDialog::OnLoadScriptButtonClick()
{
	engineRef.SetScriptToObject(selectedNodes[1].second, selectedScriptDllInfo[dllComboBox.GetCurSel()].second);

	UpdateScriptButtons();
}


void CObjectEditDialog::OnUnloadDllButtonClick()
{
	engineRef.UnsetScriptFromObject(selectedScriptDllInfo[dllComboBox.GetCurSel()].second);

	UpdateScriptButtons();
}
