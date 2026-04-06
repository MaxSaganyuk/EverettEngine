#include "pch.h"

#include "EverettGUI.h"

#include "CDLLLoaderDlg.h"
#include "CBrowseDialog.h"

IMPLEMENT_DYNAMIC(CDLLLoaderDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CDLLLoaderDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CDLLLoaderDlg::OnBrowseScriptButton)
	ON_BN_CLICKED(IDC_BUTTON3, &CDLLLoaderDlg::OnLoadScriptButtonClick)
	ON_BN_CLICKED(IDC_BUTTON4, &CDLLLoaderDlg::OnUnloadDllButtonClick)
	ON_CBN_SELENDOK(IDC_COMBO1, &CDLLLoaderDlg::OnScriptSelectionChangeOk)
END_MESSAGE_MAP()

CDLLLoaderDlg::CDLLLoaderDlg(
	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
	EverettEngine& engineRef,
	CWnd* pParent
) :
	CDialogEx(IDD_DIALOG5, pParent),
	selectedScriptDllInfo(selectedScriptDllInfo), 
	engineRef(engineRef) {}

CDLLLoaderDlg::~CDLLLoaderDlg() {}

void CDLLLoaderDlg::FillComboBoxWithScriptInfo()
{
	for (auto& scriptDllInfo : selectedScriptDllInfo)
	{
		dllComboBox.AddString(scriptDllInfo.second);
	}
}

void CDLLLoaderDlg::UpdateScriptButtons()
{
	if (!selectedScriptDllInfo.empty())
	{
		bool isDllLoadedSet = engineRef.IsDLLLoaded(selectedScriptDllInfo[dllComboBox.GetCurSel()].first);

		loadScriptButton.EnableWindow(!isDllLoadedSet);
		loadScriptButton.SetWindowTextW(CString(isDllLoadedSet ? UnloadDLL : LoadDLL));

		unloadScriptButton.EnableWindow(isDllLoadedSet);
	}
}

BOOL CDLLLoaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (selectedScriptDllInfo.size() > 0)
	{
		FillComboBoxWithScriptInfo();
		dllComboBox.SetCurSel(0);
		UpdateScriptButtons();
	}

	return true;
}

void CDLLLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
	DDX_Control(pDX, IDC_BUTTON4, unloadScriptButton);
	DDX_Control(pDX, IDC_COMBO1, dllComboBox);
}

void CDLLLoaderDlg::OnBrowseScriptButton()
{
	AdString pathStr;
	AdString fileStr;

	if (CBrowseDialog::OpenAndGetFilePath(pathStr, fileStr, {{"DLL files", "*.dll"}}))
	{
		selectedScriptDllInfo.push_back(std::pair<std::string, std::string>{ pathStr, fileStr });
		dllComboBox.AddString(fileStr);
		dllComboBox.SetCurSel(static_cast<int>(selectedScriptDllInfo.size() - 1));
		UpdateScriptButtons();
	}
}

void CDLLLoaderDlg::OnLoadScriptButtonClick()
{
	int curSel = dllComboBox.GetCurSel();

	engineRef.SetupScriptDLL(selectedScriptDllInfo[curSel].first);

	UpdateScriptButtons();
}

void CDLLLoaderDlg::OnUnloadDllButtonClick()
{
	engineRef.UnsetScript(selectedScriptDllInfo[dllComboBox.GetCurSel()].first);

	UpdateScriptButtons();
}

void CDLLLoaderDlg::OnScriptSelectionChangeOk()
{
	UpdateScriptButtons();
}