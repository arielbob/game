void export_level(Game_State *game_state, char *filename) {
    char test_string[] = "Hello, world!";
    bool32 write_result = platform_write_file(filename, test_string, sizeof(test_string), true);
    assert(write_result);
}
