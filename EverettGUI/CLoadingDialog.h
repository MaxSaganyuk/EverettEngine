#pragma once
#include "afxdialogex.h"
#include <functional>
#include <vector>
#include <string>

// CLoadingDialog dialog

class CLoadingDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadingDialog)

public:
	CLoadingDialog(
		const std::vector<std::function<bool()>>& loadingFuncs, 
		CWnd* pParent = nullptr
	);   // standard constructor
	virtual ~CLoadingDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif
	bool IsSuccess();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog() override;

	afx_msg LRESULT OnDialogShown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIncrementBar(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	void ExecuteFuncs();

	enum class LoadingResult
	{
		Loading,
		Success,
		Failure
	};

	bool success;
	std::vector<std::function<bool()>> loadingFuncs;
	CProgressCtrl loadingProgressBar;
	CEdit resultEdit;
	CButton okButton;

	constexpr static int DialogShown  = WM_USER + 1;
	constexpr static int UpdateResult = WM_USER + 2;
	constexpr static int IncrementBar = WM_USER + 3;
};
