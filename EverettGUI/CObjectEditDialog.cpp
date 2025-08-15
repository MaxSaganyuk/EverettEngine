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
	std::vector<std::pair<AdString, AdString>>& selectedScriptDllInfo,
	const std::vector<std::pair<AdString, AdString>>& selectedNodes,
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
	castedSolidInterface(nullptr),
	castedSoundInterface(nullptr)
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
	DDX_Control(pDX, IDC_ANIM_TEXT, playerText);
	DDX_Control(pDX, IDC_COMBO3, playerComboBox);
	DDX_Control(pDX, IDC_BUTTON5, playerPlayButton);
	DDX_Control(pDX, IDC_BUTTON6, playerPauseButton);
	DDX_Control(pDX, IDC_BUTTON7, playerStopButton);
	DDX_Control(pDX, IDC_CHECK4, playerLoopCheck);
	DDX_Control(pDX, IDC_SPEED_TEXT, playerSpeedText);
	DDX_Control(pDX, IDC_EDIT11, playerSpeedEdit);
}

CString CObjectEditDialog::GenerateTitle()
{
	CString titleRes(_T("Edit "));

	titleRes.Append(CString(EverettEngine::GetObjectTypeToName(objectType).c_str()));
	titleRes.Append(!subtypeName.empty() ? CString(L" : " + subtypeName) : CString());
	titleRes.Append(!objectName.empty() ? CString(L" : " + objectName) : CString());

	return titleRes;
}

void CObjectEditDialog::SetupModelParams()
{
	bool isSolid = objectType == EverettEngine::ObjectTypes::Solid;
	bool isSound = objectType == EverettEngine::ObjectTypes::Sound;

	meshComboBox.ShowWindow (isSolid);
	meshVisCheck.ShowWindow (isSolid);
	meshText.ShowWindow     (isSolid);
	modelPropText.ShowWindow(isSolid);

	playerText.ShowWindow       (isSolid           );
	playerComboBox.ShowWindow   (isSolid           );
	playerPlayButton.ShowWindow (isSolid || isSound);
	playerPauseButton.ShowWindow(isSolid || isSound);
	playerStopButton.ShowWindow (isSolid || isSound);
	playerLoopCheck.ShowWindow  (isSolid || isSound);
	playerSpeedText.ShowWindow  (isSolid || isSound);
	playerSpeedEdit.ShowWindow  (isSolid || isSound);

	playerPlayButton.EnableWindow(false);
	playerPauseButton.EnableWindow(false);
	playerStopButton.EnableWindow(false);
	playerLoopCheck.EnableWindow(false);
	playerSpeedEdit.EnableWindow(false);

	if (isSolid || isSound)
	{
		if (isSolid)
		{
			castedSolidInterface = dynamic_cast<ISolidSim*>(&currentObjectInterface);
		}
		else
		{
			castedSoundInterface = dynamic_cast<ISoundSim*>(&currentObjectInterface);
		}

		CString speedValue;
		speedValue.Format(_T("%.2f"), castedSolidInterface ? 
			castedSolidInterface->GetModelAnimationSpeed() : castedSoundInterface->GetPlaybackSpeed()
		);
		playerSpeedEdit.SetWindowTextW(speedValue);

		if (castedSolidInterface)
		{
			std::vector<std::string> meshNames = castedSolidInterface->GetModelMeshNames();
			for (auto& meshName : meshNames)
			{
				meshComboBox.AddString(AdString(meshName));
			}

			if (!meshNames.empty())
			{
				meshComboBox.SetCurSel(0);
			}

			meshVisCheck.SetCheck(
				castedSolidInterface->GetModelMeshVisibility(0)
			);

			std::vector<std::string> animNames = castedSolidInterface->GetModelAnimationNames();
			for (auto& animName : animNames)
			{
				playerComboBox.AddString(AdString(animName));
			}

			if (!animNames.empty())
			{
				playerComboBox.SetCurSel(static_cast<int>(castedSolidInterface->GetModelAnimation()));
				playerLoopCheck.EnableWindow(true);
				playerSpeedEdit.EnableWindow(true);
			}
		}

		SetPlayerButtons(
			castedSolidInterface ? castedSolidInterface->GetModelAnimationAmount() > 0 : true,
			castedSolidInterface ? castedSolidInterface->IsModelAnimationPaused() : castedSoundInterface->IsPaused(),
			castedSolidInterface ? castedSolidInterface->IsModelAnimationLooped() : castedSoundInterface->IsLooped()
		);
	}
}

void CObjectEditDialog::UpdateParams()
{
	SetObjectParams({
		currentObjectInterface.GetPositionVectorAddr(),
		currentObjectInterface.GetScaleVectorAddr(),
		currentObjectInterface.GetFrontVectorAddr()
	});
}

