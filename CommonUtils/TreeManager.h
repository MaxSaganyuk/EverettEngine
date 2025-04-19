#pragma once

#include <memory>
#include <map>
#include <vector>

template<typename KeyType, typename ValueType>
class TreeManager
{
public:
	class TreeManagerNode
	{
		friend TreeManager<KeyType, ValueType>;

	public:
		TreeManagerNode(
			const KeyType& key,
			const ValueType& value,
			TreeManagerNode* previousNode
		) : key(key), value(value), previousNode(previousNode) {}

		void AddNode(const KeyType& key, const ValueType& value)
		{
			nextNodes.emplace(
				key,
				new TreeManagerNode(
					key,
					value,
					this
				)
			);
		}

		TreeManagerNode* FindNodeBy(const KeyType& keyToFind)
		{
			if (key == keyToFind)
			{
				return this;
			}

			for (auto& node : nextNodes)
			{
				TreeManagerNode* currentNode = node.second->FindNodeBy(keyToFind);
				if (currentNode)
				{
					return currentNode;
				}
			}

			return nullptr;
		}

		void DeleteAllSubnodes()
		{
			for (auto& node : nextNodes)
			{
				TreeManagerNode* currentNode = node.second;
				if (currentNode)
				{
					currentNode->DeleteAllSubnodes();
					delete currentNode;
				}
			}

			nextNodes.clear();
		}

		ValueType& GetValue() { return value; };
		const KeyType& GetKey() { return key; };

		std::vector<std::pair<KeyType, TreeManagerNode*>> GetChildNodes()
		{
			std::vector<std::pair<KeyType, TreeManagerNode*>> res;

			for (auto& node : nextNodes)
			{
				res.push_back(node);
			}
			
			return res;
		}

		TreeManagerNode* GetParentNode()
		{
			return previousNode;
		}

	private:
		KeyType key;
		ValueType value;
		TreeManagerNode* previousNode;
		std::map<KeyType, TreeManagerNode*> nextNodes;
	};

	void AddRootNode(const KeyType& key, const ValueType& value)
	{
		rootNodes.emplace(
			key,
			new TreeManagerNode(key, value, nullptr)
		);
	}
	
	TreeManagerNode* FindNodeBy(const KeyType& key)
	{
		for (auto& node : rootNodes)
		{
			TreeManagerNode* currentNode = node.second->FindNodeBy(key);
			if (currentNode)
			{
				return currentNode;
			}
		}

		return nullptr;
	}

	std::vector<std::pair<KeyType, TreeManagerNode*>> GetChildNodes()
	{
		std::vector<std::pair<KeyType, TreeManagerNode*>> res;

		for (auto& node : rootNodes)
		{
			res.push_back(node);
		}

		return res;
	}

	~TreeManager()
	{
		DeleteAllSubnodes();
	}

private:
	std::map<KeyType, TreeManagerNode*> rootNodes;

	void DeleteAllSubnodes()
	{
		for (auto& node : rootNodes)
		{
			TreeManagerNode* currentNode = node.second;
			if (currentNode)
			{
				currentNode->DeleteAllSubnodes();
				delete currentNode;
			}
		}

		rootNodes.clear();
		
	}
};

