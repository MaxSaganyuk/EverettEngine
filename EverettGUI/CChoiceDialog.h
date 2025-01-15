#pragma once

#include "afxdialogex.h"

#include <vector>
#include <string>
#include <memory>
#include <typeinfo>
#include <queue>
#include <map>
#include <functional>

class CChoiceDialog : public CDialogEx
{
	DECLARE_MESSAGE_MAP()

private:
	constexpr static int EmptyID = 1000;
	constexpr static int EditChangeID = 1001;
	constexpr static int FileOpenID = 1002;
	constexpr static int FolderOpenID = 1003;
	constexpr static int SelectionChangeID = 1004;

public:
	struct CommonInfo
	{
		CommonInfo(const std::string& name, const std::string& title, std::function<void()>& onChangeFunc)
			: name(name), title(title), onChangeFunc(onChangeFunc) {}

		std::string name;
		std::string title;
		std::function<void()> onChangeFunc;
	};

	struct EditInfo : public CommonInfo
	{
		EditInfo(
			const std::string& name, 
			const std::string& title, 
			const std::string& text, 
			std::function<void()>& onChangeFunc
		) :
			CommonInfo(name, title, onChangeFunc), text(text) {}

		std::string text;
	};

	struct ComboBoxInfo : public CommonInfo
	{
		ComboBoxInfo(
			const std::string& name, 
			const std::string& title, 
			const std::vector<std::string>& textCollection, 
			std::function<void()>& onChangeFunc
		)
			: CommonInfo(name, title, onChangeFunc), textCollection(textCollection) {}

		std::vector<std::string> textCollection;
	};

	struct BrowseEditInfo : public CommonInfo
	{
		enum class BrowseEditType
		{
			File = FileOpenID,
			Folder = FolderOpenID
		};

		BrowseEditInfo(
			const std::string& name,
			const std::string& title,
			const std::string& text,
			const BrowseEditType browseEditType,
			std::function<void()>& onChangeFunc
		) : CommonInfo(name, title, onChangeFunc), text(text), browseEditType(browseEditType) {}

		std::string text;
		BrowseEditType browseEditType;
	};

public:
	CChoiceDialog();

	BOOL OnInitDialog() override;

	void AddEdit(
		const std::string& name, 
		const std::string& title, 
		const std::string& text, 
		std::function<void()> onChangeFunc = nullptr
	);
	void AddComboBox(
		const std::string& name,
		const std::string& title, 
		const std::vector<std::string>& textCollection, 
		std::function<void()> onChangeFunc = nullptr
	);
	void AddBrowseEdit(
		const std::string& name,
		const std::string& title,
		const std::string& text,
		const BrowseEditInfo::BrowseEditType browseEditType,
		std::function<void()> onChangeFunc = nullptr
	);

	std::string GetTextByName(const std::string& name);
	void SetTextByName(const std::string& name, const std::string& text);
private:
	CRect currentCObjectPos;

	CFont usedFont;

	void CreateTitle(CommonInfo& commonInfo);

	template<typename CObject>
	CObject* CreateCObject(CommonInfo& commonInfo);
	
	void OnEditChanged();

	void OpenBrowseDialog(BrowseEditInfo::BrowseEditType browseType);
	void OnOpenBrowseFileDialog();
	void OnOpenBrowseFolderDialog();

	std::string FindClickedButtonName();

	void OnSelectionChange();

	enum class CObjectTypes
	{
		Edit,
		ComboBox,
		BrowseEdit
	};

	struct ObjInfo
	{
		std::string text;
		std::unique_ptr<CWnd> cObj;
		std::function<void()> onChangeFunc;

		void SetText(const std::string& text)
		{
			this->text = text;

			if (cObj->m_hWnd) // Prevents a crash if set is attempted after DoModal execution
			{
				cObj->SetWindowTextW(CA2T(text.c_str()));
			}
		}
	};

	std::queue<CObjectTypes> objectTypeQueue;

	std::map<std::string, ObjInfo> cObjByNameMap;

	std::vector<std::unique_ptr<CStatic>> titleCollection;
	std::map<std::string, std::unique_ptr<CButton>> buttonMap;

	std::queue<EditInfo> editInfos;
	std::queue<ComboBoxInfo> comboBoxInfos;
	std::queue<BrowseEditInfo> browseEditInfos;
};