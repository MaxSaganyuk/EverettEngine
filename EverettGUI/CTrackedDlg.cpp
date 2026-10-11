#include "pch.h"
#include "CTrackedDlg.h"

IMPLEMENT_DYNAMIC(CTrackedDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CTrackedDlg, CDialogEx)
	ON_MESSAGE(CTrackedDlg::AbortModal, &CTrackedDlg::OnAbortModal)
END_MESSAGE_MAP()

CTrackedDlg::CTrackedDlg(unsigned int dlgID, CWnd* parentWnd)
	: CDialogEx(dlgID, parentWnd)
{
}

INT_PTR CTrackedDlg::DoModal()
{
	activeModalStack.push(this);
	doesModal = true;

	auto doModalRes = CDialogEx::DoModal();

	if (doesModal)
	{
		doesModal = false;
		activeModalStack.pop();
	}

	return doModalRes;
}

bool CTrackedDlg::CloseModals(CloseModalCode code, bool wait, size_t depth)
{
	WPARAM castedCode = static_cast<WPARAM>(code);

	while (!activeModalStack.empty() && depth)
	{
		CTrackedDlg* currentDlg = activeModalStack.top();

		currentDlg->doesModal = false;
		
		wait ? currentDlg->SendMessageW(AbortModal, castedCode) : currentDlg->PostMessageW(AbortModal, castedCode);

		activeModalStack.pop();
		--depth;
	}

	return true;
}

LRESULT CTrackedDlg::OnAbortModal(WPARAM wParam, LPARAM lParam)
{
	EndDialog(static_cast<int>(wParam));

	return 0;
}