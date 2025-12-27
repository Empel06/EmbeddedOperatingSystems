#include "game_state.h"

/* Startpositie midden van het scherm */
volatile int paddle_y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);

void game_state_init(void)
{
    paddle_y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
}
