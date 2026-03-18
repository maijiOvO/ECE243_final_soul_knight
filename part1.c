int pixel_buffer_start; // global variable

#define Blue 0x001F
#define Black 0
#define Red 0xF800
#define Pink 0xF81F
#define Green 0x07E0

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void swap(int *a, int *b);
void draw_line(int x0, int y0, int x1, int y1, int color);

// code not shown for clear_screen() and draw_line() subroutines
void clear_screen() {
    int y, x;
    for (x = 0; x < 320; x++){
        for (y = 0; y < 240; y++){
            plot_pixel (x, y, Black); // Black
        }
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;

        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

        *one_pixel_address = line_color;
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
    int delta_y = (y1 - y0 > 0) ? (y1 - y0) : (y0 - y1);
    int delta_x = (x1 - x0 > 0) ? (x1 - x0) : (x0 - x1);
    int is_steep = delta_y > delta_x;
    
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }

    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int delta_x = x1 - x0;
    int delta_y = ((y1 - y0) > 0 ? (y1 - y0) : (y0 - y1));
    int error = -(delta_x / 2);
    int y_step = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, color);
        } else {
            plot_pixel(x, y, color);
        }

        error = error + delta_y;
        if (error > 0) {
            y = y + y_step;
            error = error - delta_x;
        }
    }
}


int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    clear_screen();
    draw_line(0, 0, 150, 150, Blue);   // this line is blue
    draw_line(150, 150, 319, 0, Green); // this line is green
    draw_line(0, 239, 319, 239, Red); // this line is red
    draw_line(319, 0, 0, 239, Pink);   // this line is a pink color
}