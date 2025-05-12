#include "pch.h"
#include "MFCTreeManager.h"

MFCTreeManager::MFCTreeManagerNode::MFCTreeManagerNode(
	const std::string& title,
	const std::string& data,
	CTreeCtrl& objectTree,
	HTREEITEM nodeInfo,
	MFCTreeManagerNode* previousNode
)
	: title(title), data(data), objectTree(objectTree), nodeInfo(nodeInfo), previousNode(previousNode) {}

void MFCTreeManager::MFCTreeManagerNode::AddNode(const std::string& title, const std::string& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(CA2T((title + ": " + data).c_str()), nodeInfo);
	nextNodes.emplace(
		data,
		std::make_unique<MFCTreeManagerNode>(
			title,
			data,
			objectTree,
			newNodeInfo,
			this
		)
	);

	objectTree.RedrawWindow();
}

MFCTreeManager::MFCTreeManagerNode* MFCTreeManager::MFCTreeManagerNode::FindNodeBy(HTREEITEM item)
{
	if (item == nodeInfo)
	{
		return this;
	}

	for (auto& node : nextNodes)
	{
		MFCTreeManagerNode* currentNode = node.second->FindNodeBy(item);
		if (currentNode)
		{
			return currentNode;
		}
	}

	return nullptr;
}

void MFCTreeManager::MFCTreeManagerNode::ClearAllNextNodes()
{
	for (auto& node : nextNodes)
	{
		objectTree.DeleteItem(node.second->nodeInfo);
	}
	nextNodes.clear();
}

CTreeCtrl& MFCTreeManager::GetTreeCtrl()
{
	return objectTree;
}

void MFCTreeManager::AddRootNode(const std::string& title, const std::string& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(CA2T((title + ": " + data).c_str()));
	rootNodes.emplace(
		data,
		std::make_unique<MFCTreeManagerNode>(title, data, GetTreeCtrl(), newNodeInfo, nullptr)
	);
}

void MFCTreeManager::SetObjectTypes(
	const std::vector<std::string>& objectTypes, 
	const std::vector<std::string>& lightTypes
)
{
	this->objectTypes = objectTypes;

	for (auto& type : objectTypes)
	{
		if (type != "Camera")
		{
			AddRootNode("Object", type);
		}
	}

	for (auto& lightType : lightTypes)
	{
		rootNodes["Light"]->AddNode("LightType", lightType);
	}
}

void MFCTreeManager::AddModelToTree(const std::string& modelName)
{
	rootNodes["Solid"]->AddNode("Model", modelName);
}

void MFCTreeManager::AddSolidToTree(const std::string& modelName, const std::string& solidName)
{
	rootNodes["Solid"]->nextNodes[modelName]->AddNode("Solid", solidName);
}
void MFCTreeManager::AddLightToTree(const std::string& lightType, const std::string& lightName)
{
	rootNodes["Light"]->nextNodes[lightType]->AddNode("Light", lightName);
}

void MFCTreeManager::AddSoundToTree(const std::string& soundName)
{
	rootNodes["Sound"]->AddNode("Sound", soundName);
}

MFCTreeManager::MFCTreeManagerNode* MFCTreeManager::FindNodeByItem(HTREEITEM item)
{
	for (auto& node : rootNodes)
	{
		MFCTreeManagerNode* currentNode = node.second->FindNodeBy(item);
		if (currentNode)
		{
			return currentNode;
		}
	}

	return nullptr;
}

std::vector<std::pair<std::string, std::string>> MFCTreeManager::GetAllOfRootsSelectedNode()
{
	std::vector<std::pair<std::string, std::string>> allDataRoots;
 	
	MFCTreeManagerNode* currentNode = FindNodeByItem(objectTree.GetSelectedItem());
	
	while (currentNode)
	{
		allDataRoots.push_back({ currentNode->title, currentNode->data });
		currentNode = currentNode->previousNode;
	}

	return allDataRoots;
}


void MFCTreeManager::ClearNonRootNodes()
{
	rootNodes["Solid"]->ClearAllNextNodes();
	for (auto& lightNodes : rootNodes["Light"]->nextNodes)
	{
		lightNodes.second->ClearAllNextNodes();
	}
	rootNodes["Sound"]->ClearAllNextNodes();
}