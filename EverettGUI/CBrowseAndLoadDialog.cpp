// CLoadModelDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CBrowseAndLoadDialog.h"
#include "CBrowseDialog.h"

#include "CommonStrEdits.h"
#include "NameEditChecker.h"

#include <fstream>

// CLoadModelDialog dialog

IMPLEMENT_DYNAMIC(CBrowseAndLoadDialog, CDialogEx)

CBrowseAndLoadDialog::CBrowseAndLoadDialog(
	const AdString& objectName,
	LoaderFunc modelLoader, 
	NameCheckFunc nameCheckFunc,
	const std::vector<std::string>& loadedModelList,
	CWnd* pParent
)
	: 
	CDialogEx(IDD_DIALOG1, pParent), 
	objectName(objectName),
	modelLoader(modelLoader), 
	nameCheckFunc(nameCheckFunc),
	loadedModelList(loadedModelList), 
	path(""), 
	name("")
{
}

CBrowseAndLoadDialog::~CBrowseAndLoadDialog()
{
}

BOOL CBrowseAndLoadDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(L"Load " + objectName);
	folderLabel.SetWindowTextW(objectName + L" folder:");
	choiceLabel.SetWindowTextW(objectName + L':');

	TCHAR buffer[MAX_PATH];
	std::ifstream file;
	file.open(cacheFileName + objectName, std::ios::in);
	
	if (file)
	{
		std::string path;
		file >> path;
		AdString adPath(path);
		UpdateObjectChoice(adPath);
	}
	else if (GetCurrentDirectory(MAX_PATH, buffer))
	{
		UpdateObjectChoice(buffer);
	}

	file.close();

	return true;
}

void CBrowseAndLoadDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, modelFolderEdit);
	DDX_Control(pDX, IDC_BUTTON1, browseButton);
	DDX_Control(pDX, IDC_COMBO1, modelChoice);
	DDX_Control(pDX, IDOK, loadModelButton);
	DDX_Control(pDX, IDC_EDIT10, nameEdit);
	DDX_Control(pDX, IDC_FOLDER_LABEL, folderLabel);
	DDX_Control(pDX, IDC_CHOICE_LABEL, choiceLabel);
	DDX_Control(pDX, IDC_NAME_WARNING, nameWarning);
}


BEGIN_MESSAGE_MAP(CBrowseAndLoadDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CBrowseAndLoadDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CBrowseAndLoadDialog::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CBrowseAndLoadDialog::OnObjectSelection)
	ON_EN_CHANGE(IDC_EDIT10, &CBrowseAndLoadDialog::OnNameEditChanged)
END_MESSAGE_MAP()


// CLoadModelDialog message handlers

void CBrowseAndLoadDialog::UpdateObjectChoice(const AdString& filePath)
{
	modelFolderEdit.SetWindowTextW(filePath);

	std::vector<std::string> files = modelLoader(filePath);

	modelChoice.Clear();
	for (auto& file : files)
	{
		modelChoice.AddString(AdString(file));
	}
}

void CBrowseAndLoadDialog::OnBnClickedButton1()
{
	AdString pathStr;

	if (CBrowseDialog::OpenAndGetFolderPath(pathStr))
	{
		UpdateObjectChoice(pathStr);
	}
}


void CBrowseAndLoadDialog::OnBnClickedOk()
{
	modelFolderEdit.GetWindowTextW(path);
	modelChoice.GetLBText(modelChoice.GetCurSel(), filename);

	if (nameWarning.IsWindowVisible())
	{
		nameEdit.GetWindowTextW(name);
		std::string pathStdStrNameChecked = nameCheckFunc(name);

		if (pathStdStrNameChecked != name)
		{
			name = pathStdStrNameChecked;
		}
	}

	std::fstream file(cacheFileName + objectName, std::ios::out, std::ios::trunc);
	std::string& chosenPath = path;

	file << chosenPath;
	
	file.close();

	CDialogEx::OnOK();
}

void CBrowseAndLoadDialog::OnObjectSelection()
{
	bool curSelExists = modelChoice.GetCurSel() != -1;
	
	modelChoice.GetLBText(modelChoice.GetCurSel(), filename);

	std::string& filenameRef = filename;
	name = filenameRef.substr(0, filenameRef.find('.'));

	nameEdit.SetWindowTextW(GetChosenName());
	loadModelButton.EnableWindow(curSelExists);
}

AdString CBrowseAndLoadDialog::GetChosenPath()
{
	return path;
}

AdString CBrowseAndLoadDialog::GetChosenName()
{
	return name;
}

AdString CBrowseAndLoadDialog::GetChosenFilename()
{
	return filename;
}

AdString CBrowseAndLoadDialog::GetChosenPathAndFilename()
{
	return GetChosenPath() + '\\' + GetChosenFilename();
}

void CBrowseAndLoadDialog::OnNameEditChanged()
{
	NameEditChecker::CheckAndEditName(nameEdit, nameWarning);
}
