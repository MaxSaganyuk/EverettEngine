#pragma once

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>

/*
	This version of a TreeManager specific to handling MFC tree object
	Might have way too many extra MFC stuff to just overload basic
	TreeManager template. Will look into this later.
*/
class MFCTreeManager
{
	class MFCTreeManagerNode
	{
		friend MFCTreeManager;

	public:
		MFCTreeManagerNode(
			const std::string& title,
			const std::string& data,
			CTreeCtrl& objectTree,
			HTREEITEM nodeInfo,
			MFCTreeManagerNode* previousNode
		);

		void AddNode(const std::string& title, const std::string& data);
		MFCTreeManagerNode* FindNodeBy(HTREEITEM item);

	private:
		std::string title;
		std::string data;
		CTreeCtrl& objectTree;
		HTREEITEM nodeInfo;
		MFCTreeManagerNode* previousNode;
		std::map<std::string, std::unique_ptr<MFCTreeManagerNode>> nextNodes;
	};

public:
	void AddModelToTree(const std::string& modelName);
	void AddSolidToTree(const std::string& modelName, const std::string& solidName);
	void AddLightToTree(const std::string& lightType, const std::string& lightName);
	void AddSoundToTree(const std::string& soundName);

	void SetObjectTypes(const std::vector<std::string>& objectTypes, const std::vector<std::string>& lightTypes);

	void AddRootNode(const std::string& leftString, const std::string& rightString);

	std::vector<std::pair<std::string, std::string>> GetAllOfRootsSelectedNode();

	CTreeCtrl& GetTreeCtrl();
private:

	MFCTreeManagerNode* FindNodeByItem(HTREEITEM item);

	std::vector<std::string> objectTypes;

	std::map<std::string, std::unique_ptr<MFCTreeManagerNode>> rootNodes;
	
	CTreeCtrl objectTree;
};