#pragma once

#include "types.hpp"
#include "slaballocator.hpp"

namespace Gx {

	void dumpString(char const *const str);
	void dump64(uint64_t const value);

	constexpr int t = 3;
	class BTreeNode;
	class BTree;

	class BTreeNode {
		size_t keys[2 * t - 1];  // An array of keys
		BTreeNode *C[2* t]; // An array of child pointers
		int16_t numKeys;     // Current number of keys
		int16_t leaf; // Is true when node is leaf. Otherwise false

	public:

		BTreeNode();   // Constructor


		// A function to traverse all nodes in a subtree rooted with this node
		void traverse();

		// A function to search a key in subtree rooted with this node.
		BTreeNode *search(int k);   // returns NULL if k is not present.

		// A function that returns the index of the first key that is greater
		// or equal to k
		int findKey(int k);

		// A utility function to insert a new key in the subtree rooted with
		// this node. The assumption is, the node must be non-full when this
		// function is called
		void insertNonFull(BTree& tree, int k);

		// A utility function to split the child y of this node. i is index
		// of y in child array C[].  The Child y must be full when this
		// function is called
		void splitChild(BTree &tree, int i, BTreeNode *y);

		// A wrapper function to remove the key k in subtree rooted with
		// this node.
		void remove(BTree&, int k);

		// A function to remove the key present in idx-th position in
		// this node which is a leaf
		void removeFromLeaf(BTree&, int idx);

		// A function to remove the key present in idx-th position in
		// this node which is a non-leaf node
		void removeFromNonLeaf(BTree& tree, int idx);

		// A function to get the predecessor of the key- where the key
		// is present in the idx-th position in the node
		int getPred(int idx);

		// A function to get the successor of the key- where the key
		// is present in the idx-th position in the node
		int getSucc(int idx);

		// A function to fill up the child node present in the idx-th
		// position in the C[] array if that child has less than t-1 keys
		void fill(BTree &tree, int idx);

		// A function to borrow a key from the C[idx-1]-th node and place
		// it in C[idx]th node
		void borrowFromPrev(int idx);

		// A function to borrow a key from the C[idx+1]-th node and place it
		// in C[idx]th node
		void borrowFromNext(int idx);

		// A function to merge idx-th child of the node with (idx+1)th child of
		// the node
		void merge(BTree&, int idx);

		// Constructor (Initializes tree as empty)
		void init(bool isLeaf) {
			// Copy the given minimum degree and leaf property
			leaf = isLeaf;

			// Allocate memory for maximum number of possible keys
			// and child pointers

			// Initialize the number of keys as 0
			numKeys = 0;
		}


		// Make BTree friend of this so that we can access private members of
		// this class in BTree functions
		friend class BTree;
	};

	class BTree {
		BTreeNode *root; // Pointer to root node
	public:
		SlabAllocator<BTreeNode>& alloc;
	public:
		BTree(SlabAllocator<BTreeNode>& alloc) : alloc(alloc){
		}
		void traverse() {
			dumpString("ROOT: ");
			for(auto i = 0; i < root->numKeys; ++i) {
				dump64(root->keys[i]);
				dumpString(" ");
			}
			dumpString("TREE: ");
			if (root != nullptr) root->traverse();
			dumpString("\n");
		}

		// function to search a key in this tree
		BTreeNode *search(int k) {
			return (root == nullptr) ? nullptr : root->search(k);
		}

		// The main function that inserts a new key in this B-Tree
		void insert(BTree &tree, int k);

