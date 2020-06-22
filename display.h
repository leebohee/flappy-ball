/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Library for SSD1306

******************/
#include <stdint.h>
#include <stdio.h>


#define S_WIDTH 128
#define S_HEIGHT 64
#define S_PAGES (S_HEIGHT / 8)

void ssd1306_command(int i2c_fd, uint8_t cmd);
void ssd1306_init(int i2c_fd);
void ssd1306_data(int i2c_fd, const uint8_t* data, size_t size);
void update_area(int i2c_fd, const uint8_t* data, int x, int y, int x_len,
                 int y_len);
void update_full(int i2c_fd, uint8_t* data);

void write_char(int i2c_fd, char c, int x, int y);
void write_str(int i2c_fd, char* str, int x, int y);

void draw_line(int i2c_fd, int x0, int y0, int x1, int y1, int offset);
void draw_rectangle(int i2c_fd, int x, int y, int w, int h);
