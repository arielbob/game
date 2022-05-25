#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define FOR_LIST_NODES(type, linked_list) \
    for (Node<type> *current_node = linked_list.cap->next; \
         current_node != linked_list.cap; \
         current_node = current_node->next) 

template <class T>
struct Node {
    T value;
    Node<T> *prev;
    Node<T> *next;
};

template <class T>
struct Linked_List {
    Allocator *allocator;
    Node<T> *cap;
    int32 num_entries;
    int32 total_added_ever;
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
    list->cap = (Node<T> *) allocate(list->allocator, sizeof(Node<T>), true);
    list->cap->prev = list->cap;
    list->cap->next = list->cap;
}

template <class T>
void add(Linked_List<T> *list, T value) {
    Node<T> *new_node = (Node<T> *) allocate(list->allocator, sizeof(Node<T>));
    new_node->value = value;

    Node<T> *cap = list->cap;
    cap->prev->next = new_node;
    new_node->next = cap;
    new_node->prev = cap->prev;
    cap->prev = new_node;
    list->num_entries++;
    list->total_added_ever++;
}

template <class T>
void remove(Linked_List<T> *list, Node<T> *node) {
    Node<T> *prev = node->prev;
    Node<T> *next = node->next;
    prev->next = next;
    next->prev = prev;

    // we may want to try this, but it seems annoying, since unless every struct has a field with its allocator,
    // this will be cumbersome. actually, for some things it doesn't seem too bad, for example with textures.
    // could always add another procedure that does this.

    deallocate(node->value);
    deallocate(list->allocator, node);
    list->num_entries--;
}

template <class T>
void deallocate(Linked_List<T> *list) {
    Node<T> *current_node = list->cap->next;
    while (current_node != list->cap) {
        Node<T> *next = current_node->next;
        remove(list, current_node);
        current_node = next;
    }
}

template <class T>
bool32 is_last(Linked_List<T> *list, Node<T> *node) {
    return (node->next == list->cap);
}

#endif
