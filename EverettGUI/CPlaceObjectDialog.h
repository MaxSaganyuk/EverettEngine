#pragma once
#include "afxdialogex.h"

#include <string>
#include <functional>
#include <generator>

#include "AdString.h"
#include "NameEditChecker.h"

// CPlaceObjectDialog dialog

class CPlaceObjectDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPlaceObjectDialog)

public:
	CPlaceObjectDialog(
		const AdString& objectTypeName, 
		const AdString& sourceObjectTypeName,
		NameEditChecker::NameCheckFunc nameCheckFunc,
		std::generator<std::string_view>&& objectNameList,
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
	std::generator<std::string_view> objectNameList;
	AdString objectTypeName;
	AdString sourceObjectTypeName;
	size_t chosenIndex;
	AdString chosenObject;
	AdString newName;

	NameEditChecker::NameCheckFunc nameCheckFunc;

	CComboBox objectChoice;
	CStatic choiceLabel;
	CStatic nameLabel;
	CStatic nameWarning;

	CEdit nameEdit;
	CButton placeObjectButton;

	afx_msg void OnBnClickedOk();
	afx_msg void OnModelChoiceChange();
	afx_msg void OnNameEditChanged();

	void UpdateOKButton();
public:
	size_t   GetChosenIndex();
	AdString GetChosenObject();
	AdString GetNewObjectName();
};
