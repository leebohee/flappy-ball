/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Library for game UI

******************/
#include "ui.h"
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "display.h"
#include "gpio.h"

#define WALL_NUMS 4
#define WALL_WIDTH 12
#define EMPTY_HEIGHT 2

#define RANK_NUM 5

#define BALL_WIDTH 7

#define PAGE(Y_LOC) Y_LOC / 8

#define GEN_PERIOD 20

int walls[WALL_NUMS][2];  // (x, h) of top wall
int ball_x, ball_y;
int ranks[RANK_NUM];
int score;

// Draw a ball whose center is (x, y), note that y is row, not page.
void draw_ball(int i2c_fd, int x, int y) {
  static const unsigned char ball[] = {
      0x1C, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1C,
  };
  uint8_t* init_buf1 = (uint8_t*)malloc(BALL_WIDTH);
  uint8_t* init_buf2 = (uint8_t*)malloc(BALL_WIDTH);
  for (int i = 0; i < BALL_WIDTH; i++) {
    init_buf1[i] = 0;
    init_buf2[i] = 0;
  }
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
    update_area(i2c_fd, init_buf1, x0, PAGE(y0) - 1, BALL_WIDTH, 1);
    update_area(i2c_fd, init_buf1, x0, PAGE(y0) + 1, BALL_WIDTH, 1);

  } else {
    uint8_t* part1_buf = (uint8_t*)malloc(BALL_WIDTH);
    uint8_t* part2_buf = (uint8_t*)malloc(BALL_WIDTH);
    int offset = y0 - PAGE(y0) * 8;
    for (int i = 0; i < BALL_WIDTH; i++) {
      part1_buf[i] = ball[i] << offset;
      part2_buf[i] = ball[i] >> (8 - offset);
    }
    update_area(i2c_fd, init_buf1, x0, PAGE(y0) - 1, BALL_WIDTH, 1);
    update_area(i2c_fd, part1_buf, x0, PAGE(y0), BALL_WIDTH, 1);
    update_area(i2c_fd, part2_buf, x0, PAGE(y1), BALL_WIDTH, 1);
    update_area(i2c_fd, init_buf1, x0, PAGE(y1) + 1, BALL_WIDTH, 1);
    free(part1_buf);
    free(part2_buf);
  }
  free(init_buf1);
  free(init_buf2);
}

// Draw a floor of the map.
void draw_floor(int i2c_fd) {
  draw_line(i2c_fd, 0, S_PAGES - 1, S_WIDTH - 1, S_PAGES - 1, 0);
}

// Draw all walls on the map.
void draw_walls(int i2c_fd) {
  uint8_t* clear =
      (uint8_t*)calloc((WALL_WIDTH + 2) * (S_HEIGHT - 1), sizeof(uint8_t));
  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] == -2) {
      update_area(i2c_fd, clear, 0, 0, WALL_WIDTH + 2, S_HEIGHT - 1);
      walls[i][0] = -1;
    }
    if (walls[i][0] < 0 || walls[i][0] + WALL_WIDTH >= S_WIDTH) continue;

    // clear wall
    update_area(i2c_fd, clear, walls[i][0] + 2, 0, 1, S_HEIGHT - 1);
    update_area(i2c_fd, clear, walls[i][0] + WALL_WIDTH, 0, 2, S_HEIGHT - 1);

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

// Check if the ball at (x, y) collides with a wall, floor, or ceiling.
// If it does, return 1, or if it doesn"t return 0.
int check_collision(int i2c_fd, int x, int y) {
  int x0 = x - (BALL_WIDTH / 2);
  int y0 = y - (BALL_WIDTH / 2);
  int x1 = x + (BALL_WIDTH / 2);
  int y1 = y + (BALL_WIDTH / 2);

  // check floor
  if (y1 >= 55) return 1;

  // check ceiling
  if (y0 <= 0) return 1;

  for (int i = 0; i < WALL_NUMS; i++) {
    if (walls[i][0] < 0 || walls[i][0] + WALL_WIDTH >= S_WIDTH) continue;

    if (in_range(x0, walls[i][0], walls[i][0] + WALL_WIDTH) ||
        in_range(x1, walls[i][0], walls[i][0] + WALL_WIDTH)) {
      // check top wall
      if (y0 <= walls[i][1] * 8) return 1;
      // check bottom wall
      if (y1 >= (walls[i][1] + EMPTY_HEIGHT) * 8) return 1;
    }
  }
  return 0;
}

void init_game(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  score = 0;
  update_full(i2c_fd, clear);
  for (int i = 0; i < WALL_NUMS; i++) {
    walls[i][0] = -1;
  }
  srand((unsigned int)time(NULL));
  ball_x = 64;
  ball_y = 35;
}

