#pragma once
#include "afxdialogex.h"
#include "EverettEngine.h"

namespace Gdiplus
{
	class Bitmap;
}

// CObjectMoveDialog dialog

class CObjectMoveDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CObjectMoveDialog)

public:
	enum class TransformationType
	{
		Position,
		Scale,
		Rotation
	};

	CObjectMoveDialog(
		EverettEngine& engineRef, 
		IObjectSim& object,
		bool isSolid, 
		TransformationType moveType, 
		CWnd* pParent = nullptr
	);
	virtual ~CObjectMoveDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG8 };
#endif

protected:
	BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
private:

	enum DirectionType
	{
		Back = -1,
		None = 0,
		Forward = 1
	};

	void TransformObject(DirectionType x, DirectionType y, DirectionType z);

	CEdit rateOfChangeEdit;

	EverettEngine& engineRef;
	IObjectSim& object;
	bool isSolid;
	TransformationType transType;
	
	Gdiplus::Bitmap* coordBitmap;


	afx_msg void OnXPlusButtonClick();
	afx_msg void OnXMinusButtonClick();
	afx_msg void OnYPlusButtonClick();
	afx_msg void OnYMinusButtonClick();
	afx_msg void OnZPlusButtonClick();
	afx_msg void OnZMinusButtonClick();
};
