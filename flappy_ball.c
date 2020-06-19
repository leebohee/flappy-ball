/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Flappy Ball: A video console game

******************/
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include "display.h"
#include "ui.h"

#define SSD1306_I2C_DEV 0x3C
#define BALL_RADIUS 2
#define GEN_PERIOD 20

int main() {
  int counter = 0;
  int ball_x = 64, ball_y = 35;
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));

  int i2c_fd = open("/dev/i2c-1", O_RDWR);
  if (i2c_fd < 0) {
    printf("error opening device\n");
    return -1;
  }
  if (ioctl(i2c_fd, I2C_SLAVE, SSD1306_I2C_DEV) < 0) {
    printf("error setting i2c slave address\n");
    return -1;
  }

  ssd1306_init(i2c_fd);
  for (int i = 0; i < WALL_NUMS; i++) {
    walls[i][0] = -1;
  }
  srand((unsigned int)time(NULL));

  home_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  game_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  rank_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  reset_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  game_over_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  game_result_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  game_pause_page(i2c_fd);
  usleep(1000 * 1000);
  update_full(i2c_fd, clear);

  more_page(i2c_fd);
  while (1)
    ;

  while (1) {
    // clear display
    update_full(i2c_fd, clear);

    // check collision
    if (check_collision(i2c_fd, ball_x, ball_y)) {
      printf("GAME OVER!\n");
      break;
    }

    // generate a new wall periodically
    counter++;
    if (counter >= GEN_PERIOD) {
      generate_wall(i2c_fd);
      counter = 0;
    }

    // update ball status depending on switch input
    if (counter % 2)
      ball_y -= 1;
    else
      ball_y += 1;

    // update walls' position
    for (int i = 0; i < WALL_NUMS; i++) {
      if (walls[i][0] < 0) continue;
      walls[i][0] -= 2;
    }

    // draw map
    write_str(i2c_fd, "SCORE : ", 0, S_PAGES - 1);
    draw_floor(i2c_fd);
    draw_ball(i2c_fd, ball_x, ball_y);
    draw_walls(i2c_fd);
  }
  close(i2c_fd);
  return 0;
}