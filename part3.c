
#include<stdlib.h>
#define Blue 0x001F
#define Black 0
#define Red 0xF800
#define Pink 0xF81F
#define Green 0x07E0
#define Width 5
#define Height 5

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void swap(int *a, int *b);
void draw_line(int x0, int y0, int x1, int y1, int color);
void draw_rec(int x, int y, int width, int height, int color);
void wait();
void init_everything();

volatile int pixel_buffer_start; // global variable
short int Buffer0[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer1[240][512];
int current_buffer = 0;         // can be 0 or 1 corresponding to current buffer

int prev_x[2][8]; 
int prev_y[2][8];

int cur_x[8];
int cur_y[8];
int colors[8];
int dx[8];
int dy[8];

void clear_screen() {
    int y, x;
    for (x = 0; x < 320; x++){
        for (y = 0; y < 240; y++){
            plot_pixel (x, y, 0); // Black
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
void draw_rec(int x, int y, int width, int height, int color){
    int half_w = width >> 1;
    int half_h = height >> 1;
    for(int i = (x - half_w); i <= (x + half_w); i++){
        for(int j = (y - half_h); j <= (y + half_h); j++){
            int cur_x = i;
            int cur_y = j;
            if((cur_x >= 0) && (cur_x < 320) && (cur_y >= 0)&&(cur_y < 240)){
                plot_pixel(cur_x, cur_y, color);
            }
        }
    }
}
void draw_line(int x0, int y0, int x1, int y1, int color){

    int delta_y = (y1-y0 > 0)? y1-y0 : y0 - y1; 
    int delta_x = (x1-x0 > 0)? x1-x0 : x0 - x1; 
    int is_steep = (delta_y > delta_x)? 1 : 0;

    if(is_steep){
        swap(&x0, &y0);
        swap(&x1, &y1);
    }

    if(x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
	
	delta_x = x1 - x0;
    delta_y = (y1 - y0 > 0) ? y1 - y0 : y0 - y1; // abs

    int error = - (delta_x / 2);
    int y_step = (y0 < y1)? 1 : -1;
    int y = y0;
    for (int x = x0; x < x1; x ++){
        if (is_steep){
            plot_pixel(y, x, color);
        }
        else{
            plot_pixel(x, y, color);
        }
        error = error + delta_y;
        if (error > 0){
            y = y + y_step;
            error = error - delta_x;
        }
    }
}
void wait(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // Buffer Register
    volatile int * pixel_status_ptr = (int *)0xFF20302C; // Status Register
    *pixel_ctrl_ptr = 1;
    while (*pixel_status_ptr & 0x01);
}
void init_everything(){
    for(int i = 0; i < 8; i ++){    //init directions
        if(rand()%2){dx[i] = 1;}
        else{dx[i] = -1;}
        if(rand()%2){dy[i] = 1;}
        else{dy[i] = -1;}
    }
    for(int i = 0; i < 8; i ++){    //init coordinates
        cur_x[i] = rand()%320;
        cur_y[i] = rand()%240;

        prev_x[0][i] = cur_x[i]; prev_y[0][i] = cur_y[i]; // init prev_x, prev_y
        prev_x[1][i] = cur_x[i]; prev_y[1][i] = cur_y[i];
    }
    for (int i = 0; i < 8; i++){

        int r = rand() % 32; 
        int g = rand() % 64;
        int b = rand() % 32;

        int primary = rand() % 3;
        if (primary == 0) r = 31;      // 红通道满
        else if (primary == 1) g = 63; // 绿通道满
        else b = 31;                   // 蓝通道满

        colors[i] = (r << 11) | (g << 5) | b;
    }
}


int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;

    wait();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                             // clean up two buffers, make sure all black
    init_everything();

    while (1){

        /* Erase any boxes and lines that were drawn in the last iteration */
        for(int i = 0; i < 8; i ++){        //draw rectangle
            draw_rec(prev_x[current_buffer][i], prev_y[current_buffer][i],Width, Height, Black );
        }
        for(int i = 0; i < 7; i ++){        // draw line
            draw_line(prev_x[current_buffer][i],  prev_y[current_buffer][i], prev_x[current_buffer][i+1], prev_y[current_buffer][i+1], Black);
        }
        // code for updating the locations of boxes (not shown)
        for(int i = 0; i < 8; i++){
            if((cur_x[i] + dx[i]) < (0 + (Width >> 1)) || (cur_x[i] + dx[i]) > (320 - (Width >> 1))){dx[i] = -dx[i];}
            if((cur_y[i] + dy[i]) < (0 + (Height >> 1)) || (cur_y[i] + dy[i]) > (240 - (Height >> 1))){dy[i] = -dy[i];}
        }
        for(int i = 0; i < 8; i++){
            cur_x[i] = cur_x[i] + dx[i];
            cur_y[i] = cur_y[i] + dy[i];
        }
        // code for drawing the boxes and lines (not shown)
        for(int i = 0; i < 8; i ++){        //draw rectangle
            draw_rec(cur_x[i], cur_y[i],Width, Height, colors[i] );
        }
        for(int i = 0; i < 7; i ++){        // draw line
            draw_line(cur_x[i], cur_y[i], cur_x[i+1], cur_y[i+1], colors[i]);
        }
        // store current coordinates to prev arr
        for(int i = 0; i < 8; i ++){
            prev_x[current_buffer][i] = cur_x[i];
            prev_y[current_buffer][i] = cur_y[i];
        }
        current_buffer = 1 - current_buffer;

        wait(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}