		// The main function that removes a new key in thie B-Tree
		void remove(int k);
	};


// A utility function that returns the index of the first key that is
// greater than or equal to k
	int BTreeNode::findKey(int k) {
		int idx = 0;
		while (idx < numKeys && keys[idx] < k)
			++idx;
		return idx;
	}

// A function to remove the key k from the sub-tree rooted with this node
	void BTreeNode::remove(BTree& tree, int k) {
		int idx = findKey(k);

		// The key to be removed is present in this node
		if (idx < numKeys && keys[idx] == k) {

			// If the node is a leaf node - removeFromLeaf is called
			// Otherwise, removeFromNonLeaf function is called
			if (leaf)
				removeFromLeaf(tree, idx);
			else
				removeFromNonLeaf(tree, idx);
		}
		else {

			// If this node is a leaf node, then the key is not present in tree
			if (leaf) {
				dumpString(" NOT FOUND: ");
				dump64(k);
				dumpString("\n");
				return;
			}

			// The key to be removed is present in the sub-tree rooted with this node
			// The flag indicates whether the key is present in the sub-tree rooted
			// with the last child of this node
			bool flag = ((idx == numKeys) ? true : false);

			// If the child where the key is supposed to exist has less that t keys,
			// we fill that child
			if (C[idx]->numKeys < t)
				fill(tree, idx);

			// If the last child has been merged, it must have merged with the previous
			// child and so we recurse on the (idx-1)th child. Else, we recurse on the
			// (idx)th child which now has atleast t keys
			if (flag && idx > numKeys)
				C[idx - 1]->remove(tree, k);
			else
				C[idx]->remove(tree, k);
		}
		return;
	}

// A function to remove the idx-th key from this node - which is a leaf node
	void BTreeNode::removeFromLeaf(BTree& tree, int idx) {

		// Move all the keys after the idx-th pos one place backward
		for (int i = idx + 1; i < numKeys; ++i)
			keys[i - 1] = keys[i];

		// Reduce the count of keys
		numKeys--;

		return;
	}

// A function to remove the idx-th key from this node - which is a non-leaf node
	void BTreeNode::removeFromNonLeaf(BTree& tree, int idx) {

		int k = keys[idx];

		// If the child that precedes k (C[idx]) has atleast t keys,
		// find the predecessor 'pred' of k in the subtree rooted at
		// C[idx]. Replace k by pred. Recursively delete pred
		// in C[idx]
		if (C[idx]->numKeys >= t) {
			int pred = getPred(idx);
			keys[idx] = pred;
			C[idx]->remove(tree, pred);
		}

			// If the child C[idx] has less that t keys, examine C[idx+1].
			// If C[idx+1] has atleast t keys, find the successor 'succ' of k in
			// the subtree rooted at C[idx+1]
			// Replace k by succ
			// Recursively delete succ in C[idx+1]
		else if (C[idx + 1]->numKeys >= t) {
			int succ = getSucc(idx);
			keys[idx] = succ;
			C[idx + 1]->remove(tree, succ);
		}

			// If both C[idx] and C[idx+1] has less that t keys,merge k and all of C[idx+1]
			// into C[idx]
			// Now C[idx] contains 2t-1 keys
			// Free C[idx+1] and recursively delete k from C[idx]
		else {
			merge(tree, idx);
			C[idx]->remove(tree, k);
		}
		return;
	}

// A function to get predecessor of keys[idx]
	int BTreeNode::getPred(int idx) {
		// Keep moving to the right most node until we reach a leaf
		BTreeNode *cur = C[idx];
		while (!cur->leaf)
			cur = cur->C[cur->numKeys];

		// Return the last key of the leaf
		return cur->keys[cur->numKeys - 1];
	}

