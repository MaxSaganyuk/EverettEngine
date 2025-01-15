#include "pch.h"
#include "resource.h"
#include "CChoiceDialog.h"
#include "CBrowseDialog.h"

#include <cassert>
#include <unordered_map>
#include <typeindex>

CChoiceDialog::CChoiceDialog()
	: CDialogEx(IDD_DIALOG4, nullptr)
{
	currentCObjectPos = CRect(20, 40, 200, 60);
	usedFont.CreatePointFont(100, _T("MS Shell Dlg"));
}

BEGIN_MESSAGE_MAP(CChoiceDialog, CDialogEx)
	ON_EN_CHANGE(EditChangeID, OnEditChanged)
	ON_BN_CLICKED(FileOpenID, OnOpenBrowseFileDialog)
	ON_BN_CLICKED(FolderOpenID, OnOpenBrowseFolderDialog)
	ON_CBN_SELCHANGE(SelectionChangeID, OnSelectionChange)
END_MESSAGE_MAP()

BOOL CChoiceDialog::OnInitDialog()
{
	auto CreateEdit = [this]()
	{
		CreateTitle(editInfos.front());

		CEdit* currentEdit = CreateCObject<CEdit>(editInfos.front());
		currentEdit->SetWindowTextW(CA2T(editInfos.front().text.c_str()));

		editInfos.pop();
	};

	auto CreateComboBox = [this]()
	{
		CreateTitle(comboBoxInfos.front());

		CComboBox* currentComboBox = CreateCObject<CComboBox>(comboBoxInfos.front());
		for (auto& text : comboBoxInfos.front().textCollection)
		{
			currentComboBox->AddString(CA2T(text.c_str()));
		}

		comboBoxInfos.pop();
	};

	auto CreateBrowseEdit = [this]()
	{
		CreateTitle(browseEditInfos.front());

		CEdit* currentEdit = CreateCObject<CEdit>(browseEditInfos.front());
		currentEdit->SetWindowTextW(CA2T(browseEditInfos.front().text.c_str()));

		CRect buttonPos = currentCObjectPos;
		buttonPos.left += 190;
		buttonPos.right += 120;

		buttonMap.emplace(browseEditInfos.front().name, std::make_unique<CButton>());

		buttonMap[browseEditInfos.front().name]->Create(
			_T("Browse"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			buttonPos,
			this,
			static_cast<unsigned int>(browseEditInfos.front().browseEditType)
		);
		buttonMap[browseEditInfos.front().name]->SetFont(&usedFont);

		browseEditInfos.pop();
	};

	CDialogEx::OnInitDialog();

	while (!objectTypeQueue.empty())
	{
		switch (objectTypeQueue.front())
		{
		case CObjectTypes::Edit:
			CreateEdit();
			break;
		case CObjectTypes::ComboBox:
			CreateComboBox();
			break;
		case CObjectTypes::BrowseEdit:
			CreateBrowseEdit();
			break;
		default:
			assert(false && "Unreachable");
		}

		objectTypeQueue.pop();

		currentCObjectPos.top += 50;
		currentCObjectPos.bottom += 50;
	}

	return true;
}

void CChoiceDialog::CreateTitle(CommonInfo& commonInfo)
{
	CRect labelPos = currentCObjectPos;
	labelPos += CRect(0, 20, 0, 20);

	titleCollection.push_back(std::make_unique<CStatic>());
	titleCollection.back()->Create(
		CA2T(commonInfo.title.c_str()),
		WS_CHILD | WS_VISIBLE,
		labelPos,
		this
	);
	titleCollection.back()->SetFont(&usedFont);
}

template<typename CObject>
CObject* CChoiceDialog::CreateCObject(CommonInfo& commonInfo)
{
	static std::unordered_map<std::type_index, std::pair<long, int>> typeToParamMap  
	{
		{ typeid(CEdit),     { WS_CHILD | WS_VISIBLE | WS_BORDER,        EmptyID}},
		{ typeid(CComboBox), { WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, SelectionChangeID }}
	};
	
	cObjByNameMap.emplace(commonInfo.name, ObjInfo{ "", std::make_unique<CObject>(), commonInfo.onChangeFunc });

	CObject* currentCObject = dynamic_cast<CObject*>(cObjByNameMap[commonInfo.name].cObj.get());
	currentCObject->Create(
		typeToParamMap[typeid(CObject)].first,
		currentCObjectPos,
		this,
		typeToParamMap[typeid(CObject)].second
	);
	currentCObject->SetFont(&usedFont);

	return currentCObject;
}

void CChoiceDialog::AddEdit(
	const std::string& name, 
	const std::string& title, 
	const std::string& text, 
	std::function<void()> onChangeFunc
)
{
	editInfos.push({ name, title, text, onChangeFunc });
	objectTypeQueue.push(CObjectTypes::Edit);
}

void CChoiceDialog::AddComboBox(
	const std::string& name, 
	const std::string& title, 
	const std::vector<std::string>& textCollection,
	std::function<void()> onChangeFunc
)
{
	comboBoxInfos.push({name, title, textCollection, onChangeFunc});
	objectTypeQueue.push(CObjectTypes::ComboBox);
}

void CChoiceDialog::AddBrowseEdit(
	const std::string& name, 
	const std::string& title, 
	const std::string& text, 
	const BrowseEditInfo::BrowseEditType browseEditType,
	std::function<void()> onChangeFunc
)
{
	browseEditInfos.push({name, title, text, browseEditType, onChangeFunc});
	objectTypeQueue.push(CObjectTypes::BrowseEdit);
}

std::string CChoiceDialog::GetTextByName(const std::string& name)
{
	return cObjByNameMap[name].text;
}

void CChoiceDialog::SetTextByName(const std::string& name, const std::string& text)
{
	cObjByNameMap[name].SetText(text);
}

void CChoiceDialog::OnEditChanged()
{
	ObjInfo* currentObj = nullptr;

	for (auto& cObjs : cObjByNameMap)
	{
		if (cObjs.second.cObj.get() == GetFocus())
		{
			currentObj = &cObjs.second;
			break;
		}
	}
}

void CChoiceDialog::OpenBrowseDialog(BrowseEditInfo::BrowseEditType browseType)
{
	CString text;

	if (browseType == BrowseEditInfo::BrowseEditType::File ? 
		CBrowseDialog::OpenAndGetFilePath(text) : CBrowseDialog::OpenAndGetFolderPath(text))
	{
		ObjInfo& currentObj = cObjByNameMap[FindClickedButtonName()];

		currentObj.SetText(std::string(CT2A(text)));

		if (currentObj.onChangeFunc)
		{
			currentObj.onChangeFunc();
		}
	}
}

void CChoiceDialog::OnOpenBrowseFileDialog()
{
	OpenBrowseDialog(BrowseEditInfo::BrowseEditType::File);
}

void CChoiceDialog::OnOpenBrowseFolderDialog()
{
	OpenBrowseDialog(BrowseEditInfo::BrowseEditType::Folder);
}

std::string CChoiceDialog::FindClickedButtonName()
{
	for (auto& buttonInfo : buttonMap)
	{
		if (buttonInfo.second.get() == GetFocus())
		{
			return buttonInfo.first;
		}
	}
}

void CChoiceDialog::OnSelectionChange()
{
	ObjInfo* currentObj = nullptr;

	for (auto& cObjs : cObjByNameMap)
	{
		if (cObjs.second.cObj.get() == GetFocus())
		{
			currentObj = &cObjs.second;
			break;
		}
	}

	if (currentObj)
	{
		CComboBox* currentComboBox = dynamic_cast<CComboBox*>(currentObj->cObj.get());

		if (currentComboBox)
		{
			CString text;
			currentComboBox->GetLBText(currentComboBox->GetCurSel(), text);
			currentObj->text = CT2A(text);

			if (currentObj->onChangeFunc)
			{
				currentObj->onChangeFunc();
			}
		}
	}
}