#ifndef WIN32_THREADS
#define WIN32_THREADS

#define WIN32_MAX_CRITICAL_SECTIONS 256

// we want to be able to pass around Platform_Critical_Sections by copy..
struct Platform_Critical_Section {
    CRITICAL_SECTION *critical_section;
    Platform_Critical_Section *next_free;
    Platform_Critical_Section *next;
};

struct Win32_Critical_Section_Manager {
    CRITICAL_SECTION critical_section;
    
    //CRITICAL_SECTION critical_sections[WIN32_MAX_CRITICAL_SECTIONS];
    Platform_Critical_Section *first_free;
    Platform_Critical_Section *first;
    Platform_Critical_Section slots[WIN32_MAX_CRITICAL_SECTIONS];
    CRITICAL_SECTION critical_sections[WIN32_MAX_CRITICAL_SECTIONS];


    // deleting these critical sections should be handled by the owners of them
};

global_variable Win32_Critical_Section_Manager win32_critical_section_manager;

void win32_init_critical_sections() {
    win32_critical_section_manager = {};
    Win32_Critical_Section_Manager *manager = &win32_critical_section_manager;

    InitializeCriticalSection(&manager->critical_section);

    for (int32 i = 0; i < WIN32_MAX_CRITICAL_SECTIONS; i++) {
        manager->slots[i] = {
            &manager->critical_sections[i],
            ((i + 1) < WIN32_MAX_CRITICAL_SECTIONS) ? &manager->slots[i + 1] : NULL // next_free
        };
    }

    manager->first_free = manager->slots;
}

// we can't copy CRITICAL_SECTION objects, so all thse functions should take pointer
// arguments. if we wanted to change the API such that we return the object, i guess
// we could have some type of CRITICAL_SECTION pool and just set the struct member to
// a pointer to their CRITICAL_SECTION in the pool.
Platform_Critical_Section platform_make_critical_section() {
    // TODO: need to enter critical section for the manager    
    Win32_Critical_Section_Manager *manager = &win32_critical_section_manager;
    EnterCriticalSection(&manager->critical_section);

    if (!manager->first_free) {
        assert(!"Ran out of Win32 CRITICAL_SECTION slots!");
        LeaveCriticalSection(&manager->critical_section);
        return {};
    }

    Platform_Critical_Section *slot = manager->first_free;
    manager->first_free = slot->next_free;
    slot->next = manager->first;
    manager->first = slot;

    InitializeCriticalSection(slot->critical_section);

    LeaveCriticalSection(&manager->critical_section);

    return *slot;
}

void platform_enter_critical_section(Platform_Critical_Section *platform_critical_section) {
    EnterCriticalSection(platform_critical_section->critical_section);
}

void platform_leave_critical_section(Platform_Critical_Section *platform_critical_section) {
    LeaveCriticalSection(platform_critical_section->critical_section);
}

void platform_delete_critical_section(Platform_Critical_Section *platform_critical_section) {
    Win32_Critical_Section_Manager *manager = &win32_critical_section_manager;

    EnterCriticalSection(&manager->critical_section);

    Platform_Critical_Section *prev = NULL;
    Platform_Critical_Section *current = manager->first;
    
    while (current) {
        if (current->critical_section == platform_critical_section->critical_section) {
            break;
        }
        
        prev = current;
        current = current->next;
    }

    if (!current) {
        assert(!"Critical section does not exist in list!");
        return;
    }

    if (prev) {
        // remove from list of watchers
        prev->next = current->next;
    } else {
        // was first in list, so set manager->first
        manager->first = current->next;
    }
    
    DeleteCriticalSection(platform_critical_section->critical_section);
    platform_critical_section->next_free = manager->first_free;
    manager->first_free = platform_critical_section;
    
    LeaveCriticalSection(&manager->critical_section);
}

#endif
