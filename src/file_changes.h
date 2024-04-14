#ifndef FILE_CHANGES_H
#define FILE_CHANGES_H

struct File_Change {
    String filepath;
};

struct File_Watcher_State {
    Allocator *allocator;
    File_Change *changes;
    int32 num_changes;
    int32 capacity;
}

struct File_Changes_Queue {
    Allocator *allocator;
    File_Change *entries;
    int32 start;
    int32 end; // where to add the next one
    int32 capacity;
};

#endif
