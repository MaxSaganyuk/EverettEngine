#pragma once

#include "afxdialogex.h"

#include <stack>
#include <limits>

class CTrackedDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTrackedDlg)

	static inline std::stack<CTrackedDlg*> activeModalStack;

	bool doesModal{};

	LRESULT OnAbortModal(WPARAM wParam, LPARAM lParam);
protected:
	DECLARE_MESSAGE_MAP()
public:
	enum class CloseModalCode
	{
		OK = IDOK,
		Cancel = IDCANCEL,
		Abort = IDABORT
	};

	constexpr static size_t AbortModal = WM_USER + 7;
	constexpr static size_t MaxDepth = (std::numeric_limits<size_t>::max)(); // fucking WinAPI max macro

	CTrackedDlg(unsigned int dlgID, CWnd* parentWnd);

	INT_PTR DoModal() override;

	static bool CloseModals(CloseModalCode code, bool wait = false, size_t depth = MaxDepth);
};