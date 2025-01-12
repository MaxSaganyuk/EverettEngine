// CLoadModelDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CLoadModelDialog.h"
#include "CBrowseDialog.h"

#include <fstream>

// CLoadModelDialog dialog

IMPLEMENT_DYNAMIC(CLoadModelDialog, CDialogEx)

CLoadModelDialog::CLoadModelDialog(
	ModelLoaderFunc modelLoader, 
	const std::vector<std::string>& loadedModelList, 
	CWnd* pParent
)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	this->modelLoader = modelLoader;
	this->loadedModelList = loadedModelList;
	path = "";
	name = "";
}

CLoadModelDialog::~CLoadModelDialog()
{
}

BOOL CLoadModelDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	TCHAR buffer[MAX_PATH];
	std::ifstream file;
	file.open(cacheFileName, std::ios::in);
	
	if (file)
	{
		std::string path;
		file >> path;
		UpdateModelChoice(CA2T(path.c_str()));
	}
	else if (GetCurrentDirectory(MAX_PATH, buffer))
	{
		UpdateModelChoice(buffer);
	}

	file.close();

	return true;
}

void CLoadModelDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, modelFolderEdit);
	DDX_Control(pDX, IDC_BUTTON1, browseButton);
	DDX_Control(pDX, IDC_COMBO1, modelChoice);
	DDX_Control(pDX, IDOK, loadModelButton);
	DDX_Control(pDX, IDC_EDIT10, nameEdit);
}


BEGIN_MESSAGE_MAP(CLoadModelDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CLoadModelDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CLoadModelDialog::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CLoadModelDialog::OnModelSelection)
	ON_BN_CLICKED(IDCANCEL, &CLoadModelDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// CLoadModelDialog message handlers

void CLoadModelDialog::UpdateModelChoice(const LPCTSTR filePath)
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

void CLoadModelDialog::OnBnClickedButton1()
{
	CString pathStr;
	
	if (CBrowseDialog::OpenAndGetFolderPath(pathStr))
	{
		UpdateModelChoice(pathStr);
	}
}


void CLoadModelDialog::OnBnClickedOk()
{
	std::fstream file(cacheFileName, std::ios::out, std::ios::trunc);

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

void CLoadModelDialog::OnModelSelection()
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

std::string CLoadModelDialog::GetChosenPath()
{
	return path;
}

std::string CLoadModelDialog::GetChosenName()
{
	return name;
}

std::string CLoadModelDialog::GetChosenFilename()
{
	return filename;
}

std::string CLoadModelDialog::GetChosenPathAndFilename()
{
	return GetChosenPath() + '\\' + GetChosenFilename();
}

void CLoadModelDialog::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
