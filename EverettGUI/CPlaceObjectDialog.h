#pragma once
#include "afxdialogex.h"

#include <vector>
#include <string>
#include <functional>

#include "AdString.h"

// CPlaceObjectDialog dialog

class CPlaceObjectDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPlaceObjectDialog)
	using NameCheckFunc = std::function<std::string(const std::string&)>;

public:
	CPlaceObjectDialog(
		const AdString& objectTypeName, 
		const AdString& sourceObjectTypeName,
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
	AdString objectTypeName;
	AdString sourceObjectTypeName;
	size_t chosenIndex;
	AdString chosenObject;
	AdString newName;

	NameCheckFunc nameCheckFunc;

	CComboBox objectChoice;
	CStatic choiceLabel;
	CStatic nameLabel;
	CStatic nameWarning;

	CEdit nameEdit;
	CButton placeObjectButton;

	afx_msg void OnBnClickedOk();
	afx_msg void OnModelChoiceChange();
	afx_msg void OnNameEditChanged();
public:
	size_t   GetChosenIndex();
	AdString GetChosenObject();
	AdString GetNewObjectName();
};
