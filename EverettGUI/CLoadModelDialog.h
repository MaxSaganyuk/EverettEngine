#pragma once
#include "afxdialogex.h"
#include <functional>
#include <string>
#include <vector>

// CLoadModelDialog dialog

class CLoadModelDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadModelDialog)
	using ModelLoaderFunc = std::function<std::vector<std::string>(const std::string&)>;

public:
	CLoadModelDialog(
		ModelLoaderFunc modelLoader, 
		const std::vector<std::string>& loadedModelList, 
		CWnd* pParent = nullptr
	);   // standard constructor
	virtual ~CLoadModelDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void UpdateModelChoice(const LPCTSTR filePath);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnModelSelection();

	CEdit modelFolderEdit;
	CButton browseButton;
	CButton loadModelButton;
	CComboBox modelChoice;

	std::string path;
	std::string filename;
	std::string name;

    ModelLoaderFunc modelLoader;
	std::vector<std::string> loadedModelList;
		
	constexpr static const char cacheFileName[] = "loadCache";
public:
	std::string GetChosenPath();
	std::string GetChosenFilename();
	std::string GetChosenName();
	std::string GetChosenPathAndFilename();
private:
	CEdit nameEdit;
public:
	afx_msg void OnBnClickedCancel();
};
