#include "pch.h"
#include "CDarkDialog.h"

IMPLEMENT_DYNAMIC(CDarkDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CDarkDialog, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CDarkDialog::CDarkDialog(unsigned int dlgID, CWnd* pParent)
	: CDialogEx(dlgID, pParent)
{
	backBrush.CreateSolidBrush(backgroundColor);
}

HBRUSH CDarkDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(backgroundColor);
		pDC->SetTextColor(textColor);

		return static_cast<HBRUSH>(backBrush.GetSafeHandle());
	}

	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}
