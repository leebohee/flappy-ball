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

int walls[WALL_NUMS][2];  // (x, h) of top wall

// Draw a ball whose center is (x, y) with radius r.
void draw_ball(int i2c_fd, int x, int y, int r) {}

// Draw a floor of the map.
void draw_floor(int i2c_fd) {
  draw_line(i2c_fd, 0, S_PAGES - 2, S_WIDTH - 1, S_PAGES - 2, 7);
}

// Draw all walls on the map.
void draw_walls(int i2c_fd) {
  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] < 0 || walls[i][0] + WALL_WIDTH >= S_WIDTH) continue;
    draw_rectangle(i2c_fd, walls[i][0], 0, WALL_WIDTH,
                   walls[i][1]);  // upper wall
    draw_rectangle(i2c_fd, walls[i][0], walls[i][1] + EMPTY_HEIGHT, WALL_WIDTH,
                   S_PAGES - (walls[i][1] + EMPTY_HEIGHT) - 1);  // lower wall
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

// Check if the ball collides with a wall.
// If it does, return 1, or if it doesn't return 0.
int check_collision(int i2c_fd) {}