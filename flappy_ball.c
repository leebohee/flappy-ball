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
#include <unistd.h>
#include "display.h"
#include "ui.h"

#define SSD1306_I2C_DEV 0x3C
#define BALL_RADIUS 2
#define GEN_PERIOD 20

int main() {
  int counter = 0;
  int ball_x = 64, ball_y = 35;

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
  for (int i = 0; i < RANK_NUM; i++) ranks[i] = -1;
  home_page(i2c_fd);

  close(i2c_fd);
  return 0;
}