BOOL CObjectEditDialog::OnInitDialog()
{
	DLLLoaderCommon::OnInitDialog();

	SetWindowText(GenerateTitle());

	UpdateParams();
	SetupModelParams();

	return true;
}

BEGIN_MESSAGE_MAP(CObjectEditDialog, DLLLoaderCommon)
	ON_BN_CLICKED(IDC_BUTTON1, &CObjectEditDialog::OnUpdateParamsButtonClick)
	ON_BN_CLICKED(IDC_CHECK3, &CObjectEditDialog::OnBnClickedCheck3)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CObjectEditDialog::OnMeshCBSelChange)
	ON_BN_CLICKED(IDC_BUTTON5, &CObjectEditDialog::OnPlayPlayerButtonClick)
	ON_BN_CLICKED(IDC_BUTTON6, &CObjectEditDialog::OnPausePlayerButtonClick)
	ON_BN_CLICKED(IDC_BUTTON7, &CObjectEditDialog::OnStopPlayerButtonClick)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CObjectEditDialog::OnAnimCBSelChange)
	ON_BN_CLICKED(IDC_BUTTON8, &CObjectEditDialog::OnPosEditButtonClick)
	ON_BN_CLICKED(IDC_BUTTON9, &CObjectEditDialog::OnScaEditButtonClick)
	ON_BN_CLICKED(IDC_BUTTON10, &CObjectEditDialog::OnRotEditButtonClick)
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
			valuesToSet[i][j] = static_cast<float>(_tstof(value));
		}
	}

	currentObjectInterface.GetPositionVectorAddr() = valuesToSet[0];
	currentObjectInterface.GetScaleVectorAddr()    = valuesToSet[1];
	currentObjectInterface.GetFrontVectorAddr()    = valuesToSet[2];

	if (castedSolidInterface)
	{
		castedSolidInterface->ForceModelUpdate();
	}
}

void CObjectEditDialog::OnBnClickedCheck3()
{
	castedSolidInterface->SetModelMeshVisibility(
		meshComboBox.GetCurSel(), meshVisCheck.GetCheck()
	);
}

void CObjectEditDialog::OnMeshCBSelChange()
{
	meshVisCheck.SetCheck(
		castedSolidInterface->GetModelMeshVisibility(meshComboBox.GetCurSel())
	);
}

void CObjectEditDialog::SetPlayerButtons(bool play, bool pause, bool stop)
{
	playerPlayButton.EnableWindow(play);
	playerPauseButton.EnableWindow(pause);
	playerStopButton.EnableWindow(stop);
}

void CObjectEditDialog::OnPlayPlayerButtonClick()
{
	AdString animSpeed;
	playerSpeedEdit.GetWindowTextW(animSpeed);

	double playbackSpeed = _tstof(animSpeed);

	castedSolidInterface ? 
		castedSolidInterface->SetModelAnimationSpeed(_tstof(animSpeed)) : 
		castedSoundInterface->SetPlaybackSpeed(static_cast<float>(playbackSpeed));

	if (castedSolidInterface)
	{
		castedSolidInterface->SetModelAnimation(playerComboBox.GetCurSel());
	}

	castedSolidInterface ? 
		castedSolidInterface->PlayModelAnimation(playerLoopCheck.GetCheck()) : 
		castedSoundInterface->Play(playerLoopCheck.GetCheck());

	SetPlayerButtons(true, true, true);
}


void CObjectEditDialog::OnPausePlayerButtonClick()
{
	castedSolidInterface ? castedSolidInterface->PauseModelAnimation() : castedSoundInterface->Pause();
	SetPlayerButtons(true, false, true);
}


void CObjectEditDialog::OnStopPlayerButtonClick()
{
	castedSolidInterface ? castedSolidInterface->StopModelAnimation() : castedSoundInterface->Stop();
	SetPlayerButtons(true, false, false);
}


void CObjectEditDialog::OnAnimCBSelChange()
{
}

void CObjectEditDialog::StartObjectMoveDlg(CObjectMoveDialog::ObjectTransformType transType)
{
	CObjectMoveDialog moveDlg(
		engineRef,
		currentObjectInterface,
		castedSolidInterface != nullptr,
		transType
	);

	moveDlg.DoModal();
	UpdateParams();
}

void CObjectEditDialog::OnPosEditButtonClick()
{
	StartObjectMoveDlg(CObjectMoveDialog::ObjectTransformType::Position);
}

void CObjectEditDialog::OnScaEditButtonClick()
{
	StartObjectMoveDlg(CObjectMoveDialog::ObjectTransformType::Scale);
}

void CObjectEditDialog::OnRotEditButtonClick()
{
	StartObjectMoveDlg(CObjectMoveDialog::ObjectTransformType::Rotation);
}
