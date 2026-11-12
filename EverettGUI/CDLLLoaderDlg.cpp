#include "pch.h"

#include "resource.h"

#include "CDLLLoaderDlg.h"
#include "CBrowseDialog.h"

#include "EverettException.h"

IMPLEMENT_DYNAMIC(CDLLLoaderDlg, CTrackedDlg)

BEGIN_MESSAGE_MAP(CDLLLoaderDlg, CTrackedDlg)
	ON_BN_CLICKED(IDC_BUTTON2, &CDLLLoaderDlg::OnBrowseScriptButton)
	ON_BN_CLICKED(IDC_BUTTON3, &CDLLLoaderDlg::OnLoadScriptButtonClick)
	ON_BN_CLICKED(IDC_BUTTON4, &CDLLLoaderDlg::OnUnloadDllButtonClick)
	ON_CBN_SELENDOK(IDC_COMBO1, &CDLLLoaderDlg::OnScriptSelectionChangeOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CDLLLoaderDlg::OnPreloadDLLButtonClick)
END_MESSAGE_MAP()

CDLLLoaderDlg::CDLLLoaderDlg(
	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
	EverettEngine& engineRef,
	std::function<void()> scriptChangeCallback,
	CWnd* pParent
) :
	CTrackedDlg(IDD_DIALOG5, pParent),
	selectedScriptDllInfo(selectedScriptDllInfo), 
	scriptChangeCallback(std::move(scriptChangeCallback)),
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
		auto dllLoadState = engineRef.CheckDLLState(selectedScriptDllInfo[dllComboBox.GetCurSel()].first);

		preloadScriptButton.EnableWindow(dllLoadState == EverettStructs::DLLLoadState::Unloaded);
		loadScriptButton.EnableWindow(dllLoadState != EverettStructs::DLLLoadState::Loaded);
		unloadScriptButton.EnableWindow(engineRef.CheckIfScriptsRunning(false));

		if (scriptChangeCallback)
		{
			scriptChangeCallback();
		}
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

	resetStateCheck.SetCheck(true);
	applyToAllCheck.SetCheck(true);

	return true;
}

void CDLLLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON2, scriptBrowseButton);
	DDX_Control(pDX, IDC_BUTTON1, preloadScriptButton);
	DDX_Control(pDX, IDC_BUTTON3, loadScriptButton);
	DDX_Control(pDX, IDC_BUTTON4, unloadScriptButton);
	DDX_Control(pDX, IDC_COMBO1, dllComboBox);
	DDX_Control(pDX, IDC_CHECK2, resetStateCheck);
	DDX_Control(pDX, IDC_CHECK1, applyToAllCheck);
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

void CDLLLoaderDlg::OnPreloadDLLButtonClick()
{
	int curSel = dllComboBox.GetCurSel();

	engineRef.SetupScriptDLL(selectedScriptDllInfo[curSel].first, true);

	UpdateScriptButtons();
}

void CDLLLoaderDlg::ExecuteScriptsAccordingToRule(std::function<bool(int)> func)
{
	if (!applyToAllCheck.GetCheck())
	{
		func(dllComboBox.GetCurSel());
	}
	else
	{
		for (int i = 0; i < selectedScriptDllInfo.size(); ++i)
		{
			if(!func(i)) break;
		}
	}

	UpdateScriptButtons();
}

void CDLLLoaderDlg::OnLoadScriptButtonClick()
{
	ExecuteScriptsAccordingToRule([this](int index) {
		// One of the few good use cases for exceptions in C++ (specifically C++, since throwing is so expensive)
		// Here if we panic on "s. object get" during ScriptInit call, we can protect
		// execution from calling on a null ptr to s. object instantly. 
		// Essentially, creating safe transition from soon to be crash causing code to regular engine execution smoothly 
		try
		{
			engineRef.SetupScriptDLL(selectedScriptDllInfo[index].first);
			return true;
		}
		catch (const EverettException&)
		{
			engineRef.UnsetScripts(resetStateCheck.GetCheck());
			return false;
		}
	});
}

void CDLLLoaderDlg::OnUnloadDllButtonClick()
{
	engineRef.UnsetScripts(resetStateCheck.GetCheck());
	UpdateScriptButtons();
}

void CDLLLoaderDlg::OnScriptSelectionChangeOk()
{
	UpdateScriptButtons();
}