#pragma once
#include <string>
#include "graphics.h"

#include <readline/readline.h>
#include <readline/history.h>

// Draws the map. You can assume your load_map (string map_name)
// function is called before this function in the unit tests.
// Your main () program should do the same.
void draw_map();
void act_on_mouse_click (float x, float y, t_event_buttonPressed event);
void search(void (*drawscreen_ptr) (void));

t_bound_box getWorldCoordinates();

