#pragma once
#include "afxdialogex.h"

#include "AdString.h"
#include <vector>
#include <filesystem>
#include <unordered_set>

#include "EverettEngine.h"

// CGameProducerDlg dialog

class CGameProducerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGameProducerDlg)

public:
	CGameProducerDlg(EverettEngine& engineRef, CWnd* pParent = nullptr);   // standard constructor
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
	CButton okButton;

	AdString gameFolderStr;

	EverettEngine& engineRef;

	bool initialized;

	struct AssetPaths
	{
		std::unordered_set<std::string> modelPaths;
		std::unordered_set<std::string> soundPaths;
		std::unordered_set<std::string> scriptPaths;

		const std::unordered_set<std::string>& operator[](size_t index) const
		{
			// Indexes correspond to foldersToCreate
			switch (index)
			{
			case 0:
				throw; // worlds are not collected
			case 1:
				return modelPaths;
			case 2:
				return soundPaths;
			case 3:
				return scriptPaths;
			default:
				throw;
			}
		}
	};

	static inline std::vector<AdString> foldersToCreate
	{
		"worlds",
		"models",
		"sounds",
		"scripts"
	};

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
#ifdef _DEBUG
		"assimp-vc143-mtd.dll",
#else
		"assimp-vc143-mt.dll",
#endif
		"freetype.dll",
		"OpenAL32.dll"
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

	AdString GetFileFromPath(const AdString& path);

	AssetPaths GetPathsFromWorldFile(
		const AdString& worldPathStr,
		const AdString& worldFileName
	);
	void CopyAssetsToGamePath(const AssetPaths& assetPaths);

	void CreateFoldersInGameFolder();
	void CopyFoldersToGameFolder();
	void CopyFilesToGameFolder();
	void RenameGameExecutable(const AdString& gameNameStr);
	void CreateConfigFile(const std::vector<std::pair<AdString, AdString>>& configParams);

	void AreEditsFilled();

	afx_msg void OnGameFolderBrowseClick();
	afx_msg void OnBrowseButtonClick();
	afx_msg void OnBnClickedOk();
	afx_msg void OnWidthEditChange();
	afx_msg void OnHeightEditChange();
	afx_msg void OnGameTitleEditChange();
	afx_msg void OnGameFolderEditChange();
	afx_msg void OnStartWorldSaveEditChange();
};
