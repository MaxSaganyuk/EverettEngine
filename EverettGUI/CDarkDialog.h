#pragma once

#include "afxdialogex.h"

class CDarkDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDarkDialog)

	constexpr static COLORREF backgroundColor = RGB(0, 0, 0);
	constexpr static COLORREF textColor = RGB(240, 240, 240);

	CBrush backBrush;
protected:
	DECLARE_MESSAGE_MAP()

	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	CDarkDialog(unsigned int dlgID, CWnd* pParent = nullptr);
};