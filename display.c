/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Library for SSD1306

******************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "font.h"

#define S_WIDTH 128
#define S_HEIGHT 64
#define S_PAGES (S_HEIGHT / 8)

void update_full(int i2c_fd, uint8_t* data);

void ssd1306_command(int i2c_fd, uint8_t cmd) {
  uint8_t buffer[2];
  buffer[0] = (0 << 7) | (0 << 6);  // Co = 0, D/C# = 0
  buffer[1] = cmd;

  if (write(i2c_fd, buffer, 2) != 2) {
    printf("i2c write failed!\n");
  }
}

void ssd1306_init(int i2c_fd) {
  uint8_t* data = (uint8_t*)malloc(S_WIDTH * S_PAGES);
  int x, y;
  for (x = 0; x < S_WIDTH; x++) {
    for (y = 0; y < S_PAGES; y++) {
      data[S_WIDTH * y + x] = 0;
    }
  }
  update_full(i2c_fd, data);
  free(data);
  ssd1306_command(i2c_fd, 0xA8);  // Set Mux Ratio
  ssd1306_command(i2c_fd, 0x3f);

  ssd1306_command(i2c_fd, 0xD3);  // Set Display Offset
  ssd1306_command(i2c_fd, 0x00);

  ssd1306_command(i2c_fd, 0x40);  // Set Display Start Line

  ssd1306_command(i2c_fd, 0xA0);  // Set Segment re-map
                                  // 0xA1 for vertical inversion

  ssd1306_command(i2c_fd, 0xC0);  // Set COM Output Scan Direction
                                  // 0xC8 for horizontal inversion

  ssd1306_command(i2c_fd, 0xDA);  // Set COM Pins hardware configuration
  ssd1306_command(i2c_fd, 0x12);  // Manual says 0x2, but 0x12 is required

  ssd1306_command(i2c_fd, 0x81);  // Set Contrast Control
  ssd1306_command(i2c_fd, 0x7F);  // 0:min, 0xFF:max

  ssd1306_command(i2c_fd, 0xA4);  // Disable Entire Display On

  ssd1306_command(i2c_fd, 0xA6);  // Set Normal Display

  ssd1306_command(i2c_fd, 0xD5);  // Set Osc Frequency
  ssd1306_command(i2c_fd, 0x80);

  ssd1306_command(i2c_fd, 0x8D);  // Enable charge pump regulator
  ssd1306_command(i2c_fd, 0x14);

  ssd1306_command(i2c_fd, 0xAF);  // Display ON
}

void ssd1306_data(int i2c_fd, const uint8_t* data, size_t size) {
  uint8_t* buffer = (uint8_t*)malloc(size + 1);

  buffer[0] = (0 << 7) | (1 << 6);
  memcpy(buffer + 1, data, size);  // Co = 0 , D/C# = 1

  if (write(i2c_fd, buffer, size + 1) != size + 1) {
    printf("i2c write failed!\n");
  }

  free(buffer);
}

void update_full(int i2c_fd, uint8_t* data) {
  ssd1306_command(i2c_fd, 0x20);
  ssd1306_command(i2c_fd, 0x0);
  ssd1306_command(i2c_fd, 0x21);
  ssd1306_command(i2c_fd, 0x0);
  ssd1306_command(i2c_fd, S_WIDTH - 1);
  ssd1306_command(i2c_fd, 0x22);
  ssd1306_command(i2c_fd, 0x0);
  ssd1306_command(i2c_fd, S_PAGES - 1);
  ssd1306_data(i2c_fd, data, S_WIDTH * S_PAGES);
}

void update_area(int i2c_fd, const uint8_t* data, int x, int y, int x_len,
                 int y_len) {
  ssd1306_command(i2c_fd, 0x20);  // addressing mode
  ssd1306_command(i2c_fd, 0x0);   // horizontal addressing mode

  ssd1306_command(i2c_fd, 0x21);  // set column start/end address
  ssd1306_command(i2c_fd, x);
  ssd1306_command(i2c_fd, x + x_len - 1);

  ssd1306_command(i2c_fd, 0x22);  // set page start/end address
  ssd1306_command(i2c_fd, y);
  ssd1306_command(i2c_fd, y + y_len - 1);

  ssd1306_data(i2c_fd, data, x_len * y_len);
}

#define FONT_WIDTH 6
#define FONT_HEIGHT 1

void write_char(int i2c_fd, char c, int x, int y) {
  if (c < ' ') c = ' ';
  update_area(i2c_fd, font[c - ' '], x, y, FONT_WIDTH, FONT_HEIGHT);
}

void write_str(int i2c_fd, char* str, int x, int y) {
  char c;
  while (c = *str++) {
    write_char(i2c_fd, c, x, y);
    x += FONT_WIDTH;
  }
}

// Draw a line from (x0, y0) to (x1, y1). It should be vertical/horizontal line.
// The offset in the page is given as an argument.
void draw_line(int i2c_fd, int x0, int y0, int x1, int y1, int offset) {
  int len;
  uint8_t* buffer;

  if (x0 == x1) {  // vertical line
    len = y1 - y0 + 1;
    buffer = (uint8_t*)malloc(len);
    ssd1306_command(i2c_fd, 0x20);  // addressing mode
    ssd1306_command(i2c_fd, 0x01);  // vertical addressing mode

    ssd1306_command(i2c_fd, 0x21);  // set column start/end address
    ssd1306_command(i2c_fd, x0);
    ssd1306_command(i2c_fd, x1);

    ssd1306_command(i2c_fd, 0x22);  // set page start/end address
    ssd1306_command(i2c_fd, y0);
    ssd1306_command(i2c_fd, y1);

    for (int i = 0; i < len - 1; i++) {
      buffer[i] = 0xFF;
    }
    buffer[len - 1] = (1 << (offset + 1)) - 1;

    ssd1306_data(i2c_fd, buffer, len);
  } else if (y0 == y1) {  // horizontal line
    len = x1 - x0 + 1;
    buffer = (uint8_t*)malloc(len);
    ssd1306_command(i2c_fd, 0x20);  // addressing mode
    ssd1306_command(i2c_fd, 0x0);   // horizontal addressing mode

    ssd1306_command(i2c_fd, 0x21);  // set column start/end address
    ssd1306_command(i2c_fd, x0);
    ssd1306_command(i2c_fd, x1);

    ssd1306_command(i2c_fd, 0x22);  // set page start/end address
    ssd1306_command(i2c_fd, y0);
    ssd1306_command(i2c_fd, y1);

    for (int i = 0; i < len; i++) {
      buffer[i] = (1 << offset);
    }

    ssd1306_data(i2c_fd, buffer, len);
  } else {
    printf("invalid argument!\n");
    return;
  }
  free(buffer);
}

// Draw a rectangle, whose left-upper point is at (x, y),
// with given width and height.
void draw_rectangle(int i2c_fd, int x, int y, int w, int h) {
  draw_line(i2c_fd, x, y, x + w - 1, y, 0);              // upper-side
  draw_line(i2c_fd, x, y + h, x + w - 1, y + h, 0);      // lower-side
  draw_line(i2c_fd, x, y, x, y + h, 0);                  // left-side
  draw_line(i2c_fd, x + w - 1, y, x + w - 1, y + h, 0);  // right-side
}