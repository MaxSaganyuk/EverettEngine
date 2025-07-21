#pragma once
#include "afxdialogex.h"

#include "AdString.h"
#include <vector>
#include <filesystem>

// CGameProducerDlg dialog

class CGameProducerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGameProducerDlg)

public:
	CGameProducerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CGameProducerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG7 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CEdit windowWidthEdit;
	CEdit windowHeightEdit;
	CEdit gameTitleEdit;
	CEdit gameFolderEdit;
	CEdit startWorldSaveEdit;

	CButton fullscreenForceCheck;
	CButton debugTextCheck;
	CButton defaultWASDCheck;

	static inline std::vector<AdString> foldersToCopy
	{
		"fonts",
		"shaders"
	};

	static inline std::vector<AdString> filesToCopy
	{
		"EverettPlayer.exe",
		"LGL.dll",
		"EverettCore.dll",
		"assimp-vc143-mtd.dll",
		"freetype.dll"
	};

	constexpr static const char configFileName[] = "config.ini";
#ifdef _DEBUG
	std::string assetPath = "..\\ProjectEverett\\";
	std::string exePath = "..\\x64\\Debug";
#else // _DEBUG
	std::string assetPath = ".";
	std::string exePath = ".";
#endif

	BOOL OnInitDialog() override;

	void CopyFoldersToGameFolder(const AdString& gameFolderStr);
	void CopyFilesToGameFolder(const AdString& gameFolderStr);
	void RenameGameExecutable(const AdString& gameFolderStr, const AdString& gameNameStr);
	void CreateConfigFile(const AdString& gameFolderStr, const std::vector<std::pair<AdString, AdString>>& configParams);

	afx_msg void OnGameFolderBrowseClick();
	afx_msg void OnBrowseButtonClick();
	afx_msg void OnBnClickedOk();
};
