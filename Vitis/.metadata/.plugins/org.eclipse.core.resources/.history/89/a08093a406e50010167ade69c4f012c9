#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>

/* Scherm */
#define SCREEN_WIDTH   1920
#define SCREEN_HEIGHT  1080

/* Paddle */
#define PADDLE_WIDTH   30
#define PADDLE_HEIGHT  200
#define PADDLE_SPEED   10

/* Ball */
#define BALL_SIZE      20
#define BALL_SPEED_X   2
#define BALL_SPEED_Y   2

/* Paddle posities */
extern volatile int paddle1_y;
extern volatile int paddle2_y;

/* Ball */
extern volatile int ball_x;
extern volatile int ball_y;
extern volatile int ball_vx;
extern volatile int ball_vy;

/* Players connected */
extern volatile int player1_connected;
extern volatile int player2_connected;

void game_state_init(void);
void game_task(void *arg);

#endif
