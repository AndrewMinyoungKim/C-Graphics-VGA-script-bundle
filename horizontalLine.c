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

void wait_for_vsync() {
	volatile int *pixel_ctrl_ptr = 0xFF203020;
	register int status;
	
	*pixel_ctrl_ptr = 1;
	
	status = *(pixel_ctrl_ptr + 3);
	while((status & 0x01) != 0) {
		status = *(pixel_ctrl_ptr + 3);
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


int main(void) {
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
	
	const int x0 = 100;
	const int x1 = 219;
	int y = 0;
	bool move_down = true;
	
    clear_screen();
	while(true) {
		
		if(y >= 239) {
			move_down = false;
		}else if(y <= 0) {
			move_down = true;
		}
		
		if(move_down) {
			draw_line(x0, y, x1, y, 0x0000);
			y = y+1;
			draw_line(x0, y, x1, y, 0xF81F);
		}else {
			draw_line(x0, y, x1, y, 0x0000);
			y = y-1;
			draw_line(x0, y, x1, y, 0xF81F);
		}
		
		wait_for_vsync();
	}
}

// code not shown for clear_screen() and draw_line() subroutines