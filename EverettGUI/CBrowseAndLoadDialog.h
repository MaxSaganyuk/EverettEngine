#pragma once
#include "afxdialogex.h"
#include <functional>
#include <string>
#include <vector>
#include "AdString.h"

// CLoadModelDialog dialog

class CBrowseAndLoadDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CBrowseAndLoadDialog)
	using LoaderFunc = std::function<std::vector<std::string>(const std::string&)>;
	using NameCheckFunc = std::function<std::string(const std::string&)>;

public:
	CBrowseAndLoadDialog(
		const AdString& objectName,
		LoaderFunc loader,
		NameCheckFunc nameCheckFunc,
		const std::vector<std::string>& loadedObjectsList, 
		CWnd* pParent = nullptr
	);   // standard constructor
	virtual ~CBrowseAndLoadDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void UpdateObjectChoice(const AdString& filePath);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnObjectSelection();
	afx_msg void OnNameEditChanged();

	CEdit modelFolderEdit;
	CButton browseButton;
	CButton loadModelButton;
	CComboBox modelChoice;
	CStatic folderLabel;
	CStatic choiceLabel;
	CEdit nameEdit;
	CStatic nameWarning;

	AdString objectName;
	AdString path;
	AdString filename;
	AdString name;

	LoaderFunc modelLoader;
	NameCheckFunc nameCheckFunc;
	std::vector<std::string> loadedModelList;
		
	constexpr static const char cacheFileName[] = "loadCache";
public:
	AdString GetChosenPath();
	AdString GetChosenFilename();
	AdString GetChosenName();
	AdString GetChosenPathAndFilename();
};
