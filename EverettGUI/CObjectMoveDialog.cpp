// CObjectMoveDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CObjectMoveDialog.h"

#include <gdiplus.h>

#include <iostream>

// CObjectMoveDialog dialog

IMPLEMENT_DYNAMIC(CObjectMoveDialog, CDialogEx)

CObjectMoveDialog::CObjectMoveDialog(
	EverettEngine& engineRef, 
	IObjectSim& object, 
	bool isSolid,
	ObjectTransformType transType, 
	CWnd* pParent
)
	: 
	engineRef(engineRef),
	object(object),
	isSolid(isSolid),
	transType(transType),
	WASDBasedTransOn(false),
	CDialogEx(IDD_DIALOG8, pParent)
{

}

CObjectMoveDialog::~CObjectMoveDialog()
{
	if (coordBitmap)
	{
		delete coordBitmap;
	}

	if (WASDBasedTransOn)
	{
		ToggleWASDBasedControls();
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
ON_BN_CLICKED(IDC_BUTTON1, &CObjectMoveDialog::OnWASDBasedButtonClick)
END_MESSAGE_MAP()


// CObjectMoveDialog message handlers

void CObjectMoveDialog::TransformObject(const std::array<DirectionType, 3>& directionValues)
{
	glm::vec3* vectorPtr = nullptr;
	bool rotation = false;

	switch (transType)
	{
	case ObjectTransformType::Position:
		vectorPtr = &object.GetPositionVectorAddr();
		break;
	case ObjectTransformType::Scale:
		vectorPtr = &object.GetScaleVectorAddr();
		break;
	case ObjectTransformType::Rotation:
		rotation = true;
		break;
	default:
		throw;
	}

	CString value;
	rateOfChangeEdit.GetWindowTextW(value);
	float rateOfChange = static_cast<float>(_tstof(value));

	float x = static_cast<float>(directionValues[0]);
	float y = static_cast<float>(directionValues[1]);
	float z = static_cast<float>(directionValues[2]);
	
	float xChange = x * rateOfChange;
	float yChange = y * rateOfChange;
	float zChange = z * rateOfChange;

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
	TransformObject(directions[XPlus]);
}

void CObjectMoveDialog::OnXMinusButtonClick()
{
	TransformObject(directions[XMinus]);
}

void CObjectMoveDialog::OnYPlusButtonClick()
{
	TransformObject(directions[YPlus]);
}

void CObjectMoveDialog::OnYMinusButtonClick()
{
	TransformObject(directions[YMinus]);
}

void CObjectMoveDialog::OnZPlusButtonClick()
{
	TransformObject(directions[ZPlus]);
}

void CObjectMoveDialog::OnZMinusButtonClick()
{
	TransformObject(directions[ZMinus]);
}

void CObjectMoveDialog::ToggleWASDBasedControls()
{
	std::string keys = "ADWSRF";

	WASDBasedTransOn = !WASDBasedTransOn;

	if (WASDBasedTransOn)
	{
		engineRef.SetDefaultWASDControls(!WASDBasedTransOn);
	}

	for (int i = 0; i < keys.size(); ++i)
	{
		engineRef.SetInteractable(
			keys[i],
			false,
			WASDBasedTransOn ?
			[this, i]() { TransformObject(directions[static_cast<DirectionXYZ>(i)]); } : std::function<void()>(nullptr)
		);
	}

	if (!WASDBasedTransOn)
	{
		engineRef.SetDefaultWASDControls(!WASDBasedTransOn);
	}

	std::cout << "Turned " << (WASDBasedTransOn ? "On" : "Off") << " WASD based transformations\n";
}

void CObjectMoveDialog::OnWASDBasedButtonClick()
{
	ToggleWASDBasedControls();
}
