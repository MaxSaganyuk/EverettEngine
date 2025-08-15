#pragma once
#include "afxdialogex.h"
#include "EverettEngine.h"

#include <array>
#include <map>

namespace Gdiplus
{
	class Bitmap;
}

// CObjectMoveDialog dialog

class CObjectMoveDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CObjectMoveDialog)

public:
	enum class ObjectTransformType
	{
		Position,
		Scale,
		Rotation
	};

	CObjectMoveDialog(
		EverettEngine& engineRef, 
		IObjectSim& object,
		bool isSolid, 
		ObjectTransformType moveType,
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

	void TransformObject(const std::array<DirectionType, 3>& directionValues);

	CEdit rateOfChangeEdit;

	EverettEngine& engineRef;
	IObjectSim& object;
	bool isSolid;
	ObjectTransformType transType;

	bool WASDBasedTransOn;

	enum DirectionXYZ
	{
		XPlus,
		XMinus,
		YPlus,
		YMinus,
		ZPlus,
		ZMinus
	};

	std::map<DirectionXYZ, std::array<DirectionType, 3>> directions
	{
		{ XPlus,  { Forward, None,    None    } },
		{ XMinus, { Back   , None,    None    } },
		{ YPlus,  { None,    Forward, None    } },
		{ YMinus, { None,    Back,    None    } },
		{ ZPlus,  { None,    None,    Forward } },
		{ ZMinus, { None,    None,    Back    } }
	};
	
	Gdiplus::Bitmap* coordBitmap;

	void ToggleWASDBasedControls();

	afx_msg void OnXPlusButtonClick();
	afx_msg void OnXMinusButtonClick();
	afx_msg void OnYPlusButtonClick();
	afx_msg void OnYMinusButtonClick();
	afx_msg void OnZPlusButtonClick();
	afx_msg void OnZMinusButtonClick();

	afx_msg void OnWASDBasedButtonClick();
};
