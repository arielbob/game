void calculate_percentage_and_fill_remaining_on_level(UI_Widget *level_node) {
    // node just needs to be on the same level
    UI_Widget *node = level_node;

    if (node->parent) {
        node = level_node->parent->first;
    }
    // if there's no parent, then we're at root and by definition it has no siblings

    while (node) {
        // i think we actually need to go pre-order
        Allocator *temp_region = begin_region();
        debug_print(to_char_array(temp_region, node->id.name));
        debug_print("\n");
        end_region(temp_region);
        node = node->next;
    }
}

// this is "breadth-first" but only for the children of each node
void ui_calculate_percentage_and_fill_remaining(UI_Widget *current) {
    // depth-first level order traversal
    bool32 revisiting = false;
    while (current) {
        UI_Widget *parent = current->parent;

        if (!current->prev && !revisiting) {
            calculate_percentage_and_fill_remaining_on_level(current);
        }
        
        if (current->first && !revisiting) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
                revisiting = false;
            } else {
                if (!parent) return;

                current = parent;
                revisiting = true;
            }
        }
    }
}

void ui_calculate_percentage_and_fill_remaining_depth_first(UI_Widget *current) {
    // depth-first level order traversal
    bool32 revisiting = false;
    while (current) {
        UI_Widget *parent = current->parent;

        if (current->first && !revisiting) {
            current = current->first;
        } else {
            if (revisiting) {
                if (!current->next) {
                    // do the per level stuff here
                    if (!parent) {
                        printf("at parent\n");
                    } else {
                        UI_Widget *node = parent->first;
                        while (node) {
                            Allocator *temp_region = begin_region();
                            debug_print(to_char_array(temp_region, node->id.name));
                            debug_print("\n");
                            end_region(temp_region);
                            node = node->next;
                        }
                    }
                }

                revisiting = false;
            } else {
                // we're at a leaf node
                if (!current->next && !current->first) {
                    // compute it
                    UI_Widget *node = parent->first;
                    while (node) {
                        Allocator *temp_region = begin_region();
                        debug_print(to_char_array(temp_region, node->id.name));
                        debug_print("\n");
                        end_region(temp_region);
                        node = node->next;
                    }
                }
            }
            
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                current = parent;
                revisiting = true;
            }
        }
    }
}

void test_depth_first_traversal() {
    UI_Widget *test_root = ui_push_widget("test_root", {});
    {
        ui_push_widget("20", {});
        {
            ui_push_widget("40", {});
            {
                ui_push_widget("80", {});
                {
                    ui_add_widget("120", {});
                } ui_pop_widget();
            } ui_pop_widget();
            ui_push_widget("50", {});
            {
                ui_add_widget("90", {});
            } ui_pop_widget();
        } ui_pop_widget();

        ui_push_widget("30", {});
        {
            ui_add_widget("60", {});
            ui_push_widget("70", {});
            {
                ui_add_widget("100", {});
                ui_add_widget("110", {});
            } ui_pop_widget();
        } ui_pop_widget();
    } ui_pop_widget();

    ui_calculate_percentage_and_fill_remaining(test_root);
}

