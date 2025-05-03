// CObjectEditDialog.cpp : implementation file
//

#include "pch.h"
#include "EverettGUI.h"
#include "afxdialogex.h"
#include "CObjectEditDialog.h"
#include "CBrowseDialog.h"


// CObjectEditDialog dialog

IMPLEMENT_DYNAMIC(CObjectEditDialog, DLLLoaderCommon)

CObjectEditDialog::CObjectEditDialog(
	EverettEngine& engine, 
	EverettEngine::ObjectTypes objectTypeP,
	std::vector<std::pair<std::string, std::string>>& selectedScriptDllInfo,
	const std::vector<std::pair<std::string, std::string>>& selectedNodes,
	CWnd* pParent 
)
	: 
	DLLLoaderCommon(
		IDD_DIALOG4, 
		selectedScriptDllInfo, 
		[this](const std::string& dllName) { 
			return engineRef.IsObjectScriptSet(objectType, subtypeName, objectName, dllName); 
		},
		[this](const std::string& dllName, const std::string& dllPath) { 
			engineRef.SetScriptToObject(objectType, subtypeName, objectName, dllName, dllPath); 
		},
		[this](const std::string& dllPath) { engineRef.UnsetScript(dllPath); },
		pParent
	),
	engineRef(engine), 
	objectType(objectTypeP),
	subtypeName(selectedNodes.size() > 1 ? selectedNodes[1].second : ""),
	objectName(selectedNodes.size() > 0 ? selectedNodes[0].second : ""),
	currentObjectInterface(*engineRef.GetObjectInterface(objectType, subtypeName, objectName)),
	castedCurrentObject(nullptr)
{
}

CObjectEditDialog::~CObjectEditDialog()
{
}

void CObjectEditDialog::DoDataExchange(CDataExchange* pDX)
{
	DLLLoaderCommon::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, objectInfoEdits[0][0]);
	DDX_Control(pDX, IDC_EDIT2, objectInfoEdits[0][1]);
	DDX_Control(pDX, IDC_EDIT3, objectInfoEdits[0][2]);
	DDX_Control(pDX, IDC_EDIT4, objectInfoEdits[1][0]);
	DDX_Control(pDX, IDC_EDIT5, objectInfoEdits[1][1]);
	DDX_Control(pDX, IDC_EDIT6, objectInfoEdits[1][2]);
	DDX_Control(pDX, IDC_EDIT7, objectInfoEdits[2][0]);
	DDX_Control(pDX, IDC_EDIT8, objectInfoEdits[2][1]);
	DDX_Control(pDX, IDC_EDIT9, objectInfoEdits[2][2]);

	DDX_Control(pDX, IDC_COMBO2, meshComboBox);
	DDX_Control(pDX, IDC_CHECK3, meshVisCheck);
	DDX_Control(pDX, IDC_MESH_TEXT, meshText);
	DDX_Control(pDX, IDC_MODEL_PROP_TEXT, modelPropText);
	DDX_Control(pDX, IDC_ANIM_TEXT, animText);
	DDX_Control(pDX, IDC_COMBO3, animComboBox);
	DDX_Control(pDX, IDC_BUTTON5, animPlayButton);
	DDX_Control(pDX, IDC_BUTTON6, animPauseButton);
	DDX_Control(pDX, IDC_BUTTON7, animStopButton);
	DDX_Control(pDX, IDC_CHECK4, animLoopCheck);
}

CString CObjectEditDialog::GenerateTitle()
{
	CString titleRes(_T("Edit "));

	titleRes.Append(CA2T(EverettEngine::GetObjectTypeToName(objectType).c_str()));
	titleRes.Append(CA2T((!subtypeName.empty() ? " : " + subtypeName : "").c_str()));
	titleRes.Append(CA2T((!objectName.empty() ? " : " + objectName : "").c_str()));

	return titleRes;
}

void CObjectEditDialog::SetupModelParams()
{
	bool modelParamsShow = objectType == EverettEngine::ObjectTypes::Solid;

	meshComboBox.ShowWindow(modelParamsShow);
	meshVisCheck.ShowWindow(modelParamsShow);
	meshText.ShowWindow(modelParamsShow);
	modelPropText.ShowWindow(modelParamsShow);

	animText.ShowWindow(modelParamsShow);
	animComboBox.ShowWindow(modelParamsShow);
	animPlayButton.ShowWindow(modelParamsShow);
	animPlayButton.EnableWindow(false);
	animPauseButton.ShowWindow(modelParamsShow);
	animPauseButton.EnableWindow(false);
	animStopButton.ShowWindow(modelParamsShow);
	animStopButton.EnableWindow(false);
	animLoopCheck.ShowWindow(modelParamsShow);

	if (modelParamsShow)
	{
		castedCurrentObject = dynamic_cast<ISolidSim*>(&currentObjectInterface);
		std::vector<std::string> meshNames = castedCurrentObject->GetModelMeshNames();
		for (auto& meshName : meshNames)
		{
			meshComboBox.AddString(CA2T(meshName.c_str()));
		}

		std::vector<std::string> animNames = castedCurrentObject->GetModelAnimationNames();
		for (auto& animName : animNames)
		{
			animComboBox.AddString(CA2T(animName.c_str()));
		}
	}
}

