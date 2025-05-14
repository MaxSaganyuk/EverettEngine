// CLoadingDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CLoadingDialog.h"

#include "AdString.h"

#include <thread>

// CLoadingDialog dialog

IMPLEMENT_DYNAMIC(CLoadingDialog, CDialogEx)

CLoadingDialog::CLoadingDialog(
	const std::vector<std::function<bool()>>& loadingFuncs, 
	CWnd* pParent
)
	: CDialogEx(IDD_DIALOG3, pParent)
{
	success = false;
	this->loadingFuncs = loadingFuncs;
}

CLoadingDialog::~CLoadingDialog()
{
}

BOOL CLoadingDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(_T("Load Dialog"));

	okButton.EnableWindow(false);

	loadingProgressBar.SetRange32(0, loadingFuncs.size());
	loadingProgressBar.SetStep(0);

	PostMessage(DialogShown);

	return true;
}

void CLoadingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, loadingProgressBar);
	DDX_Control(pDX, IDC_EDIT1, resultEdit);
	DDX_Control(pDX, IDOK, okButton);
}

void CLoadingDialog::ExecuteFuncs()
{
	PostMessage(UpdateResult, static_cast<WPARAM>(LoadingResult::Loading), 0);
	success = false;
	
	for (size_t i = 0; i < loadingFuncs.size(); ++i)
	{
		if (!loadingFuncs[i]())
		{
			PostMessage(UpdateResult, static_cast<WPARAM>(LoadingResult::Failure), 0);
			return;
		}
		
		PostMessage(IncrementBar, static_cast<WPARAM>(i + 1), 0);
	}

	success = true;
	PostMessage(UpdateResult, static_cast<WPARAM>(LoadingResult::Success), 0);
}

bool CLoadingDialog::IsSuccess()
{
	return success;
}

LRESULT CLoadingDialog::OnDialogShown(WPARAM wParam, LPARAM lParam)
{
	std::thread([this](){ ExecuteFuncs(); }).detach();

	return 0;
}

LRESULT CLoadingDialog::OnUpdateResult(WPARAM wParam, LPARAM lParam)
{
	static std::vector<AdString> resTexts { L"Loading", L"Success", L"Failure" };
	
	LoadingResult res = static_cast<LoadingResult>(wParam);
	resultEdit.SetWindowTextW(resTexts[static_cast<int>(res)]);

	okButton.EnableWindow(res != LoadingResult::Loading);

	return 0;
}

LRESULT CLoadingDialog::OnIncrementBar(WPARAM wParam, LPARAM lParam)
{
	loadingProgressBar.SetStep(static_cast<int>(wParam));
	loadingProgressBar.StepIt();

	return 0;
}

BEGIN_MESSAGE_MAP(CLoadingDialog, CDialogEx)
	ON_MESSAGE(DialogShown, OnDialogShown)
	ON_MESSAGE(UpdateResult, OnUpdateResult)
	ON_MESSAGE(IncrementBar, OnIncrementBar)
END_MESSAGE_MAP()


// CLoadingDialog message handlers
