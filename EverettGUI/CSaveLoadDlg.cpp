// CSaveLoadDlg.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CSaveLoadDlg.h"
#include "CBrowseDialog.h"


// CSaveLoadDlg dialog

IMPLEMENT_DYNAMIC(CSaveLoadDlg, CDialogEx)

CSaveLoadDlg::CSaveLoadDlg(Mode mode, const CString& saveFileType, CWnd* pParent)
	: chosenMode(mode), saveFileType(saveFileType), CDialogEx(IDD_DIALOG6, pParent) {}

CSaveLoadDlg::~CSaveLoadDlg()
{
}

std::string CSaveLoadDlg::GetFilePathAndName()
{
	return chosenMode == Mode::Save ? filePathStr + fileNameStr : filePathStr;
}

void CSaveLoadDlg::OnBnClickedOk()
{
	CString filePath;
	filePathEdit.GetWindowTextW(filePath);

	filePathStr = CT2A(filePath);

	CString fileName;
	fileNameEdit.GetWindowTextW(fileName);

	fileNameStr = CT2A(fileName);

	CDialogEx::OnOK();
}

BOOL CSaveLoadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return true;
}

void CSaveLoadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, filePathEdit);
	DDX_Control(pDX, IDC_BUTTON1, browseButton);
	DDX_Control(pDX, IDC_EDIT2, fileNameEdit);
}


BEGIN_MESSAGE_MAP(CSaveLoadDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSaveLoadDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CSaveLoadDlg::OnBrowseButtonClick)
	ON_EN_CHANGE(IDC_EDIT2, &CSaveLoadDlg::OnFileNameEditChanged)
END_MESSAGE_MAP()


// CSaveLoadDlg message handlers


void CSaveLoadDlg::OnBrowseButtonClick()
{
	CString pathStr;
	CString fileStr = _T("");

	chosenMode == Mode::Save ? 
		CBrowseDialog::OpenAndGetFolderPath(pathStr) : 
		CBrowseDialog::OpenAndGetFilePath(pathStr, fileStr, {{"esav files", "*.esav"}});

	filePathEdit.SetWindowTextW(pathStr);
	fileNameEdit.SetWindowTextW(fileStr);
}


void CSaveLoadDlg::OnFileNameEditChanged()
{
}
