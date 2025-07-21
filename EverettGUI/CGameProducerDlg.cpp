// CGameProducerDlg.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CGameProducerDlg.h"

#include "CBrowseDialog.h"

#include <fstream>


// CGameProducerDlg dialog

IMPLEMENT_DYNAMIC(CGameProducerDlg, CDialogEx)

CGameProducerDlg::CGameProducerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG7, pParent)
{

}

CGameProducerDlg::~CGameProducerDlg()
{
}

void CGameProducerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, windowWidthEdit);
	DDX_Control(pDX, IDC_EDIT2, windowHeightEdit);
	DDX_Control(pDX, IDC_CHECK1, fullscreenForceCheck);
	DDX_Control(pDX, IDC_EDIT10, gameTitleEdit);
	DDX_Control(pDX, IDC_EDIT12, startWorldSaveEdit);
	DDX_Control(pDX, IDC_EDIT13, gameFolderEdit);
	DDX_Control(pDX, IDC_CHECK2, debugTextCheck);
	DDX_Control(pDX, IDC_CHECK3, defaultWASDCheck);
}


BEGIN_MESSAGE_MAP(CGameProducerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CGameProducerDlg::OnBrowseButtonClick)
	ON_BN_CLICKED(IDOK, &CGameProducerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON8, &CGameProducerDlg::OnGameFolderBrowseClick)
END_MESSAGE_MAP()

BOOL CGameProducerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	windowWidthEdit.SetWindowTextW(L"800");
	windowHeightEdit.SetWindowTextW(L"600");
	gameTitleEdit.SetWindowTextW(L"Game");

	return true;
}

void CGameProducerDlg::CopyFoldersToGameFolder(const AdString& gameFolderStr)
{
	for (auto& folderName : foldersToCopy)
	{
		std::filesystem::copy(assetPath + '\\' + folderName, gameFolderStr + '\\' + folderName);
	}
}

void CGameProducerDlg::CopyFilesToGameFolder(const AdString& gameFolderStr)
{
	for (auto& filename : filesToCopy)
	{
		std::filesystem::copy_file(exePath + '\\' + filename, gameFolderStr + '\\' + filename);
	}
}

void CGameProducerDlg::RenameGameExecutable(const AdString& gameFolderStr, const AdString& gameNameStr)
{
	std::filesystem::rename(gameFolderStr + '\\' + filesToCopy[0], gameFolderStr + '\\' + gameNameStr + ".exe");
}

void CGameProducerDlg::CreateConfigFile(const AdString& gameFolderStr, const std::vector<std::pair<AdString, AdString>>& configParams)
{
	std::fstream file(gameFolderStr + '\\' + configFileName, std::ios::out);

	for (auto& currentParam : configParams)
	{
		file << currentParam.first + '=' + currentParam.second + '\n';
	}
}

// CGameProducerDlg message handlers

void CGameProducerDlg::OnBrowseButtonClick()
{
	AdString pathStr;
	AdString fileStr;

	if (CBrowseDialog::OpenAndGetFilePath(pathStr, fileStr, {{"esav files", "*.esav"}}))
	{
		startWorldSaveEdit.SetWindowTextW(pathStr);
	}
}

void CGameProducerDlg::OnGameFolderBrowseClick()
{
	AdString pathStr;

	if (CBrowseDialog::OpenAndGetFolderPath(pathStr))
	{
		gameFolderEdit.SetWindowTextW(pathStr);
	}
}

void CGameProducerDlg::OnBnClickedOk()
{
	auto BoolToStr = [](bool value)
	{
		return value ? L"1" : L"0";
	};

	AdString gameFolderStr;
	gameFolderEdit.GetWindowTextW(gameFolderStr);

	AdString gameTitleStr;
	gameTitleEdit.GetWindowTextW(gameTitleStr);

	CopyFoldersToGameFolder(gameFolderStr);
	CopyFilesToGameFolder(gameFolderStr);
	RenameGameExecutable(gameFolderStr, gameTitleStr);

	AdString windowWidthStr;
	AdString windowHeightStr;
	AdString startWorldSaveStr;

	windowWidthEdit.GetWindowTextW(windowWidthStr);
	windowHeightEdit.GetWindowTextW(windowHeightStr);
	startWorldSaveEdit.GetWindowTextW(startWorldSaveStr);

	CreateConfigFile(gameFolderStr, {
		{ "WindowWidth",     windowWidthStr                             },
		{ "WindowHeight",    windowHeightStr                            },
		{ "WindowTitle",     gameTitleStr                               },
		{ "StartSave",       startWorldSaveStr                          },
		{ "FullscreenForce", BoolToStr(fullscreenForceCheck.GetCheck()) },
		{ "DebugText",       BoolToStr(debugTextCheck.GetCheck())       },
		{ "DefaultWASD",     BoolToStr(defaultWASDCheck.GetCheck())     }
	});

	CDialogEx::OnOK();
}