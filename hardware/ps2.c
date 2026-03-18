#include "ps2.h"
#include "address_map.h"

bool is_W_pressed = false;
bool is_A_pressed = false;
bool is_S_pressed = false;
bool is_D_pressed = false;

void ps2_init() {

}

void ps2_update_keys() {
    volatile int *ps2_ptr = (volatile int *)PS2_BASE;
    int data;
    
    static bool is_break_code = false; 

    while (1) {
        data = *ps2_ptr;
        
        if ((data & 0x8000) == 0) {
            break; 
        }

        unsigned char byte = data & 0xFF;

        // prevent conflict of the direction keys with 0xE0 prefix
        if (byte == 0xE0) {
            continue; 
        } else if (byte == 0xF0) {
            is_break_code = true;
        } else {
            bool key_state = !is_break_code;

            // update key states
            if      (byte == 0x1D) is_W_pressed = key_state;
            else if (byte == 0x1C) is_A_pressed = key_state;
            else if (byte == 0x1B) is_S_pressed = key_state;
            else if (byte == 0x23) is_D_pressed = key_state;
            is_break_code = false; 
        }
    }
}