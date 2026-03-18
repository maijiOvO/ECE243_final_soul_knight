#include "map.h"
#include <stdlib.h>

Rect obstacle[MAX_OBS];
int num_obstacles = 0;

//no collision checks, return 1 if none of them happened
int overlap(Rect a, Rect b){
    if(a.x + a.w <= b.x) return 0;          //if a's right edge is at or before b's left edge, no overlap
    if(b.x + b.w <= a.x) return 0;          //if b's right edge is at or before a's left edge, no overlap
    if(a.y + a.h <= b.y) return 0;          //if a's bottom edge is at or above b's top edge, no overlap.
    if(b.y + b.h <= a.y) return 0;          //if b's bottom edge is at or above a's top edge, no overlap.
}

void init_obstacles(void){
    //1. choose number of obstacles
    num_obstacles = MIN_OBS + rand() % (MAX_OBS - MIN_OBS + 1);
    //2. define a safe zone
    Rect 
    //3. for each obstacle
    // -keep trying until you place it
    // -build a candidate rectangle 
    // -randomize orientation, size, and position
    // -check if it overlaps old obstacles or spwan zone 
    // -if valid, store it


}