void print_home_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  draw_rectangle(i2c_fd, 0, 0, S_WIDTH, S_PAGES);
  write_str(i2c_fd, "FLAPPY BALL", 35, 2, 0);

  write_str(i2c_fd, "START", 5, S_PAGES - 2, 0);
  write_str(i2c_fd, "MORE", 54, S_PAGES - 2, 0);
  write_str(i2c_fd, "RANK", 100, S_PAGES - 2, 0);
}

void home_page(int i2c_fd) {
  init_gpios();

  print_home_page(i2c_fd);

  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 17, &gpio_17_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    if (gpio_4_value == 0) {
      init_game(i2c_fd);
      game_page(i2c_fd);
      print_home_page(i2c_fd);
    } else if (gpio_17_value == 0) {
      more_page(i2c_fd);
      print_home_page(i2c_fd);
    } else if (gpio_27_value == 0) {
      rank_page(i2c_fd);
      print_home_page(i2c_fd);
    }
  }
}

void game_page(int i2c_fd) {
  int counter = 0, ret;
  int flag = -1;
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  init_game(i2c_fd);

  while (1) {
    // clear display
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 17, &gpio_17_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    // update ball status depending on switch input
    if (gpio_4_value == 0) {
      ball_y -= 3;
    } else if (gpio_17_value == 0) {
      ball_y += 3;
    } else if (gpio_27_value == 0) {
      flag = game_pause_page(i2c_fd);
      if (flag == 1) {  // resume;
        update_full(i2c_fd, clear);
        flag = -1;
        continue;
      } else if (flag == 2) {  // restart
        flag = -1;
        init_game(i2c_fd);
        continue;
      } else if (flag == 3) {
        flag = -1;
        break;
      }
    }

    // check collision
    if (check_collision(i2c_fd, ball_x, ball_y)) {
      game_over_page(i2c_fd);
      usleep(1000 * 1000);
      ret = game_result_page(i2c_fd);
      if (ret == 4) {
        init_game(i2c_fd);
        continue;
      } else if (ret == 17) {
        break;
      }
    } else {
      score++;
    }

    // generate a new wall periodically
    counter++;
    if (counter >= GEN_PERIOD) {
      generate_wall(i2c_fd);
      counter = 0;
    }

    // update walls' position
    for (int i = 0; i < WALL_NUMS; i++) {
      if (walls[i][0] < 0) continue;
      walls[i][0] -= 2;
    }

    // draw map
    draw_floor(i2c_fd);
    char _score[15];
    sprintf(_score, "SCORE : %d", score);
    write_str(i2c_fd, _score, 0, S_PAGES - 1, 1);
    draw_ball(i2c_fd, ball_x, ball_y);
    draw_walls(i2c_fd);
  }
}

void print_rank_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);

  write_str(i2c_fd, "NO.", 24, 0, 0);
  write_str(i2c_fd, "SCORE", 80, 0, 0);

  // print ranks
  uint8_t no[2], scores[10];
  for (int i = 0; i < RANK_NUM; i++) {
    if (ranks[i] < 0) break;
    sprintf(no, "%d", i + 1);
    sprintf(scores, "%d", ranks[i]);
    write_str(i2c_fd, no, 24, i + 1, 0);
    write_str(i2c_fd, scores, 80, i + 1, 0);
  }

  write_str(i2c_fd, "HOME", 5, S_PAGES - 1, 0);
  write_str(i2c_fd, "RESET", 95, S_PAGES - 1, 0);
}

void rank_page(int i2c_fd) {
  print_rank_page(i2c_fd);
  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    if (gpio_4_value == 0) {
      break;
    } else if (gpio_27_value == 0) {
      reset_page(i2c_fd);
      print_rank_page(i2c_fd);
    }
  }
}
void reset_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  draw_rectangle(i2c_fd, 0, 0, S_WIDTH, S_PAGES);
  write_str(i2c_fd, "Do you", 48, 2, 0);
  write_str(i2c_fd, "want to reset?", 24, 3, 0);

  write_str(i2c_fd, "YES", 5, S_PAGES - 2, 0);
  write_str(i2c_fd, "NO", 100, S_PAGES - 2, 0);

  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    if (gpio_4_value == 0) {
      for (int i = 0; i < RANK_NUM; i++) ranks[i] = -1;
      break;
    } else if (gpio_27_value == 0) {
      break;
    }
  }
}

