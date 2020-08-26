#include <stdio.h> 
#include <math.h>
#include <stdbool.h>

volatile int pixel_buffer_start; // global variable

void plot_pixel(int x, int y, short int line_color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(void) {
	short int black = 0x0000;
	for(int i = 0; i < 319; i++) {
		for(int j = 0; j < 239; j++) {
			plot_pixel(i, j, black);
		}
	}
}

void draw_line(int x0, int y0, int x1, int y1, short int colour) {
	 
	 bool is_steep = abs(y1-y0) > abs(x1-x0);
	 if(is_steep) {
		int temp0 = x0;
		int temp1 = x1;
		x0 = y0;
		x1 = y1;
		y0 = temp0;
		y1 = temp1;
	 }
	
	 if(x0 > x1) {
		int	tempx = x0;
		int tempy = y0;
		x0 = x1;
		y0 = y1;
		x1 = tempx;
		y1 = tempy;
	 }
	 
	 int deltax = x1 - x0;
	 int deltay = abs(y1 - y0);
	 int error = -(deltax / 2);
	 int y = y0;
	 
	 int y_step;
	 if(y0 < y1) {
	 	y_step = 1;
	 }else {
	 	y_step = -1;
	 }
	
	 for(int x = x0; x < x1; x++) {
	 	if(is_steep) {
	 		plot_pixel(y, x, colour);
		}else {
	 		plot_pixel(x, y, colour);
		}
		 error = error + deltay;
		 if(error >= 0) {
			 y = y + y_step;
			 error = error - deltax;
		 }
	 }
}

void wait_for_vsync() {
	volatile int *pixel_ctrl_ptr = 0xFF203020;
	register int status;
	
	*pixel_ctrl_ptr = 1;
	
	status = *(pixel_ctrl_ptr + 3);
	while((status & 0x01) != 0) {
		status = *(pixel_ctrl_ptr + 3);
	}
}

void draw_box(int x, int y, short int colour) {
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			plot_pixel(x+i, y+j, colour);
		}
	}
}

void draw(int x[8], int y[8], int dx[8], int dy[8], short int colour_box[8]) {
	
	clear_screen();
	/*for(int i = 0; i < 8; i++) {
		draw_box(x[i]-dx[i], y[i]-dy[i], 0x0000);
		draw_line(x[i]+2-dx[i], y[i]+2-dy[i], x[(i+1)%8]+2-dx[i], y[(i+1)%8]+2-dy[i], 0x0000);
	}*/
	
	for(int i = 0; i < 8; i++) {
		
		draw_box(x[i], y[i], colour_box[i]);
		draw_line(x[i]+2, y[i]+2, x[(i+1)%8]+2, y[(i+1)%8]+2, colour_box[i]);
		
		x[i] += dx[i];
		y[i] += dy[i];
		
		//draw_box(x[i]-dx[i], y[i]-dy[i], 0x0000);
		//draw_line(x[i]+2-dx[i], y[i]+2-dy[i], x[(i+1)%8]+2-dx[i], y[(i+1)%8]+2-dy[i], 0x0000);
		
		if(x[i] >= (319+4)) dx[i] = -1;
		else if(x[i] <= 0) dx[i] = 1;
		if(y[i] >= (239+4)) dy[i] = -1;
		else if(y[i] <= 0) dy[i] = 1;
	}
}


volatile int pixel_buffer_start; // global variable

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
	
	int x[8];
	int y[8];
	int dx[8];
	int dy[8];
	short colour_box[8];
	short colour_list[5] = {0xFFFF, 0xF800, 0x07E0, 0x001F, 0xF81F};
	
	for(int i = 0; i < 8; i++) {
		x[i] = rand() % 319;
		y[i] = rand() % 239;
		dx[i] = rand() % 2 * 2 - 1;
		dy[i] = rand() % 2 * 2 -1;
		colour_box[i] = colour_list[rand() % 5];
	}
	
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
       	draw(x, y, dx, dy, colour_box);
		
        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}