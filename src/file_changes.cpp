#include "file_changes.h"

File_Changes_Queue make_file_changes_queue(Allocator *allocator, int32 capacity) {
    File_Changes_Queue queue = {};
    queue.allocator = allocator;
    queue.capacity = capacity;
    queue.start = 0;
    //queue.end = 0;
    queue.num_entries = 0;
    queue.entries = (File_Change *) allocate(allocator, sizeof(File_Change)*capacity);

    return queue;
}

void add(File_Changes_Queue *queue, String s) {
    assert(queue->num_entries < queue->capacity);

    // TODO: this may not be necessary
    File_Change new_entry = {};
    new_entry = copy(queue->allocator, s);
    
    int32 index = (queue->start + queue.num_entries) % queue.capacity;
    queue->entries[index] = new_entry;

    queue->num_entries++;
}

File_Change pop(File_Changes_Queue *queue) {
    // the lifetime of the change of the filename strings starts after we've finished processing
    // changes and ends when we've processed them.
    // so.. we can just have an arena allocator for everything and just clear it ever loop
}


void clear(File_Changes_Queue *queue) {
    clear(queue->allocator);
}

/*
[_, _, _]
   e   s

e == s means we're either full or empty
*/
