// CLoadModelDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CBrowseAndLoadDialog.h"
#include "CBrowseDialog.h"

#include <fstream>

// CLoadModelDialog dialog

IMPLEMENT_DYNAMIC(CBrowseAndLoadDialog, CDialogEx)

CBrowseAndLoadDialog::CBrowseAndLoadDialog(
	const std::string& objectName,
	LoaderFunc modelLoader, 
	const std::vector<std::string>& loadedModelList, 
	CWnd* pParent
)
	: 
	CDialogEx(IDD_DIALOG1, pParent), 
	objectName(objectName),
	modelLoader(modelLoader), 
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

	SetWindowText(CA2T(("Load " + objectName).c_str()));
	folderLabel.SetWindowTextW(CA2T((objectName + " folder:").c_str()));
	choiceLabel.SetWindowTextW(CA2T((objectName + ':').c_str()));

	TCHAR buffer[MAX_PATH];
	std::ifstream file;
	file.open(cacheFileName + objectName, std::ios::in);
	
	if (file)
	{
		std::string path;
		file >> path;
		UpdateObjectChoice(CA2T(path.c_str()));
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
}


BEGIN_MESSAGE_MAP(CBrowseAndLoadDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CBrowseAndLoadDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CBrowseAndLoadDialog::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CBrowseAndLoadDialog::OnObjectSelection)
END_MESSAGE_MAP()


// CLoadModelDialog message handlers

void CBrowseAndLoadDialog::UpdateObjectChoice(const LPCTSTR filePath)
{
	modelFolderEdit.SetWindowTextW(filePath);

	std::string filePathSTD = CT2A(filePath);
	std::vector<std::string> files = modelLoader(filePathSTD);

	modelChoice.Clear();
	for (auto& file : files)
	{
		modelChoice.AddString(CA2T(file.c_str()));
	}
}

void CBrowseAndLoadDialog::OnBnClickedButton1()
{
	CString pathStr;
	
	if (CBrowseDialog::OpenAndGetFolderPath(pathStr))
	{
		UpdateObjectChoice(pathStr);
	}
}


void CBrowseAndLoadDialog::OnBnClickedOk()
{
	std::fstream file(cacheFileName + objectName, std::ios::out, std::ios::trunc);

	file << GetChosenPath();
	
	file.close();

	CString nameStr;
	nameEdit.GetWindowTextW(nameStr);
	name = CT2A(nameStr);

	for (auto& loadedModelName : loadedModelList)
	{
		if (loadedModelName == name)
		{
			name += '_';
		}
	}

	CDialogEx::OnOK();
}

void CBrowseAndLoadDialog::OnObjectSelection()
{
	bool curSelExists = modelChoice.GetCurSel() != -1;
	
	if (curSelExists)
	{
		CString pathStr;
		modelFolderEdit.GetWindowTextW(pathStr);

		CString filenameStr;
		modelChoice.GetLBText(modelChoice.GetCurSel(), filenameStr);

		filename = CT2A(filenameStr);
		name = filename.substr(0, filename.find('.'));
		path = CT2A(pathStr);
	}

	nameEdit.SetWindowTextW(CA2T(GetChosenName().c_str()));
	loadModelButton.EnableWindow(curSelExists);
}

std::string CBrowseAndLoadDialog::GetChosenPath()
{
	return path;
}

std::string CBrowseAndLoadDialog::GetChosenName()
{
	return name;
}

std::string CBrowseAndLoadDialog::GetChosenFilename()
{
	return filename;
}

std::string CBrowseAndLoadDialog::GetChosenPathAndFilename()
{
	return GetChosenPath() + '\\' + GetChosenFilename();
}
