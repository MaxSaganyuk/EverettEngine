// CCopyDlg.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CCopyObjDlg.h"


// CCopyDlg dialog

IMPLEMENT_DYNAMIC(CCopyObjDlg, CTrackedDlg)

CCopyObjDlg::CCopyObjDlg(CWnd* pParent /*=nullptr*/)
	: CTrackedDlg(IDD_DIALOG10, pParent)
{

}

CCopyObjDlg::~CCopyObjDlg()
{
}

size_t CCopyObjDlg::GetPickedAmount()
{
	return pickedAmount;
}

void CCopyObjDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, amountEdit);
}

void CCopyObjDlg::OnBnClickedOk()
{
	CString amountEditStr;
	amountEdit.GetWindowTextW(amountEditStr);

	pickedAmount = _tstoi(amountEditStr);

	CDialogEx::OnOK();
}

BOOL CCopyObjDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	amountEdit.SetWindowTextW(L"1");

	return 0;
}


BEGIN_MESSAGE_MAP(CCopyObjDlg, CTrackedDlg)
	ON_BN_CLICKED(IDOK, &CCopyObjDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCopyDlg message handlers
