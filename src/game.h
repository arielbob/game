#ifndef GAME_H
#define GAME_H

struct Sound_Output {
    int16 *sound_buffer;
    uint32 buffer_size;
    uint32 max_samples;
    uint32 samples_per_second;
};

struct Game_State {
};

#endif
