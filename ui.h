/*****************

2016311477 Lee Bohee
2016312506 Lee Seongtae

Library for game UI

******************/
#define WALL_NUMS 4
#define WALL_WIDTH 12
#define EMPTY_HEIGHT 2

int walls[WALL_NUMS][2];  // (x, h) of top wall

void draw_ball(int i2c_fd, int x, int y);
void draw_floor(int i2c_fd);
void draw_walls(int i2c_fd);
void generate_wall(int i2c_fd);
int check_collision(int i2c_fd, int x, int y);