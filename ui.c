/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Library for game UI

******************/
#include <stdlib.h>
#include <time.h>
#include "display.h"

#define WALL_NUMS 4
#define WALL_WIDTH 12
#define EMPTY_HEIGHT 2

#define BALL_WIDTH 7

#define PAGE(Y_LOC) Y_LOC / 8

int walls[WALL_NUMS][2];  // (x, h) of top wall

// Draw a ball whose center is (x, y), note that y is row, not page.
void draw_ball(int i2c_fd, int x, int y) {
  static const unsigned char ball[] = {
      0x1C, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1C,
  };

  int x0 = x - (BALL_WIDTH / 2);
  int y0 = y - (BALL_WIDTH / 2);
  int x1 = x + (BALL_WIDTH / 2);
  int y1 = y + (BALL_WIDTH / 2);

  if (x0 < 0 || y0 < 0 || x1 >= S_WIDTH || y1 >= S_HEIGHT) {
    printf("invalid argument!\n");
    return;
  }

  if (PAGE(y0) == PAGE(y1)) {
    if (y0 != PAGE(y0) * 8) {
      uint8_t* buf = (uint8_t*)malloc(BALL_WIDTH);
      for (int i = 0; i < BALL_WIDTH; i++) {
        buf[i] = ball[i] << 1;
      }
      update_area(i2c_fd, buf, x0, PAGE(y0), BALL_WIDTH, 1);
      free(buf);
    } else {
      update_area(i2c_fd, ball, x0, PAGE(y0), BALL_WIDTH, 1);
    }
  } else {
    uint8_t* part1_buf = (uint8_t*)malloc(BALL_WIDTH);
    uint8_t* part2_buf = (uint8_t*)malloc(BALL_WIDTH);
    int offset = y0 - PAGE(y0) * 8;
    for (int i = 0; i < BALL_WIDTH; i++) {
      part1_buf[i] = ball[i] << offset;
      part2_buf[i] = ball[i] >> (8 - offset);
    }
    update_area(i2c_fd, part1_buf, x0, PAGE(y0), BALL_WIDTH, 1);
    update_area(i2c_fd, part2_buf, x0, PAGE(y1), BALL_WIDTH, 1);
    free(part1_buf);
    free(part2_buf);
  }
}

// Draw a floor of the map.
void draw_floor(int i2c_fd) {
  draw_line(i2c_fd, 0, S_PAGES - 2, S_WIDTH - 1, S_PAGES - 2, 7);
}

// Draw all walls on the map.
void draw_walls(int i2c_fd) {
  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] < 0 || walls[i][0] + WALL_WIDTH >= S_WIDTH) continue;

    // top wall
    draw_rectangle(i2c_fd, walls[i][0], 0, WALL_WIDTH, walls[i][1]);
    // bottom wall
    draw_rectangle(i2c_fd, walls[i][0], walls[i][1] + EMPTY_HEIGHT, WALL_WIDTH,
                   S_PAGES - (walls[i][1] + EMPTY_HEIGHT) - 1);
  }
}

// Generate a new wall with random height.
void generate_wall(int i2c_fd) {
  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] < 0) {
      walls[i][0] = S_WIDTH;
      walls[i][1] = rand() % 4 + 1;
      break;
    }
  }
}

// Return 1 if a is in range of [r0, r1], or return 0.
int in_range(int a, int r0, int r1) {
  if (a >= r0 && a <= r1)
    return 1;
  else
    return 0;
}

// Check if the ball at (x, y) collides with a wall or the floor.
// If it does, return 1, or if it doesn't return 0.
int check_collision(int i2c_fd, int x, int y) {
  int x0 = x - (BALL_WIDTH / 2);
  int y0 = y - (BALL_WIDTH / 2);
  int x1 = x + (BALL_WIDTH / 2);
  int y1 = y + (BALL_WIDTH / 2);

  // check floor
  if (y1 >= 55) return 1;

  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] < 0 || walls[i][0] + WALL_WIDTH >= S_WIDTH) continue;

    if (in_range(x0, walls[i][0], walls[i][0] + WALL_WIDTH) ||
        in_range(x1, walls[i][0], walls[i][0] + WALL_WIDTH)) {
      // check top wall
      if (y0 <= walls[i][1]) return 1;
      // check bottom wall
      if (y1 >= walls[i][1] + EMPTY_HEIGHT) return 1;
    }
  }
  return 0;
}