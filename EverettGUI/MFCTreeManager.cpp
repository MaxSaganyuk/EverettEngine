#include "pch.h"
#include "MFCTreeManager.h"

MFCTreeManager::MFCTreeManagerNode::MFCTreeManagerNode(
	const AdString& title,
	const AdString& data,
	CTreeCtrl& objectTree,
	HTREEITEM nodeInfo,
	MFCTreeManagerNode* previousNode
)
	: title(title), data(data), objectTree(objectTree), nodeInfo(nodeInfo), previousNode(previousNode) {}

void MFCTreeManager::MFCTreeManagerNode::AddNode(const AdString& title, const AdString& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(title + L": " + data, nodeInfo);
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

void MFCTreeManager::AddRootNode(const AdString& title, const AdString& data)
{
	HTREEITEM newNodeInfo = objectTree.InsertItem(title + L": " + data);
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

void MFCTreeManager::AddModelToTree(const AdString& modelName)
{
	rootNodes["Solid"]->AddNode("Model", modelName);
}

void MFCTreeManager::AddSolidToTree(const AdString& modelName, const AdString& solidName)
{
	rootNodes["Solid"]->nextNodes[modelName]->AddNode("Solid", solidName);
}
void MFCTreeManager::AddLightToTree(const AdString& lightType, const AdString& lightName)
{
	rootNodes["Light"]->nextNodes[lightType]->AddNode("Light", lightName);
}

void MFCTreeManager::AddSoundToTree(const AdString& soundName)
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

std::vector<std::pair<AdString, AdString>> MFCTreeManager::GetAllOfRootsSelectedNode()
{
	std::vector<std::pair<AdString, AdString>> allDataRoots;
 	
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