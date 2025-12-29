#include "game_state.h"
#include "FreeRTOS.h"
#include "task.h"

/* Game update rate ~60Hz */
#define GAME_TICK_MS 16

void game_task(void *arg)
{
    (void)arg;

    while (1) {

        /* Start pas als beide spelers verbonden zijn */
        if (player1_connected && player2_connected) {

            /* Beweeg bal */
            ball_x += ball_vx;
            ball_y += ball_vy;

            /* Bounce boven / onder */
            if (ball_y <= 0 || ball_y >= (SCREEN_HEIGHT - BALL_SIZE)) {
                ball_vy = -ball_vy;
            }

            /* Paddle 1 collision */
            if (ball_x <= 50 + PADDLE_WIDTH &&
                ball_y + BALL_SIZE >= paddle1_y &&
                ball_y <= paddle1_y + PADDLE_HEIGHT) {

                ball_vx = -ball_vx;
            }

            /* Paddle 2 collision */
            if (ball_x + BALL_SIZE >= SCREEN_WIDTH - 50 - PADDLE_WIDTH &&
                ball_y + BALL_SIZE >= paddle2_y &&
                ball_y <= paddle2_y + PADDLE_HEIGHT) {

                ball_vx = -ball_vx;
            }

            /* Links of rechts uit beeld → reset bal */
            if (ball_x < 0 || ball_x > SCREEN_WIDTH) {
                ball_x = (SCREEN_WIDTH - BALL_SIZE) / 2;
                ball_y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
                ball_vx = BALL_SPEED_X;
                ball_vy = BALL_SPEED_Y;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(GAME_TICK_MS));
    }
}
