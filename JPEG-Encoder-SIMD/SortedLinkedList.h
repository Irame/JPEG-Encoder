// parts copied from http://trestle.icarnegie.com/content/SSD/SSD5/2.2/normal/pg-linear/pg-linked-lists/pg-list-implementation/pg-list-implementation.html

/*!Begin Snippet:filebegin*/
#ifndef _SortedLinkedList_H_
#define _SortedLinkedList_H_
#include <iterator>
#include <cassert>

using namespace std;

/*!Begin Snippet:fullnode*/
/*!Begin Snippet:private*/
template <typename T, class C>
class SortedLinkedList {

private:
	class Node {
		friend class SortedLinkedList<T, C>;

	private:
		T data;
		Node* next;

	public:
		Node(T d, Node* n = nullptr) : data(d), next(n) {}
	};
	/*!End Snippet:fullnode*/

	C comp;

	Node* head;  // Beginning of list
	int count;    // Number of nodes in list
				  /*!End Snippet:private*/

public:
	~SortedLinkedList(void);  // Destructor

						/*!Begin Snippet:simple*/
						// Default constructor
	SortedLinkedList(C comp) : comp(comp), head(nullptr), count(0) {}

	// Returns a reference to first element
	T& top(void) {
		assert(head != nullptr);
		return head->data;
	}

	// Returns count of elements of list
	int size(void) {
		return count;
	}

	// Returns whether or not list contains any elements
	bool empty(void) {
		return count == 0;
	}
	/*!End Snippet:simple*/

	void push(T);  // Insert element at beginning
	void pop(void);  // Remove element from end

	void dump(void);  // Output contents of list
};

/*!Begin Snippet:destructor*/
// Destructor
template <typename T, class C>
SortedLinkedList<T, C>::~SortedLinkedList(void) {

	while (!this->empty()) {
		this->pop();
	}
}
/*!End Snippet:destructor*/

/*!Begin Snippet:push*/
// Insert an element at the beginning
template <typename T, class C>
void SortedLinkedList<T, C>::push(T d) {
	Node* prevNode = nullptr;

	Node* curNode = head;
	while (curNode != nullptr && comp(curNode->data, d))
	{
		prevNode = curNode;
		curNode = curNode->next;
	}
	
	if (prevNode == nullptr)
	{
		head = new Node(d, head);
	}
	else
	{
		prevNode->next = new Node(d, curNode);
	}

	count++;
}
/*!End Snippet:push*/

/*!Begin Snippet:pop*/
// Remove an element from the beginning
template <typename T, class C>
void SortedLinkedList<T, C>::pop(void) {

	assert(head != nullptr);

	Node* old_head = head;

	if (this->size() == 1) {
		head = nullptr;
	}
	else {
		head = head->next;
	}

	delete old_head;
	count--;
}
/*!End Snippet:pop*/

/*!Begin Snippet:printlist*/
// Display the contents of the list
template <typename T, class C>
void SortedLinkedList<T, C>::dump(void) {

	cout << "(";

	Node* current = head;

	if (current != nullptr) {

		while (current->next != nullptr) {
			cout << current->data << ", ";
			current = current->next;
		}
		cout << current->data;
	}

	cout << ")" << endl;
}
/*!End Snippet:printlist*/

/*!End Snippet:filebegin*/
#endif