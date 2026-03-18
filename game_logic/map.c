#include "map.h"
#include <stdlib.h>

Rect obstacles[MAX_OBS];
int num_obstacles = 0;

//no collision checks, return 1 if none of them happened
int overlap(Rect a, Rect b){
    if(a.x + a.w <= b.x) return 0;          //if a's right edge is at or before b's left edge, no overlap
    if(b.x + b.w <= a.x) return 0;          //if b's right edge is at or before a's left edge, no overlap
    if(a.y + a.h <= b.y) return 0;          //if a's bottom edge is at or above b's top edge, no overlap.
    if(b.y + b.h <= a.y) return 0;          //if b's bottom edge is at or above a's top edge, no overlap.

    return 1;
}

void draw_rect(int x, int y, int w, int h, short int color){
    for(int row = y; row < y + h; row ++){
        for(int col = x; col < x + w; col ++){
            plot_pixel(col, row, color);
        }
    }
}

void draw_border(void){
    draw_rect(0, 0, SCREEN_W, BORDER_THICKNESS, RED);
    draw_rect(0, SCREEN_H - BORDER_THICKNESS, SCREEN_W, BORDER_THICKNESS, RED);
    draw_rect(0, 0, BORDER_THICKNESS, SCREEN_H, RED);
    draw_rect(SCREEN_W - BORDER_THICKNESS, 0, BORDER_THICKNESS, SCREEN_H, RED);
}

void draw_obstacles(void){
    for(int i = 0; i < num_obstacles; i++){
        draw_rect(obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h, RED);
    }
}

void draw_map(void){
    draw_border();
    draw_obstacles();
}


void init_obstacles(void) {
    num_obstacles = MIN_OBS + rand() % (MAX_OBS - MIN_OBS + 1);

    for (int i = 0; i < num_obstacles; i++) {
        int placed = 0;
        int tries = 0;

        while (!placed && tries < 200) {
            tries++;

            Rect candidate;
            int horizontal = rand() % 2;

            int len = MIN_OBS_LEN + rand() % (MAX_OBS_LEN - MIN_OBS_LEN + 1);
            int thick = MIN_OBS_THICK + rand() % (MAX_OBS_THICK - MIN_OBS_THICK + 1);

            if (horizontal) {
                candidate.w = len;
                candidate.h = thick;
            } else {
                candidate.w = thick;
                candidate.h = len;
            }

            candidate.x = BORDER_THICKNESS + 5 +
                          rand() % (SCREEN_W - candidate.w - 2 * (BORDER_THICKNESS + 5));
            candidate.y = BORDER_THICKNESS + 5 +
                          rand() % (SCREEN_H - candidate.h - 2 * (BORDER_THICKNESS + 5));

            int ok = 1;

            for (int j = 0; j < i; j++) {
                if (overlap(candidate, obstacles[j])) {
                    ok = 0;
                    break;
                }
            }

            if (ok) {
                obstacles[i] = candidate;
                placed = 1;
            }
        }

        if (!placed) {
            num_obstacles = i;
            break;
        }
    }
}

int can_move_to(int new_x, int new_y, int player_size) {
    Rect player = {new_x, new_y, player_size, player_size};

    Rect borders[4] = {
        {0, 0, SCREEN_W, BORDER_THICKNESS},
        {0, SCREEN_H - BORDER_THICKNESS, SCREEN_W, BORDER_THICKNESS},
        {0, 0, BORDER_THICKNESS, SCREEN_H},
        {SCREEN_W - BORDER_THICKNESS, 0, BORDER_THICKNESS, SCREEN_H}
    };

    for (int i = 0; i < 4; i++) {
        if (overlap(player, borders[i])) {
            return 0;
        }
    }

    for (int i = 0; i < num_obstacles; i++) {
        if (overlap(player, obstacles[i])) {
            return 0;
        }
    }

    return 1;
}