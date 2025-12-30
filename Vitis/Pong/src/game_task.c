#include "game_state.h"
#include "FreeRTOS.h"
#include "task.h"

/* Game update rate ~60Hz */
#define GAME_TICK_MS 16

void game_task(void *arg)
{
    (void)arg;

    static int win_timer_ms = 0;

    while (1) {

        /* =========================
         * GAME OVER STATE
         * ========================= */
        if (game_over) {

            win_timer_ms += GAME_TICK_MS;

            if (win_timer_ms >= 3000) {   // 3 seconden win screen
                win_timer_ms = 0;
                game_reset();
            }

            vTaskDelay(pdMS_TO_TICKS(GAME_TICK_MS));
            continue;
        }

        /* =========================
         * NORMALE GAME LOGIC
         * ========================= */

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
                ball_x = 50 + PADDLE_WIDTH + 1;   // voorkom vastplakken
            }

            /* Paddle 2 collision */
            if (ball_x + BALL_SIZE >= SCREEN_WIDTH - 50 - PADDLE_WIDTH &&
                ball_y + BALL_SIZE >= paddle2_y &&
                ball_y <= paddle2_y + PADDLE_HEIGHT) {

                ball_vx = -ball_vx;
                ball_x = SCREEN_WIDTH - 50 - PADDLE_WIDTH - BALL_SIZE - 1;
            }

            /* =========================
             * SCORE DETECTIE
             * ========================= */

            if (ball_x < 0) {
                score2++;

                if (score2 >= WIN_SCORE) {
                    game_over = 1;
                    winner = 2;
                }

                /* Reset bal (serve richting speler 1) */
                ball_x = (SCREEN_WIDTH - BALL_SIZE) / 2;
                ball_y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
                ball_vx = BALL_SPEED_X;
                ball_vy = BALL_SPEED_Y;
            }
            else if (ball_x > SCREEN_WIDTH - BALL_SIZE) {
                score1++;

                if (score1 >= WIN_SCORE) {
                    game_over = 1;
                    winner = 1;
                }

                /* Reset bal (serve richting speler 2) */
                ball_x = (SCREEN_WIDTH - BALL_SIZE) / 2;
                ball_y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
                ball_vx = -BALL_SPEED_X;
                ball_vy = BALL_SPEED_Y;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(GAME_TICK_MS));
    }
}
