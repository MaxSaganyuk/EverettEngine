#pragma once
#include "afxdialogex.h"

#include "CTrackedDlg.h"

// CCopyDlg dialog

class CCopyObjDlg : public CTrackedDlg
{
	DECLARE_DYNAMIC(CCopyObjDlg)

public:
	CCopyObjDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCopyObjDlg();

	size_t GetPickedAmount();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG10 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnBnClickedOk();

	BOOL OnInitDialog();

	CEdit amountEdit;
	size_t pickedAmount{};
};
