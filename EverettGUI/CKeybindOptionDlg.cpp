// CKeybindOptionDlg.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CKeybindOptionDlg.h"

#include <thread>

// CKeybindOptionDlg dialog

IMPLEMENT_DYNAMIC(CKeybindOptionDlg, DLLLoaderCommon)

CKeybindOptionDlg::CKeybindOptionDlg(
	EverettEngine& engine,
	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
	CWnd* pParent
)
	: 
	DLLLoaderCommon(
		IDD_DIALOG5,
		selectedScriptDllInfo,
		[this](const std::string& dllPath) { return engineRef.IsDLLLoaded(dllPath); },
		[this](const std::string& dllName) { return !keyName.empty() && engineRef.IsKeyScriptSet(keyName, dllName); },
		[this](const std::string& dllName, const std::string& dllPath) { 
			engineRef.SetScriptToKey(keyName, holdableCheck.GetCheck(), dllName, dllPath); 
		},
		[this](const std::string& dllPath) { engineRef.UnsetScript(dllPath); },
		true,
		pParent
	), 
	engineRef(engine), keyName("") {}

BOOL CKeybindOptionDlg::OnInitDialog()
{
	DLLLoaderCommon::OnInitDialog();

	SetWindowText(_T("Keybind Options"));
	
	return true;
}

CKeybindOptionDlg::~CKeybindOptionDlg()
{
	if (pollForKeyPressThread)
	{
		engineRef.AbortKeyPressWait();
		pollForKeyPressThread->detach();
	}

	ResetPollForKeyPressThread();
}

void CKeybindOptionDlg::ResetPollForKeyPressThread()
{
	if (pollForKeyPressThread)
	{
		pollForKeyPressThread.reset();
	}
}

void CKeybindOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	DLLLoaderCommon::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, keybindInterButton);
	DDX_Control(pDX, IDC_EDIT1, keyNameEdit);
	DDX_Control(pDX, IDC_CHECK1, holdableCheck);
}


BEGIN_MESSAGE_MAP(CKeybindOptionDlg, DLLLoaderCommon)
	ON_BN_CLICKED(IDC_BUTTON1, &CKeybindOptionDlg::OnKeybindInterClick)
	ON_MESSAGE(BringEverettGuiBack, OnBringEverettGuiBack)
END_MESSAGE_MAP()


// CKeybindOptionDlg message handlers


void CKeybindOptionDlg::OnKeybindInterClick()
{
	engineRef.ForceFocusOnWindow("LGL");
	
	ResetPollForKeyPressThread();
	pollForKeyPressThread = std::make_unique<std::jthread>([this]() { SetEditToKeyName(); });
}

void CKeybindOptionDlg::SetEditToKeyName()
{
	keybindInterButton.EnableWindow(false);

	int keyID = engineRef.PollForLastKeyPressed();

	if (keyID != -2)
	{
		keyName = EverettEngine::ConvertKeyTo(keyID);
		SendMessage(BringEverettGuiBack);
		keyNameEdit.SetWindowTextW(keyName);
		BlockLoadScriptButton(false);
		keybindInterButton.EnableWindow(true);
	}
}

LRESULT CKeybindOptionDlg::OnBringEverettGuiBack(WPARAM wParam, LPARAM lParam)
{
	engineRef.ForceFocusOnWindow("EverettGUI");

	return 0;
}

