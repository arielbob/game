#include "platform.h"
#include "memory.h"
#include "game.h"

global_variable int32 samples_written_2;

void fill_sound_buffer_with_sine_wave(Sound_Output *sound_output, uint32 num_samples) {
    assert(num_samples < sound_output->max_samples); 

    real32 frequency = 252.0f;
    real32 volume = 10000.0f;
    int16 *sound_buffer = sound_output->sound_buffer;

    // NOTE: we use 252.0 is a factor of our sample rate (44100). we want it to be a factor so that we can
    //       cleanly loop around our samples_written variable. this prevents sound artifacts due to floating
    //       point error.
    int32 sample_count_period = (int32) ((1.0f / frequency) * sound_output->samples_per_second);

    for (uint32 i = 0; i < num_samples; i++) {
        real32 x = (real32) samples_written_2 / sound_output->samples_per_second;

        int16 sample = (int16) (volume * sinf(frequency * 2.0f * PI * x));

        *(sound_buffer++) = sample;
        *(sound_buffer++) = sample;
        samples_written_2++;
        samples_written_2 %= sample_count_period;
    }
}

void fill_sound_buffer_with_audio(Sound_Output *sound_output, bool32 is_playing,
                                  Audio_Source *audio,
                                  int32 num_samples) {
    assert((uint32) num_samples < sound_output->max_samples); 
    int16 *sound_buffer = sound_output->sound_buffer;
    int32 i = 0;

    if (is_playing) {
        int16 *music_buffer_at = audio->samples;
        music_buffer_at += audio->current_sample * 2;

        for (; i < num_samples; i++) {
            if (audio->current_sample >= audio->total_samples) {
                if (audio->should_loop) {
                    audio->current_sample = 0;
                    music_buffer_at = audio->samples;
                } else {
                    break;
                }
            }

            *(sound_buffer++) = (int16) (*(music_buffer_at++) * audio->volume);
            *(sound_buffer++) = (int16) (*(music_buffer_at++) * audio->volume);
            audio->current_sample++;
        }
    }

    for (; i < num_samples; i++) {
        *(sound_buffer++) = 0;
        *(sound_buffer++) = 0;
    }
}

Audio_Source make_audio_source(uint32 total_samples, uint32 current_sample,
                               real32 volume, bool32 should_loop,
                               int16 *samples) {
    Audio_Source audio_source;
    audio_source.total_samples = total_samples;
    audio_source.current_sample = current_sample;
    audio_source.volume = volume;
    audio_source.should_loop = should_loop;
    audio_source.samples = samples;

    return audio_source;
}

Entity make_entity(char *mesh_name, Transform transform) {
    Entity entity = { mesh_name, transform };
    return entity;
}

void add_mesh(Game_State *game_state, Mesh mesh) {
    assert(game_state->num_meshes < MAX_MESHES);
    game_state->meshes[game_state->num_meshes++] = mesh;
}

void add_entity(Game_State *game_state, Entity entity) {
    assert(game_state->num_entities < MAX_ENTITIES);
    game_state->entities[game_state->num_entities++] = entity;
}

void init_camera(Camera *camera, Display_Output *display_output) {
    camera->position = make_vec3(0.0f, 0.0f, -5.0f);
    camera->pitch = 0.0f;
    camera->fov_x_degrees = 90.0f;
    camera->aspect_ratio = (real32) display_output->width / display_output->height;
    camera->near = 0.1f;
    camera->far = 1000.0f;
    camera->forward = z_axis;
    camera->right = x_axis;
}

void init_game(Memory *memory, Game_State *game_state,
               Display_Output *display_output,
               Sound_Output *sound_output, uint32 num_samples) {
    Arena_Allocator *game_data_arena = &memory->game_data;
    File_Data music_file_data = platform_open_and_read_file((Allocator *) game_data_arena,
                                                            "../drive my car.wav");
    Wav_Data *wav_data = (Wav_Data *) music_file_data.contents;

    Audio_Source *music = &game_state->music;
    *music = make_audio_source(wav_data->subchunk_2_size / (wav_data->bits_per_sample / 8 * 2),
                               0, 1.0f, true, (int16 *) &wav_data->data);

    Camera *camera = &game_state->render_state.camera;
    init_camera(camera, display_output);

    game_state->is_initted = true;
}

bool32 was_clicked(Controller_Button_State button_state) {
    return (button_state.is_down && !button_state.was_down);
}

void update(Memory *memory, Game_State *game_state,
            Controller_State *controller_state,
            Display_Output *display_output,
            Sound_Output *sound_output, uint32 num_samples) {
    if (!game_state->is_initted) {
        init_game(memory, game_state, display_output, sound_output, num_samples);
        return;
    }

    game_state->cursor_pos = controller_state->current_mouse;
    UI_Manager *ui_manager = &game_state->ui_manager;

    //game_state->left_mouse_is_down = controller_state->left_mouse.is_down;
    bool32 btn1_clicked = do_button(ui_manager, controller_state,
                                      20.0f, 50.0f, 100.0f, 30.0f,
                                      "load mesh", "times24", "load_mesh");
    bool32 btn2_clicked = do_button(ui_manager, controller_state,
                                    50.0f, 360.0f, 200.0f, 30.0f,
                                    "toggle music", "times24", "toggle_music");

#if 0    
    do_text_box(ui_manager, controller_state,
                250.0f, 360.0f, 200.0f, 30.0f,
                "toggle music", "times24", "toggle_music");
#endif

    // TODO: GetOpenFileName blocks, so we should do the open file dialog stuff on a separate thread.
    //       https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads
    if (btn1_clicked) {
        char *filepath = (char *) arena_push(&memory->string_arena, PLATFORM_MAX_PATH);
        char *mesh_name_buffer = (char *) arena_push(&memory->string_arena, MESH_NAME_MAX_SIZE);

        if (platform_open_file_dialog(filepath, PLATFORM_MAX_PATH)) {
            //Marker m = begin_region(memory);
            //File_Data file_data = platform_open_and_read_file((Allocator *) &memory->global_stack, filepath);
            //end_region(memory, m);

            // TODO: prompt user to enter name for mesh; just using filepath name for now
            Mesh mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, filepath,
                                           mesh_name_buffer, MESH_NAME_MAX_SIZE);
            game_state->is_naming_mesh = true;
            game_state->mesh_to_add = mesh;

            // add_mesh(game_state, mesh);

#if 0
            Transform transform = {};
            transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
            Entity entity = make_entity(mesh.name, transform);
            add_entity(game_state, entity);
#endif
        }
    }

    if (game_state->is_naming_mesh) {
        UI_Text_Box_Style style = {
            "times24",
            500.0f, 30.0f,
            5.0f, 5.0f,
        };

        do_text_box(ui_manager, controller_state,
                    251.0f, 360.0f,
                    game_state->mesh_to_add.name, game_state->mesh_to_add.name_size,
                    style, "mesh_name_text_box");
        bool32 submit_clicked = do_button(ui_manager, controller_state,
                                          752.0f, 360.0f, 200.0f, 30.0f,
                                          "submit", "times24", "mesh_name_text_box_submit");
        if (submit_clicked) {
            game_state->is_naming_mesh = false;
            add_mesh(game_state, game_state->mesh_to_add);
        }
        // TODO: add a cancel button that cancels this operation and deallocates the mesh strings and mesh data
        // TODO: we may want to think of a better API for strings, since it's kind of painful right now
    }

    if (btn2_clicked) {
        game_state->is_playing_music = !game_state->is_playing_music;
    }

    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    //game_state->current_char = controller_state->pressed_key;
}