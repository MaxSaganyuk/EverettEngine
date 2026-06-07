// CRenameObjDlg.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CRenameObjDlg.h"

#include "AdString.h"

// CRenameObjDlg dialog

IMPLEMENT_DYNAMIC(CRenameObjDlg, CDialogEx)

CRenameObjDlg::CRenameObjDlg(const AdString& oldName, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG9, pParent), oldName(oldName)
{

}

CRenameObjDlg::~CRenameObjDlg()
{
}

AdString CRenameObjDlg::GetNewName()
{
	return newName;
}

void CRenameObjDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RENAME_WARNING, renameWarning);
	DDX_Control(pDX, IDC_EDIT2, newNameBox);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDC_EDIT1, oldNameBox);
}

BOOL CRenameObjDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	oldNameBox.SetWindowTextW(oldName);
	renameWarning.ShowWindow(false);
	okButton.EnableWindow(false);

	return 0;
}


BEGIN_MESSAGE_MAP(CRenameObjDlg, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT2, &CRenameObjDlg::OnNewNameBoxChange)
	ON_BN_CLICKED(IDOK, &CRenameObjDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRenameObjDlg message handlers

void CRenameObjDlg::OnNewNameBoxChange()
{
	NameEditChecker::CheckAndEditName(newNameBox, renameWarning);

	newNameBox.GetWindowTextW(newName);

	bool newNameEmpty = newName.empty();
	bool namesSame = oldName == newName;
	
	bool showError = newNameEmpty || namesSame;

	if (showError)
	{
		renameWarning.SetWindowTextW(namesSame ? sameNameError : nameExistsError);
	}

	okButton.EnableWindow(!showError);
}

void CRenameObjDlg::OnBnClickedOk()
{
	newName = NameEditChecker::GetNameCheckedString(newName);

	CDialogEx::OnOK();
}
