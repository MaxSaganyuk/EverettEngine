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

CGameProducerDlg::CGameProducerDlg(EverettEngine& engineRef, CWnd* pParent /*=nullptr*/)
	: engineRef(engineRef), initialized(false), CDialogEx(IDD_DIALOG7, pParent)
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
	DDX_Control(pDX, IDOK, okButton);
}


BEGIN_MESSAGE_MAP(CGameProducerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CGameProducerDlg::OnBrowseButtonClick)
	ON_BN_CLICKED(IDOK, &CGameProducerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON8, &CGameProducerDlg::OnGameFolderBrowseClick)
	ON_EN_CHANGE(IDC_EDIT1, &CGameProducerDlg::OnWidthEditChange)
	ON_EN_CHANGE(IDC_EDIT2, &CGameProducerDlg::OnHeightEditChange)
	ON_EN_CHANGE(IDC_EDIT10, &CGameProducerDlg::OnGameTitleEditChange)
	ON_EN_CHANGE(IDC_EDIT13, &CGameProducerDlg::OnGameFolderEditChange)
	ON_EN_CHANGE(IDC_EDIT12, &CGameProducerDlg::OnStartWorldSaveEditChange)
END_MESSAGE_MAP()

BOOL CGameProducerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	windowWidthEdit.SetWindowTextW(L"800");
	windowHeightEdit.SetWindowTextW(L"600");
	gameTitleEdit.SetWindowTextW(L"Game");

	okButton.EnableWindow(false);

	initialized = true;

	return true;
}

AdString CGameProducerDlg::GetFileFromPath(const AdString& path)
{
	std::string file = path;
	return file.substr(file.rfind('\\') + 1);
}

AdString CGameProducerDlg::ToLowercase(const AdString& str)
{
	std::string stdStr = str;
	std::transform(stdStr.begin(), stdStr.end(), stdStr.begin(), [](unsigned char c) { return std::tolower(c); });

	return stdStr;
}

void CGameProducerDlg::CreateFoldersInGameFolder()
{
	for (int i = 0; i < EverettStructs::AssetPaths::GetAssetTypeAmount(); ++i)
	{
		std::filesystem::create_directory(
			gameFolderStr + '\\' + ToLowercase(EverettStructs::AssetPaths::GetAssetNameIndex(i))
		);
	}
}

void CGameProducerDlg::CopyFoldersToGameFolder()
{
	for (auto& folderName : foldersToCopy)
	{
		std::filesystem::copy(assetPath + '\\' + folderName, gameFolderStr + '\\' + folderName);
	}
}

void CGameProducerDlg::CopyFilesToGameFolder()
{
	for (auto& filename : filesToCopy)
	{
		std::filesystem::copy_file(exePath + '\\' + filename, gameFolderStr + '\\' + filename);
	}
}

void CGameProducerDlg::RenameGameExecutable(const AdString& gameNameStr)
{
	std::filesystem::rename(gameFolderStr + '\\' + filesToCopy[0], gameFolderStr + '\\' + gameNameStr + ".exe");
}

void CGameProducerDlg::CreateConfigFile(const std::vector<std::pair<AdString, AdString>>& configParams)
{
	std::fstream file(gameFolderStr + '\\' + configFileName, std::ios::out);

	for (auto& currentParam : configParams)
	{
		file << currentParam.first + '=' + currentParam.second + '\n';
	}
}

EverettStructs::AssetPaths CGameProducerDlg::GetPathsFromWorldFile(
	const AdString& worldPathStr, 
	const AdString& worldFileName
)
{
	EverettStructs::AssetPaths assetPaths = engineRef.GetPathsFromWorldFile(worldPathStr);
	engineRef.HidePathsInWorldFile(
		worldPathStr, 
		gameFolderStr + "//" + ToLowercase(EverettStructs::AssetPaths::GetAssetNameIndex(0)) + "//" + worldFileName
	);

	return assetPaths;
}

void CGameProducerDlg::CopyAssetsToGamePath(const EverettStructs::AssetPaths& assetPaths)
{
	for (int i = 1; i < EverettStructs::AssetPaths::GetAssetTypeAmount(); ++i)
	{
		for (auto& path : assetPaths[i])
		{
			AdString fileName = GetFileFromPath(path);
			std::filesystem::copy(
				path, 
				gameFolderStr + "\\" + ToLowercase(EverettStructs::AssetPaths::GetAssetNameIndex(i)) + "\\" + fileName
			);
		}
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

	gameFolderEdit.GetWindowTextW(gameFolderStr);

	AdString gameTitleStr;
	gameTitleEdit.GetWindowTextW(gameTitleStr);

	CreateFoldersInGameFolder();
	CopyFoldersToGameFolder();
	CopyFilesToGameFolder();
	RenameGameExecutable(gameTitleStr);

	AdString windowWidthStr;
	AdString windowHeightStr;
	AdString startWorldSaveStr;

	windowWidthEdit.GetWindowTextW(windowWidthStr);
	windowHeightEdit.GetWindowTextW(windowHeightStr);
	startWorldSaveEdit.GetWindowTextW(startWorldSaveStr);

	AdString worldFileName = GetFileFromPath(startWorldSaveStr);

	CopyAssetsToGamePath(GetPathsFromWorldFile(startWorldSaveStr, worldFileName));

	CreateConfigFile({
		{ "WindowWidth",     windowWidthStr                             },
		{ "WindowHeight",    windowHeightStr                            },
		{ "WindowTitle",     gameTitleStr                               },
		{ "StartSave",       worldFileName                              },
		{ "FullscreenForce", BoolToStr(fullscreenForceCheck.GetCheck()) },
		{ "DebugText",       BoolToStr(debugTextCheck.GetCheck())       },
		{ "DefaultWASD",     BoolToStr(defaultWASDCheck.GetCheck())     }
	});

	CDialogEx::OnOK();
}

void CGameProducerDlg::AreEditsFilled()
{
	if (initialized)
	{
		okButton.EnableWindow(!MFCUtilities::EditsAnyEmpty({
			&windowWidthEdit,
			&windowHeightEdit,
			&gameTitleEdit,
			&gameFolderEdit,
			&startWorldSaveEdit
			}));
	}
}

void CGameProducerDlg::OnWidthEditChange()
{
	AreEditsFilled();
}

void CGameProducerDlg::OnHeightEditChange()
{
	AreEditsFilled();
}

void CGameProducerDlg::OnGameTitleEditChange()
{
	AreEditsFilled();
}

void CGameProducerDlg::OnGameFolderEditChange()
{
	AreEditsFilled();
}

void CGameProducerDlg::OnStartWorldSaveEditChange()
{
	AreEditsFilled();
}
