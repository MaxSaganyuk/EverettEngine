#include "pch.h"
#include "TreeManager.h"

TreeManager::TreeManagerNode::TreeManagerNode(
	const std::string& title,
	const std::string& data,
	CTreeCtrl& objectTree,
	HTREEITEM nodeInfo,
	TreeManagerNode* previousNode
)
	: title(title), data(data), objectTree(objectTree), nodeInfo(nodeInfo), previousNode(previousNode) {}

void TreeManager::TreeManagerNode::AddNode(const std::string& title, const std::string& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(CA2T((title + ": " + data).c_str()), nodeInfo);
	nextNodes.emplace(
		data,
		std::make_unique<TreeManagerNode>(
			title,
			data,
			objectTree,
			newNodeInfo,
			this
		)
	);

	objectTree.RedrawWindow();
}

TreeManager::TreeManagerNode* TreeManager::TreeManagerNode::FindNodeBy(HTREEITEM item)
{
	if (item == nodeInfo)
	{
		return this;
	}

	for (auto& node : nextNodes)
	{
		TreeManagerNode* currentNode = node.second->FindNodeBy(item);
		if (currentNode)
		{
			return currentNode;
		}
	}

	return nullptr;
}

CTreeCtrl& TreeManager::GetTreeCtrl()
{
	return objectTree;
}

void TreeManager::AddRootNode(const std::string& title, const std::string& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(CA2T((title + ": " + data).c_str()));
	rootNodes.emplace(
		data,
		std::make_unique<TreeManagerNode>(title, data, GetTreeCtrl(), newNodeInfo, nullptr)
	);
}

void TreeManager::SetObjectTypes(
	const std::vector<std::string>& objectTypes, 
	const std::vector<std::string>& lightTypes
)
{
	this->objectTypes = objectTypes;

	for (auto& type : objectTypes)
	{
		AddRootNode("Object", type);
	}

	for (auto& lightType : lightTypes)
	{
		rootNodes["Light"]->AddNode("LightType", lightType);
	}
}

void TreeManager::AddModelToTree(const std::string& modelName)
{
	rootNodes["Solid"]->AddNode("Model", modelName);
}

void TreeManager::AddSolidToTree(const std::string& modelName, const std::string& solidName)
{
	rootNodes["Solid"]->nextNodes[modelName]->AddNode("Solid", solidName);
}
void TreeManager::AddLightToTree(const std::string& lightType, const std::string& lightName)
{
	rootNodes["Light"]->nextNodes[lightType]->AddNode("Light", lightName);
}

void TreeManager::AddSoundToTree(const std::string& soundName)
{
	rootNodes["Sound"]->AddNode("Sound", soundName);
}

TreeManager::TreeManagerNode* TreeManager::FindNodeByItem(HTREEITEM item)
{
	for (auto& node : rootNodes)
	{
		TreeManagerNode* currentNode = node.second->FindNodeBy(item);
		if (currentNode)
		{
			return currentNode;
		}
	}

	return nullptr;
}

std::map<std::string, std::string> TreeManager::GetAllOfRootsSelectedNode()
{
	std::map<std::string, std::string> allDataRoots;
 	
	TreeManagerNode* currentNode = FindNodeByItem(objectTree.GetSelectedItem());
	
	while (currentNode->previousNode)
	{
		allDataRoots.emplace(currentNode->title, currentNode->data);
		currentNode = currentNode->previousNode;
	}

	return allDataRoots;
}