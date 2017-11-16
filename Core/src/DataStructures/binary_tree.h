#pragma once

#include <map>

namespace ng {
	namespace dstructs {

		template <typename T>
		class BinaryTree
		{
		private:
			template <typename T>
			struct BinaryTreeNode {
				T value;
				BinaryTreeNode* leftChild = nullptr;
				BinaryTreeNode* rightChild = nullptr;
			};

			int numOfNodes = 1;
			BinaryTreeNode<T>* startNode = nullptr;
		public:

			void insert(BinaryTreeNode<T>* node);

			BinaryTreeNode* search(T value);
		};

		template<typename T>
		inline void BinaryTree<T>::search(T value)
		{
			BinaryTreeNode<T>* currentNode = startNode;
			for (int i = 0; i < numOfNodes; ++i) {
				if (currentNode->value == value) {
					return currentNode;
				}
				else if (value > currentNode->value) {
					currentNode = currentNode->rightChild;
				}
				else {
					currentNode = currentNode->leftChild;
				}
			}
		}

		template<typename T>
		inline void BinaryTree<T>::insert(BinaryTreeNode<T>* node)
		{
			BinaryTreeNode<T>* currentNode = startNode;
			for (int i = 0; i < numOfNodes; ++i) {
				if (node->number > currentNode->number) {
					if (currentNode->rightChild != nullptr) {
						currentNode = currentNode->rightChild;
					}
					else {
						currentNode->rightChild = node;
						break;
					}
				}
				else {
					if (currentNode->leftChild != nullptr) {
						currentNode = currentNode->leftChild;
					}
					else {
						currentNode->leftChild = node;
						break;
					}
				}
			}
		}

	}
}
