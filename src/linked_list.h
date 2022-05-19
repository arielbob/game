#ifndef LINKED_LIST_H
#define LINKED_LIST_H

template <class T>
struct Node {
    T value;
    Node<T> *prev;
    Node<T> *next;
};

template <class T>
struct Linked_List {
    Allocator *allocator;
    Node<T> cap;
};

#define make_and_init_linked_list(type, linked_list_pointer, allocator_pointer) \
    *linked_list_pointer = make_linked_list<type>(allocator_pointer);   \
    init_linked_list(linked_list_pointer)

template <class T>
Linked_List<T> make_linked_list(Allocator *allocator) {
    Linked_List<T> list = {};
    list.allocator = allocator;
    return list;
}

template <class T>
void init_linked_list(Linked_List<T> *list) {
    // an empty linked list just has a single node whose prev and next pointers point to itself
    list->cap.prev = &list->cap;
    list->cap.next = &list->cap;
}

template <class T>
void add(Linked_List<T> *list, T value) {
    Node<T> *new_node = (Node<T> *) allocate(list->allocator, sizeof(Node<T>));
    new_node->value = value;

    Node<T> *cap = &list->cap;
    cap->prev->next = new_node;
    new_node->next = cap;
    new_node->prev = cap->prev;
    cap->prev = new_node;
}

template <class T>
void remove(Linked_List<T> *list, Node<T> *node) {
    Node *prev = node->prev;
    Node *next = node->next;
    prev->next = next;
    next->prev = prev;

    // we may want to try this, but it seems annoying, since unless every struct has a field with its allocator,
    // this will be cumbersome. actually, for some things it doesn't seem too bad, for example with textures.
    // could always add another procedure that does this.
    // deallocate(node->value);

    // NOTE: this only deallocates the node and not the value inside the node.
    deallocate(list->allocator, node);
}

#endif
