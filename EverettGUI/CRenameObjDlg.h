#pragma once
#include "afxdialogex.h"

#include "NameEditChecker.h"
#include "AdString.h"

class CRenameObjDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRenameObjDlg)

public:
	CRenameObjDlg(const AdString& oldName, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRenameObjDlg();

	AdString GetNewName();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG9 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	BOOL OnInitDialog();

	AdString oldName;
	AdString newName;
	
	CEdit oldNameBox;
	CEdit newNameBox;
	CStatic renameWarning;
	CButton okButton;
public:
	afx_msg void OnNewNameBoxChange();
};
