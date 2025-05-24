#pragma once

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>

#include "AdString.h"

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
			const AdString& title,
			const AdString& data,
			CTreeCtrl& objectTree,
			HTREEITEM nodeInfo,
			MFCTreeManagerNode* previousNode
		);

		void AddNode(const AdString& title, const AdString& data);
		MFCTreeManagerNode* FindNodeBy(HTREEITEM item);
		void ClearAllNextNodes(bool includeItself = false);

	private:
		AdString title;
		AdString data;
		CTreeCtrl& objectTree;
		HTREEITEM nodeInfo;
		MFCTreeManagerNode* previousNode;
		std::map<AdString, std::unique_ptr<MFCTreeManagerNode>> nextNodes;
	};

public:
	void AddModelToTree(const AdString& modelName);
	void AddSolidToTree(const AdString& modelName, const AdString& solidName);
	void AddLightToTree(const AdString& lightType, const AdString& lightName);
	void AddSoundToTree(const AdString& soundName);

	void SetObjectTypes();

	void AddRootNode(const AdString& leftString, const AdString& rightString);

	std::vector<std::pair<AdString, AdString>> GetAllOfRootsSelectedNode();

	CTreeCtrl& GetTreeCtrl();

	void ClearNonRootNodes();

	void DeleteNodeByItem(HTREEITEM item, bool includeItself = false);

private:

	MFCTreeManagerNode* FindNodeByItem(HTREEITEM item);

	std::vector<std::string> objectTypes;

	std::map<AdString, std::unique_ptr<MFCTreeManagerNode>> rootNodes;
	
	CTreeCtrl objectTree;
};