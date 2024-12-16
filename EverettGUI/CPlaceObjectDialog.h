#pragma once
#include "afxdialogex.h"

#include <vector>
#include <string>

// CPlaceObjectDialog dialog

class CPlaceObjectDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPlaceObjectDialog)

public:
	CPlaceObjectDialog(
		const std::string& objectTypeName, 
		const std::vector<std::string>& objectNameList, 
		CWnd* pParent = nullptr
	);   // standard constructor
	virtual ~CPlaceObjectDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	std::vector<std::string> objectNameList;
	size_t chosenIndex;
	std::string chosenObject;
	std::string newName;
	CComboBox objectChoice;
public:
	afx_msg void OnBnClickedOk();

	size_t      GetChosenIndex();
	std::string GetChosenObject();
	std::string GetNewObjectName();
private:
	CEdit nameEdit;
public:
	afx_msg void OnModelChoiceChange();
};