	int BTreeNode::getSucc(int idx) {

		// Keep moving the left most node starting from C[idx+1] until we reach a leaf
		BTreeNode *cur = C[idx + 1];
		while (!cur->leaf)
			cur = cur->C[0];

		// Return the first key of the leaf
		return cur->keys[0];
	}

// A function to fill child C[idx] which has less than t-1 keys
	void BTreeNode::fill(BTree &tree, int idx) {

		// If the previous child(C[idx-1]) has more than t-1 keys, borrow a key
		// from that child
		if (idx != 0 && C[idx - 1]->numKeys >= t)
			borrowFromPrev(idx);

			// If the next child(C[idx+1]) has more than t-1 keys, borrow a key
			// from that child
		else if (idx != numKeys && C[idx + 1]->numKeys >= t)
			borrowFromNext(idx);

			// Merge C[idx] with its sibling
			// If C[idx] is the last child, merge it with with its previous sibling
			// Otherwise merge it with its next sibling
		else {
			if (idx != numKeys)
				merge(tree, idx);
			else
				merge(tree, idx - 1);
		}
		return;
	}

// A function to borrow a key from C[idx-1] and insert it
// into C[idx]
	void BTreeNode::borrowFromPrev(int idx) {

		BTreeNode *child = C[idx];
		BTreeNode *sibling = C[idx - 1];

		// The last key from C[idx-1] goes up to the parent and key[idx-1]
		// from parent is inserted as the first key in C[idx]. Thus, the  loses
		// sibling one key and child gains one key

		// Moving all key in C[idx] one step ahead
		for (int i = child->numKeys - 1; i >= 0; --i)
			child->keys[i + 1] = child->keys[i];

		// If C[idx] is not a leaf, move all its child pointers one step ahead
		if (!child->leaf) {
			for (int i = child->numKeys; i >= 0; --i)
				child->C[i + 1] = child->C[i];
		}

		// Setting child's first key equal to keys[idx-1] from the current node
		child->keys[0] = keys[idx - 1];

		// Moving sibling's last child as C[idx]'s first child
		if (!child->leaf)
			child->C[0] = sibling->C[sibling->numKeys];

		// Moving the key from the sibling to the parent
		// This reduces the number of keys in the sibling
		keys[idx - 1] = sibling->keys[sibling->numKeys - 1];

		child->numKeys += 1;
		sibling->numKeys -= 1;

		return;
	}

// A function to borrow a key from the C[idx+1] and place
// it in C[idx]
	void BTreeNode::borrowFromNext(int idx) {

		BTreeNode *child = C[idx];
		BTreeNode *sibling = C[idx + 1];

		// keys[idx] is inserted as the last key in C[idx]
		child->keys[(child->numKeys)] = keys[idx];

		// Sibling's first child is inserted as the last child
		// into C[idx]
		if (!(child->leaf))
			child->C[(child->numKeys) + 1] = sibling->C[0];

		//The first key from sibling is inserted into keys[idx]
		keys[idx] = sibling->keys[0];

		// Moving all keys in sibling one step behind
		for (int i = 1; i < sibling->numKeys; ++i)
			sibling->keys[i - 1] = sibling->keys[i];

		// Moving the child pointers one step behind
		if (!sibling->leaf) {
			for (int i = 1; i <= sibling->numKeys; ++i)
				sibling->C[i - 1] = sibling->C[i];
		}

		// Increasing and decreasing the key count of C[idx] and C[idx+1]
		// respectively
		child->numKeys += 1;
		sibling->numKeys -= 1;

		return;
	}

// A function to merge C[idx] with C[idx+1]
// C[idx+1] is freed after merging
	void BTreeNode::merge(BTree& tree, int idx) {
		BTreeNode *child = C[idx];
		BTreeNode *sibling = C[idx + 1];

		// Pulling a key from the current node and inserting it into (t-1)th
		// position of C[idx]
		child->keys[t - 1] = keys[idx];

		// Copying the keys from C[idx+1] to C[idx] at the end
		for (int i = 0; i < sibling->numKeys; ++i)
			child->keys[i + t] = sibling->keys[i];

		// Copying the child pointers from C[idx+1] to C[idx]
		if (!child->leaf) {
			for (int i = 0; i <= sibling->numKeys; ++i)
				child->C[i + t] = sibling->C[i];
		}

		// Moving all keys after idx in the current node one step before -
		// to fill the gap created by moving keys[idx] to C[idx]
		for (int i = idx + 1; i < numKeys; ++i)
			keys[i - 1] = keys[i];

		// Moving the child pointers after (idx+1) in the current node one
		// step before
		for (int i = idx + 2; i <= numKeys; ++i)
			C[i - 1] = C[i];

		// Updating the key count of child and the current node
		child->numKeys += sibling->numKeys + 1;
		numKeys--;

		// Freeing the memory occupied by sibling
		tree.alloc.dealloc(sibling);
		return;
	}

// The main function that inserts a new key in this B-Tree
	void BTree::insert(BTree &tree, int k) {
		// If tree is empty
		if (root == nullptr) {
			// Allocate memory for root
			root = alloc.alloc();
			root->init(true);;
			root->keys[0] = k;  // Insert key
			root->numKeys = 1;  // Update number of keys in root
		}
		else // If tree is not empty
		{
			// If root is full, then tree grows in height
			if (root->numKeys == (2 * t - 1)) {
				// Allocate memory for new root
				BTreeNode *s = alloc.alloc();
				s->init(false);

				// Make old root as child of new root
				s->C[0] = root;

				// Split the old root and move 1 key to the new root
				s->splitChild(tree, 0, root);

				// New root has two children now.  Decide which of the
				// two children is going to have new key
				int i = 0;
				if (s->keys[0] < k)
					i++;
				s->C[i]->insertNonFull(tree,k);

				// Change root
				root = s;
			}
			else  // If root is not full, call insertNonFull for root
				root->insertNonFull(tree,k);
		}
	}

// A utility function to insert a new key in this node
// The assumption is, the node must be non-full when this
// function is called
	void BTreeNode::insertNonFull(BTree& tree, int k) {
		// Initialize index as index of rightmost element
		int i = numKeys - 1;

		// If this is a leaf node
		if (leaf == true) {
			// The following loop does two things
			// a) Finds the location of new key to be inserted
			// b) Moves all greater keys to one place ahead
			while (i >= 0 && keys[i] > k) {
				keys[i + 1] = keys[i];
				i--;
			}

			// Insert the new key at found location
			keys[i + 1] = k;
			numKeys = numKeys + 1;
		}
		else // If this node is not leaf
		{
			// Find the child which is going to have the new key
			while (i >= 0 && keys[i] > k)
				i--;

			// See if the found child is full
			if (C[i + 1]->numKeys == 2 * t - 1) {
				// If the child is full, then split it
				splitChild(tree, i + 1, C[i + 1]);

				// After split, the middle key of C[i] goes up and
				// C[i] is splitted into two.  See which of the two
				// is going to have the new key
				if (keys[i + 1] < k)
					i++;
			}
			C[i + 1]->insertNonFull(tree, k);
		}
	}

// A utility function to split the child y of this node
// Note that y must be full when this function is called
	void BTreeNode::splitChild(BTree &tree, int i, BTreeNode *y) {
		// Create a new node which is going to store (t-1) keys
		// of y
		BTreeNode *z = tree.alloc.alloc();
		z->init(y->leaf);
		z->numKeys = t - 1;

		// Copy the last (t-1) keys of y to z
		for (int j = 0; j < t - 1; j++)
			z->keys[j] = y->keys[j + t];

		// Copy the last t children of y to z
		if (y->leaf == false) {
			for (int j = 0; j < t; j++)
				z->C[j] = y->C[j + t];
		}

		// Reduce the number of keys in y
		y->numKeys = t - 1;

		// Since this node is going to have a new child,
		// create space of new child
		for (int j = numKeys; j >= i + 1; j--)
			C[j + 1] = C[j];

		// Link the new child to this node
		C[i + 1] = z;

		// A key of y will move to this node. Find location of
		// new key and move all greater keys one space ahead
		for (int j = numKeys - 1; j >= i; j--)
			keys[j + 1] = keys[j];

		// Copy the middle key of y to this node
		keys[i] = y->keys[t - 1];

		// Increment count of keys in this node
		numKeys = numKeys + 1;
	}

// Function to traverse all nodes in a subtree rooted with this node
	void BTreeNode::traverse() {
		// There are n keys and n+1 children, travers through n keys
		// and first n children
		int i;
		for (i = 0; i < numKeys; i++) {
			// If this is not leaf, then before printing key[i],
			// traverse the subtree rooted with child C[i].
			if (leaf == false)
				C[i]->traverse();
			dumpString(" "); dump64(keys[i]);
		}

		// Print the subtree rooted with last child
		if (leaf == false)
			C[i]->traverse();
	}

// Function to search key k in subtree rooted with this node
	BTreeNode *BTreeNode::search(int k) {
		// Find the first key greater than or equal to k
		int i = 0;
		while (i < numKeys && k > keys[i])
			i++;

		// If the found key is equal to k, return this node
		if (keys[i] == k)
			return this;

		// If key is not found here and this is a leaf node
		if (leaf == true)
			return nullptr;

		// Go to the appropriate child
		return C[i]->search(k);
	}

	void BTree::remove(int k) {
		if (!root) {
			dumpString("The tree is empty\n");
			return;
		}

		// Call the remove function for root
		root->remove(*this, k);

		// If the root node has 0 keys, make its first child as the new root
		//  if it has a child, otherwise set root as NULL
		if (root->numKeys == 0) {
			BTreeNode *tmp = root;
			if (root->leaf)
				root = nullptr;
			else
				root = root->C[0];

			// Free the old root
			alloc.dealloc(tmp);
		}
		return;
	}
}
