#pragma once
#include "afxdialogex.h"
#include <string>

// CSaveLoadDlg dialog

class CSaveLoadDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSaveLoadDlg)

public:
	enum class Mode
	{
		Save,
		Load
	};

	CSaveLoadDlg(Mode mode, const CString& saveFileType, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSaveLoadDlg();

	std::string GetFilePathAndName();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG6 };
#endif

protected:
	BOOL OnInitDialog() override;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	Mode chosenMode;
	CEdit filePathEdit;
	CButton browseButton;
	CEdit fileNameEdit;

	std::string filePathStr;
	std::string fileNameStr;

	CString saveFileType;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBrowseButtonClick();
	afx_msg void OnFileNameEditChanged();
};
