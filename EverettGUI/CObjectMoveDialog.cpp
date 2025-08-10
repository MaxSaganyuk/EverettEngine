// CObjectMoveDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CObjectMoveDialog.h"

#include <gdiplus.h>


// CObjectMoveDialog dialog

IMPLEMENT_DYNAMIC(CObjectMoveDialog, CDialogEx)

CObjectMoveDialog::CObjectMoveDialog(
	EverettEngine& engineRef, 
	IObjectSim& object, 
	bool isSolid,
	TransformationType transType, 
	CWnd* pParent
)
	: 
	engineRef(engineRef),
	object(object),
	isSolid(isSolid),
	transType(transType),
	CDialogEx(IDD_DIALOG8, pParent)
{

}

CObjectMoveDialog::~CObjectMoveDialog()
{
	if (coordBitmap)
	{
		delete coordBitmap;
	}
}

void CObjectMoveDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, rateOfChangeEdit);
}

BOOL CObjectMoveDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString value;
	value.Format(_T("%.4f"), 0.1f);

	rateOfChangeEdit.SetWindowTextW(value);

	return true;
}

void CObjectMoveDialog::OnPaint()
{
	if (!coordBitmap)
	{
		coordBitmap = MFCUtilities::LoadPNGFromResource(AfxGetResourceHandle(), IDB_PNG1, _T("PNG"));
	}

	if (coordBitmap)
	{
		CPaintDC dc(this);
		Gdiplus::Graphics graphics(dc.GetSafeHdc());

		graphics.DrawImage(coordBitmap, 75, 10, 200, 200);
	}
}

BOOL CObjectMoveDialog::OnEraseBkgnd(CDC* pDC)
{
	return true;
}


BEGIN_MESSAGE_MAP(CObjectMoveDialog, CDialogEx)
ON_WM_PAINT()
ON_BN_CLICKED(IDC_BUTTON2, &CObjectMoveDialog::OnXPlusButtonClick)
ON_BN_CLICKED(IDC_BUTTON3, &CObjectMoveDialog::OnXMinusButtonClick)
ON_BN_CLICKED(IDC_BUTTON4, &CObjectMoveDialog::OnYPlusButtonClick)
ON_BN_CLICKED(IDC_BUTTON11, &CObjectMoveDialog::OnYMinusButtonClick)
ON_BN_CLICKED(IDC_BUTTON12, &CObjectMoveDialog::OnZPlusButtonClick)
ON_BN_CLICKED(IDC_BUTTON13, &CObjectMoveDialog::OnZMinusButtonClick)
END_MESSAGE_MAP()


// CObjectMoveDialog message handlers

void CObjectMoveDialog::TransformObject(DirectionType x, DirectionType y, DirectionType z)
{
	glm::vec3* vectorPtr = nullptr;
	bool rotation = false;

	switch (transType)
	{
	case TransformationType::Position:
		vectorPtr = &object.GetPositionVectorAddr();
		break;
	case TransformationType::Scale:
		vectorPtr = &object.GetScaleVectorAddr();
		break;
	case TransformationType::Rotation:
		rotation = true;
		break;
	default:
		throw;
	}

	CString value;
	rateOfChangeEdit.GetWindowTextW(value);
	float rateOfChange = static_cast<float>(_tstof(value));
	
	float xChange = static_cast<float>(x) * rateOfChange;
	float yChange = static_cast<float>(y) * rateOfChange;
	float zChange = static_cast<float>(z) * rateOfChange;

	if (rateOfChange != 0.0f)
	{
		if (vectorPtr)
		{
			vectorPtr->x += xChange;
			vectorPtr->y += yChange;
			vectorPtr->z += zChange;
		}
		else if (rotation)
		{
			if (isSolid)
			{
				dynamic_cast<ISolidSim&>(object).Rotate({ xChange, yChange, zChange });
			}
			else
			{
				object.Rotate({ xChange, yChange, zChange });
			}
		}

		if (isSolid)
		{
			dynamic_cast<ISolidSim&>(object).ForceModelUpdate();
		}
	}
}

void CObjectMoveDialog::OnXPlusButtonClick()
{
	TransformObject(Forward, None, None);
}

void CObjectMoveDialog::OnXMinusButtonClick()
{
	TransformObject(Back, None, None);
}

void CObjectMoveDialog::OnYPlusButtonClick()
{
	TransformObject(None, Forward, None);
}

void CObjectMoveDialog::OnYMinusButtonClick()
{
	TransformObject(None, Back, None);
}

void CObjectMoveDialog::OnZPlusButtonClick()
{
	TransformObject(None, None, Forward);
}

void CObjectMoveDialog::OnZMinusButtonClick()
{
	TransformObject(None, None, Back);
}
