#pragma once

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>

class TreeManager
{
	class TreeManagerNode
	{
		friend TreeManager;

	public:
		TreeManagerNode(
			const std::string& title,
			const std::string& data,
			CTreeCtrl& objectTree,
			HTREEITEM nodeInfo,
			TreeManagerNode* previousNode
		);

		void AddNode(const std::string& title, const std::string& data);
		TreeManagerNode* FindNodeBy(HTREEITEM item);

	private:
		std::string title;
		std::string data;
		CTreeCtrl& objectTree;
		HTREEITEM nodeInfo;
		TreeManagerNode* previousNode;
		std::map<std::string, std::unique_ptr<TreeManagerNode>> nextNodes;
	};

public:
	void AddModelToTree(const std::string& modelName);
	void AddSolidToTree(const std::string& modelName, const std::string& solidName);
	void AddLightToTree(const std::string& lightType, const std::string& lightName);
	void AddSoundToTree(const std::string& soundName);

	void SetObjectTypes(const std::vector<std::string>& objectTypes, const std::vector<std::string>& lightTypes);

	void AddRootNode(const std::string& leftString, const std::string& rightString);

	std::map<std::string, std::string> GetAllOfRootsSelectedNode();

	CTreeCtrl& GetTreeCtrl();
private:

	TreeManagerNode* FindNodeByItem(HTREEITEM item);

	std::vector<std::string> objectTypes;

	std::map<std::string, std::unique_ptr<TreeManagerNode>> rootNodes;
	
	CTreeCtrl objectTree;
};