BOOL CObjectEditDialog::OnInitDialog()
{
	DLLLoaderCommon::OnInitDialog();

	SetWindowText(GenerateTitle());

	SetObjectParams({
		currentObjectInterface.GetPositionVectorAddr(),
		currentObjectInterface.GetScaleVectorAddr(),
		currentObjectInterface.GetFrontVectorAddr()
	});

	SetupModelParams();

	return true;
}

BEGIN_MESSAGE_MAP(CObjectEditDialog, DLLLoaderCommon)
	ON_BN_CLICKED(IDC_BUTTON1, &CObjectEditDialog::OnUpdateParamsButtonClick)
	ON_BN_CLICKED(IDC_CHECK3, &CObjectEditDialog::OnBnClickedCheck3)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CObjectEditDialog::OnMeshCBSelChange)
	ON_BN_CLICKED(IDC_BUTTON5, &CObjectEditDialog::OnPlayAnimButtonClick)
	ON_BN_CLICKED(IDC_BUTTON6, &CObjectEditDialog::OnPauseAnimButtonClick)
	ON_BN_CLICKED(IDC_BUTTON7, &CObjectEditDialog::OnStopAnimButtonClick)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CObjectEditDialog::OnAnimCBSelChange)
END_MESSAGE_MAP()


void CObjectEditDialog::SetObjectParams(const std::vector<glm::vec3>& params)
{
	for (int i = 0; i < objectInfoEdits.size(); ++i)
	{
		for (int j = 0; j < objectInfoEdits.front().size(); ++j)
		{
			CString value;
			value.Format(_T("%.4f"), params[i][j]);
			objectInfoEdits[i][j].SetWindowTextW(value);
			objectInfoEdits[i][j].EnableWindow(true);
			objectInfoEdits[i][j].RedrawWindow();
		}
	}
}


void CObjectEditDialog::OnUpdateParamsButtonClick()
{
	std::vector<glm::vec3> valuesToSet;

	for (int i = 0; i < objectInfoEdits.size(); ++i)
	{
		valuesToSet.push_back({});
		for (int j = 0; j < objectInfoEdits.front().size(); ++j)
		{
			CString value;
			objectInfoEdits[i][j].GetWindowTextW(value);
			valuesToSet[i][j] = _tstof(value);
		}
	}

	currentObjectInterface.GetPositionVectorAddr() = valuesToSet[0];
	currentObjectInterface.GetScaleVectorAddr()    = valuesToSet[1];
	currentObjectInterface.GetFrontVectorAddr()    = valuesToSet[2];

	if (objectType == EverettEngine::ObjectTypes::Solid)
	{
		castedCurrentObject->ForceModelUpdate();
	}
}

void CObjectEditDialog::OnBnClickedCheck3()
{
	castedCurrentObject->SetModelMeshVisibility(
		meshComboBox.GetCurSel(), meshVisCheck.GetCheck()
	);
}

void CObjectEditDialog::OnMeshCBSelChange()
{
	meshVisCheck.SetCheck(
		castedCurrentObject->GetModelMeshVisibility(meshComboBox.GetCurSel())
	);
}

void CObjectEditDialog::SetAnimButtons(bool play, bool pause, bool stop)
{
	animPlayButton.EnableWindow(play);
	animPauseButton.EnableWindow(pause);
	animStopButton.EnableWindow(stop);
}

void CObjectEditDialog::OnPlayAnimButtonClick()
{
	castedCurrentObject->SetModelAnimation(animComboBox.GetCurSel());
	castedCurrentObject->PlayModelAnimation(animLoopCheck.GetCheck());
	SetAnimButtons(true, true, true);
}


void CObjectEditDialog::OnPauseAnimButtonClick()
{
	castedCurrentObject->PauseModelAnimation();
	SetAnimButtons(true, false, true);
}


void CObjectEditDialog::OnStopAnimButtonClick()
{
	castedCurrentObject->StopModelAnimation();
	SetAnimButtons(true, false, false);
}


void CObjectEditDialog::OnAnimCBSelChange()
{
	if (!castedCurrentObject->IsModelAnimationPlaying())
	{
		SetAnimButtons(true, false, false);
	}
}
