#include "pch.h"

#include "EverettGUI.h"

#include "DLLLoaderCommon.h"
#include "CBrowseDialog.h"

IMPLEMENT_DYNAMIC(DLLLoaderCommon, CDialogEx)

BEGIN_MESSAGE_MAP(DLLLoaderCommon, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &DLLLoaderCommon::OnBrowseScriptButton)
	ON_BN_CLICKED(IDC_BUTTON3, &DLLLoaderCommon::OnLoadScriptButtonClick)
	ON_BN_CLICKED(IDC_BUTTON4, &DLLLoaderCommon::OnUnloadDllButtonClick)
	ON_CBN_SELENDOK(IDC_COMBO1, &DLLLoaderCommon::OnScriptSelectionChangeOk)
END_MESSAGE_MAP()

DLLLoaderCommon::DLLLoaderCommon(
	int dialogID,
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
	IsScriptSetFunc isScriptSetFunc,
	SetScriptFunc setScriptFunc,
	UnsetScriptFunc unsetScriptFunc,
	CWnd* pParent
) :
	CDialogEx(dialogID, pParent), 
	selectedScriptDllInfo(selectedScriptDllInfo), 
	isScriptSetFunc(isScriptSetFunc), 
	setScriptFunc(setScriptFunc), 
	unsetScriptFunc(unsetScriptFunc)
{

}

DLLLoaderCommon::~DLLLoaderCommon()
{
}

void DLLLoaderCommon::FillComboBoxWithScriptInfo()
{
	for (auto& scriptDllInfo : selectedScriptDllInfo)
	{
		dllComboBox.AddString(CA2T(scriptDllInfo.second.c_str()));
	}
}

void DLLLoaderCommon::UpdateScriptButtons()
{
	bool isObjectScriptSet = isScriptSetFunc(selectedScriptDllInfo[dllComboBox.GetCurSel()].second);

	scriptRunIndicator.SetCheck(isObjectScriptSet);
	loadScriptButton.EnableWindow(!isObjectScriptSet);
	unloadScriptButton.EnableWindow(isObjectScriptSet);
}

BOOL DLLLoaderCommon::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (selectedScriptDllInfo.size() > 0)
	{
		FillComboBoxWithScriptInfo();
		dllComboBox.SetCurSel(0);
		loadScriptButton.EnableWindow(true);
		scriptRunIndicator.SetCheck(isScriptSetFunc(selectedScriptDllInfo[0].second));
	}

	return true;
}

void DLLLoaderCommon::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
	DDX_Control(pDX, IDC_BUTTON4, unloadScriptButton);
	DDX_Control(pDX, IDC_COMBO1, dllComboBox);
	DDX_Control(pDX, IDC_CHECK2, scriptRunIndicator);
}

void DLLLoaderCommon::OnBrowseScriptButton()
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

void DLLLoaderCommon::OnLoadScriptButtonClick()
{
	int curSel = dllComboBox.GetCurSel();

	setScriptFunc(selectedScriptDllInfo[curSel].first, selectedScriptDllInfo[curSel].second);

	UpdateScriptButtons();
}

void DLLLoaderCommon::OnUnloadDllButtonClick()
{
	unsetScriptFunc(selectedScriptDllInfo[dllComboBox.GetCurSel()].first);

	UpdateScriptButtons();
}

void DLLLoaderCommon::OnScriptSelectionChangeOk()
{
	UpdateScriptButtons();
}