#pragma once
#include "afxdialogex.h"

#include <vector>
#include <string>
#include <functional>

// CPlaceObjectDialog dialog

class CPlaceObjectDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPlaceObjectDialog)
	using NameCheckFunc = std::function<std::string(const std::string&)>;

public:
	CPlaceObjectDialog(
		const std::string& objectTypeName, 
		const std::string& sourceObjectTypeName,
		NameCheckFunc nameCheckFunc,
		const std::vector<std::string>& objectNameList = {},
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
	std::string objectTypeName;
	std::string sourceObjectTypeName;
	size_t chosenIndex;
	std::string chosenObject;
	std::string newName;

	NameCheckFunc nameCheckFunc;

	CComboBox objectChoice;
	CStatic choiceLabel;
	CStatic nameLabel;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnModelChoiceChange();

	size_t      GetChosenIndex();
	std::string GetChosenObject();
	std::string GetNewObjectName();
private:
	CEdit nameEdit;
};
