// CPlaceObjectDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CPlaceObjectDialog.h"
#include "EverettEngine.h"

#include "CommonStrEdits.h"
#include "NameEditChecker.h"

// CPlaceObjectDialog dialog

IMPLEMENT_DYNAMIC(CPlaceObjectDialog, CDialogEx)

CPlaceObjectDialog::CPlaceObjectDialog(
	EverettEngine& engineRef,
	const AdString& objectTypeName, 
	const AdString& sourceObjectTypeName,
	NameCheckFunc nameCheckFunc,
	CWnd* pParent
)
	: 
	CDialogEx(IDD_DIALOG2, pParent), 
	engineRef(engineRef),
	objectTypeName(objectTypeName), 
	sourceObjectTypeName(sourceObjectTypeName), 
	nameCheckFunc(nameCheckFunc),
	chosenIndex(-1)
{
}

CPlaceObjectDialog::~CPlaceObjectDialog()
{
}

BOOL CPlaceObjectDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(L"Place " + objectTypeName);
	choiceLabel.SetWindowTextW(L"Choose " + sourceObjectTypeName + L':');
	nameLabel.SetWindowTextW(objectTypeName + L" name:");

	if (objectTypeName == "Solid")
	{
		for (auto& object : engineRef.GetCreatedModelNames())
		{
			objectChoice.AddString(AdString(object));
		}
	}
	else
	{
		for (auto& object : engineRef.GetLightTypeList())
		{
			objectChoice.AddString(AdString(object));
		}
	}

	placeObjectButton.EnableWindow(false);
	placeObjectButton.SetWindowTextW(L"Place " + objectTypeName);

	return true;
}

void CPlaceObjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, objectChoice);
	DDX_Control(pDX, IDC_EDIT1, nameEdit);
	DDX_Control(pDX, IDC_CHOICE_LABEL, choiceLabel);
	DDX_Control(pDX, IDC_NAME_LABEL, nameLabel);
	DDX_Control(pDX, IDC_NAME_WARNING, nameWarning);
	DDX_Control(pDX, IDOK, placeObjectButton);
}


BEGIN_MESSAGE_MAP(CPlaceObjectDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPlaceObjectDialog::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPlaceObjectDialog::OnModelChoiceChange)
	ON_EN_CHANGE(IDC_EDIT1, &CPlaceObjectDialog::OnNameEditChanged)
END_MESSAGE_MAP()


// CPlaceObjectDialog message handlers


void CPlaceObjectDialog::OnBnClickedOk()
{
	nameEdit.GetWindowTextW(newName);
	std::string pathStdStrNameChecked = nameCheckFunc(newName);

	if (pathStdStrNameChecked != newName)
	{
		newName = pathStdStrNameChecked;
	}


	CDialogEx::OnOK();
}

size_t CPlaceObjectDialog::GetChosenIndex()
{
	return chosenIndex;
}

AdString CPlaceObjectDialog::GetChosenObject()
{
	return chosenObject;
}

AdString CPlaceObjectDialog::GetNewObjectName()
{
	return newName;
}

void CPlaceObjectDialog::OnModelChoiceChange()
{
	chosenIndex = objectChoice.GetCurSel();

	objectChoice.GetLBText(static_cast<int>(chosenIndex), chosenObject);

	AdString modelStdStrChecked = nameCheckFunc(chosenObject);

	nameEdit.SetWindowTextW(modelStdStrChecked);
}


void CPlaceObjectDialog::OnNameEditChanged()
{
	NameEditChecker::CheckAndEditName(nameEdit, nameWarning);

	//AdString nameStr;
	//nameEdit.GetWindowTextW(nameStr);
	//placeObjectButton.EnableWindow(chosenIndex != -1 && !nameStr.empty());

	placeObjectButton.EnableWindow(!MFCUtilities::EditIsEmpty(nameEdit));
}