void game_over_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  draw_rectangle(i2c_fd, 0, 0, S_WIDTH, S_PAGES);
  write_str(i2c_fd, "GAME OVER", 37, 2, 0);
  write_str(i2c_fd, "T  T", 52, 4, 0);
  write_str(i2c_fd, " __ ", 52, 5, 0);
  return;
}

int game_result_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  draw_rectangle(i2c_fd, 0, 0, S_WIDTH, S_PAGES);

  char _score[15];
  char _rank[9];
  int i, j;
  for (i = 0; i < RANK_NUM; i++) {
    if (score > ranks[i]) {
      for (j = RANK_NUM - 1; j > i; j--) {
        ranks[j] = ranks[j - 1];
      }
      ranks[i] = score;
      break;
    }
  }
  sprintf(_score, "SCORE : %d", score);
  sprintf(_rank, "RANK : %d", i + 1);
  write_str(i2c_fd, _score, 20, 2, 0);
  write_str(i2c_fd, _rank, 20, 4, 0);

  write_str(i2c_fd, "RESTART", 5, S_PAGES - 2, 0);
  write_str(i2c_fd, "HOME", 54, S_PAGES - 2, 0);
  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 17, &gpio_17_value);

    if (gpio_4_value == 0) {  // restart
      return 4;
    } else if (gpio_17_value == 0) {  // home
      return 17;
    }
  }
}

int game_pause_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  draw_rectangle(i2c_fd, 0, 0, S_WIDTH, S_PAGES);
  char _score[15];
  sprintf(_score, "SCORE : %d", score);
  write_str(i2c_fd, _score, 20, 2, 0);

  write_str(i2c_fd, "RESUME", 5, S_PAGES - 2, 0);
  write_str(i2c_fd, "RESTART", 50, S_PAGES - 2, 0);
  write_str(i2c_fd, "HOME", 100, S_PAGES - 2, 0);
  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 17, &gpio_17_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    if (gpio_4_value == 0) {  // resume
      return 1;
    } else if (gpio_17_value == 0) {  // restart
      return 2;
    } else if (gpio_27_value == 0) {  /// home
      return 3;
    }
  }
}

int more_page(int i2c_fd) {
  uint8_t* clear = (uint8_t*)calloc(S_WIDTH * S_PAGES, sizeof(uint8_t));
  update_full(i2c_fd, clear);
  int flag = 0;
  static char info[37][22] = {"This game is that you",
                              " make the ball to be ",
                              "survived as long as p",
                              "ossible without colli",
                              "ding with obstacles. ",
                              "When the leftmost but",
                              "ton is pressed, the b",
                              "all bounces up then f",
                              "alls down. The obstac",
                              "le has a hole, and yo",
                              "u should control the ",
                              "movement of the ball ",
                              "properly so that the ",
                              "ball can pass through",
                              " the hole. The game e",
                              "nds:\n  1) When the ba",
                              "ll falls down and hit",
                              "s the floor\n  2) When",
                              " the ball collides an",
                              "d obstacle(=wall)\n  3",
                              ") When the ball bounc",
                              "es up and hits the ce",
                              "iling\n\n\nIn addition t",
                              "o balls and obstacles",
                              ", items appear random",
                              "ly on the map. You ca",
                              "n eat the item by mov",
                              "ing the ball and use ",
                              "it with the rightmost",
                              " button. If you use a",
                              "n item, the ball beco",
                              "mes invincible for a ",
                              "short time (for 2-3 s",
                              "econds) so that it wi",
                              "ll not die even if it",
                              " collides with obstab",
                              "les."};

  write_str(i2c_fd, "FLAPPY BALL", 35, 0, 0);

  // print explanation
  for (int i = flag; i < flag + 5; i++) {
    write_str(i2c_fd, info[i], i - flag, i + 2 - flag, 0);
  }

  write_str(i2c_fd, "HOME", 5, S_PAGES - 1, 0);
  write_str(i2c_fd, "NEXT", 100, S_PAGES - 1, 0);
  while (1) {
    get_gpio_input_value(gpio_ctr, 4, &gpio_4_value);
    get_gpio_input_value(gpio_ctr, 27, &gpio_27_value);

    if (gpio_4_value == 0) {
      return 4;
    } else if (gpio_27_value == 0) {
      if (flag == 35 || flag != 30) {
        if (flag == 35) {
          flag = 0;
        } else {
          flag += 5;
        }
        for (int i = flag; i < flag + 5; i++) {
          write_str(i2c_fd, info[i], i - flag, i + 2 - flag, 0);
        }
      } else {
        flag += 5;
        for (int i = flag; i < flag + 2; i++) {
          write_str(i2c_fd, info[i], i - flag, i + 2 - flag, 0);
        }
      }
      continue;
    }
  }
}