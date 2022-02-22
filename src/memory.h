#ifndef MEMORY_H
#define MEMORY_H

struct Arena {
    uint32 size;
    uint32 used;
    void *data;
};

#endif
