// parts copied from http://trestle.icarnegie.com/content/SSD/SSD5/2.2/normal/pg-linear/pg-linked-lists/pg-list-implementation/pg-list-implementation.html

/*!Begin Snippet:filebegin*/
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include <iterator>
#include <cassert>

using namespace std;

/*!Begin Snippet:fullnode*/
/*!Begin Snippet:private*/
template <typename T>
class LinkedList {

private:
	class Node {
		friend class LinkedList<T>;

	private:
		T data;
		Node* prev;
		Node* next;

	public:
		Node(T d, Node* p = nullptr, Node* n = nullptr) : data(d), prev(p), next(n) {}
	}
	;
	/*!End Snippet:fullnode*/

	// iterator class is parametrized by pointer type 
	class LinkedListIterator : public std::iterator<input_iterator_tag, T> {
		friend class LinkedList<T>;

	private:
		explicit LinkedListIterator(Node* node, int index);

		Node* node;
		int index;

	public:
		LinkedListIterator(const LinkedListIterator& other);
		LinkedListIterator& operator=(const LinkedListIterator& other);

		LinkedListIterator operator++(int);
		LinkedListIterator& operator++();

		LinkedListIterator operator--(int);
		LinkedListIterator& operator--();

		bool operator==(const LinkedListIterator& other);
		bool operator!=(const LinkedListIterator& other);

		T& operator*();
		T* operator->();

		int operator-(const LinkedListIterator& other);

		bool operator<(const LinkedListIterator& other);
		bool operator>(const LinkedListIterator& other);

		LinkedListIterator operator-(int val);
		LinkedListIterator operator+(int val);
	};

	Node* head;  // Beginning of list
	Node* tail;  // End of list
	int count;    // Number of nodes in list
				  /*!End Snippet:private*/

public:

	LinkedList(const LinkedList<T>& src);  // Copy constructor
	~LinkedList(void);  // Destructor

						/*!Begin Snippet:simple*/
						// Default constructor
	LinkedList(void) : head(nullptr), tail(nullptr), count(0) {}

	// Returns a reference to first element
	T& front(void) {
		assert(head != nullptr);
		return head->data;
	}

	// Returns a reference to last element
	T& back(void) {
		assert(tail != nullptr);
		return tail->data;
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

	void push_front(T);  // Insert element at beginning
	void push_back(T);   // Insert element at end
	void pop_front(void);  // Remove element from beginning
	void pop_back(void);  // Remove element from end

	LinkedListIterator begin();
	LinkedListIterator end();

	void dump(void);  // Output contents of list
};

template <typename T>
LinkedList<T>::LinkedListIterator::LinkedListIterator(Node* node, int index)
	: node(node), index(index)
{}

template <typename T>
LinkedList<T>::LinkedListIterator::LinkedListIterator(const LinkedListIterator& other)
	: node(other.node), index(other.index)
{}

template <typename T>
typename LinkedList<T>::LinkedListIterator& LinkedList<T>::LinkedListIterator::operator=(const LinkedListIterator& other)
{
	if (this != &other)
	{
		node = other.node;
		index = other.index;
	}
	return *this;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::LinkedListIterator::operator++(int)
{
	LinkedListIterator cResult(*this);
	++(*this);
	return cResult;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator& LinkedList<T>::LinkedListIterator::operator++()
{
	node = node->next;
	return *this;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::LinkedListIterator::operator--(int)
{
	LinkedListIterator cResult(*this);
	--(*this);
	return cResult;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator& LinkedList<T>::LinkedListIterator::operator--()
{
	node = node->prev;
	return *this;
}

template <typename T>
bool LinkedList<T>::LinkedListIterator::operator==(const LinkedListIterator& other)
{
	return node == other.node;
}

template <typename T>
bool LinkedList<T>::LinkedListIterator::operator!=(const LinkedListIterator& other)
{
	return node != other.node;
}

template <typename T>
T& LinkedList<T>::LinkedListIterator::operator*()
{
	return node->data;
}

template <typename T>
T* LinkedList<T>::LinkedListIterator::operator->()
{
	return &(node->data);
}

template <typename T>
int LinkedList<T>::LinkedListIterator::operator-(const LinkedListIterator& other)
{
	return index - other.index;
}

template <typename T>
bool LinkedList<T>::LinkedListIterator::operator<(const LinkedListIterator& other)
{
	return index < other.index;
}

template <typename T>
bool LinkedList<T>::LinkedListIterator::operator>(const LinkedListIterator& other)
{
	return index > other.index;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::LinkedListIterator::operator-(int val)
{
	LinkedListIterator result(*this);
	for (int i = 0; i < val; i++)
	{
		--result;
	}
	return result;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::LinkedListIterator::operator+(int val)
{
	LinkedListIterator result(*this);
	for (int i = 0; i < val; i++)
	{
		++result;
	}
	return result;
}

/*!Begin Snippet:copyconstructor*/
// Copy constructor
template <typename T>
LinkedList<T>::LinkedList(const LinkedList<T>& src) :
	head(nullptr), tail(nullptr), count(0) {

	Node* current = src.head;
	while (current != nullptr) {
		this->push_back(current->data);
		current = current->next;
	}

}
/*!End Snippet:copyconstructor*/

/*!Begin Snippet:destructor*/
// Destructor
template <typename T>
LinkedList<T>::~LinkedList(void) {

	while (!this->empty()) {
		this->pop_front();
	}
}
/*!End Snippet:destructor*/

/*!Begin Snippet:pushfront*/
// Insert an element at the beginning
template <typename T>
void LinkedList<T>::push_front(T d) {

	Node* new_head = new Node(d, nullptr, head);

	if (this->empty()) {
		tail = new_head;
	}
	else {
		head->prev = new_head;
	}

	head = new_head;
	count++;
}
/*!End Snippet:pushfront*/

/*!Begin Snippet:pushback*/
// Insert an element at the end
template <typename T>
void LinkedList<T>::push_back(T d) {

	Node* new_tail = new Node(d, tail, nullptr);

	if (this->empty()) {
		head = new_tail;
	}
	else {
		tail->next = new_tail;
	}

	tail = new_tail;
	count++;
}
/*!End Snippet:pushback*/

/*!Begin Snippet:popfront*/
// Remove an element from the beginning
template <typename T>
void LinkedList<T>::pop_front(void) {

	assert(head != nullptr);

	Node* old_head = head;

	if (this->size() == 1) {
		head = nullptr;
		tail = nullptr;
	}
	else {
		head = head->next;
		head->prev = nullptr;
	}

	delete old_head;
	count--;
}
/*!End Snippet:popfront*/

/*!Begin Snippet:popback*/
// Remove an element from the end
template <typename T>
void LinkedList<T>::pop_back(void) {

	assert(tail != nullptr);

	Node* old_tail = tail;

	if (this->size() == 1) {
		head = nullptr;
		tail = nullptr;
	}
	else {
		tail = tail->prev;
		tail->next = nullptr;
	}

	delete old_tail;
	count--;
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::begin()
{
	return LinkedListIterator(head, 0);
}

template <typename T>
typename LinkedList<T>::LinkedListIterator LinkedList<T>::end()
{
	return LinkedListIterator(nullptr, count);
}

/*!End Snippet:popback*/

/*!Begin Snippet:printlist*/
// Display the contents of the list
template <typename T>
void LinkedList<T>::dump(void) {

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