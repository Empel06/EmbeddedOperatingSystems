/*
 * Pynq-Z2 HDMI 1080p Pong Game
 * FreeRTOS10 + lwIP + UDP + HDMI
 * 2 Spelers, 2 UDP poorten
 */

#include <stdio.h>
#include <string.h>
#include <sleep.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform_config.h"
#include "xil_printf.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "xil_cache.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ==================== CONFIG ==================== */
#define DEFAULT_IP_ADDRESS      "192.168.1.10"
#define DEFAULT_IP_MASK         "255.255.255.0"
#define DEFAULT_GW_ADDRESS      "192.168.1.1"

#define UDP_PORT_PLAYER1        5001
#define UDP_PORT_PLAYER2        5002

#define SCREEN_WIDTH            1920
#define SCREEN_HEIGHT           1080
#define BYTES_PER_PIXEL         3
#define FRAMEBUFFER_SIZE        (SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL)
#define FRAMEBUFFER_BASEADDR    0x01000000

#define PADDLE_HEIGHT           120
#define PADDLE_WIDTH            20
#define BALL_SIZE               15
#define BALL_SPEED              5
#define MAX_SCORE               11

#define THREAD_STACKSIZE        2048

/* ==================== GLOBALS ==================== */
struct netif server_netif;
static int complete_nw_thread = 0;

typedef struct {
    int ball_x, ball_y;
    int ball_vx, ball_vy;
    int paddle1_y;
    int paddle2_y;
    int score1, score2;
    int game_running;
} GameState;

static GameState game_state = {
    .ball_x = SCREEN_WIDTH / 2,
    .ball_y = SCREEN_HEIGHT / 2,
    .ball_vx = BALL_SPEED,
    .ball_vy = BALL_SPEED,
    .paddle1_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2,
    .paddle2_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2,
    .score1 = 0,
    .score2 = 0,
    .game_running = 1
};

static SemaphoreHandle_t xGameMutex = NULL;
static struct udp_pcb *udp_pcb1 = NULL;
static struct udp_pcb *udp_pcb2 = NULL;

/* ==================== FRAMEBUFFER ==================== */

static inline void draw_pixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;

    uint8_t *fb = (uint8_t *)FRAMEBUFFER_BASEADDR;
    uint32_t offset = (y * SCREEN_WIDTH + x) * BYTES_PER_PIXEL;

    fb[offset]     = (color >> 16) & 0xFF;
    fb[offset + 1] = (color >> 8) & 0xFF;
    fb[offset + 2] = color & 0xFF;
}

static void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    if (x + w > SCREEN_WIDTH)
        w = SCREEN_WIDTH - x;
    if (y + h > SCREEN_HEIGHT)
        h = SCREEN_HEIGHT - y;

    uint8_t *fb = (uint8_t *)FRAMEBUFFER_BASEADDR;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    for (uint16_t row = 0; row < h; row++) {
        uint32_t row_offset = ((y + row) * SCREEN_WIDTH + x) * BYTES_PER_PIXEL;
        for (uint16_t col = 0; col < w; col++) {
            uint32_t offset = row_offset + col * BYTES_PER_PIXEL;
            fb[offset]     = r;
            fb[offset + 1] = g;
            fb[offset + 2] = b;
        }
    }
}

static void clear_screen(void)
{
    memset((void *)FRAMEBUFFER_BASEADDR, 0, FRAMEBUFFER_SIZE);
}

/* ==================== GAME LOGIC ==================== */

static void update_game_logic(void)
{
    if (!game_state.game_running)
        return;

    game_state.ball_x += game_state.ball_vx;
    game_state.ball_y += game_state.ball_vy;

    // Botsing met bovenkant en onderkant
    if (game_state.ball_y <= 0 || game_state.ball_y >= SCREEN_HEIGHT - BALL_SIZE) {
        game_state.ball_vy = -game_state.ball_vy;
    }

    // Botsing met paddel 1 (links)
    if (game_state.ball_x <= PADDLE_WIDTH &&
        game_state.ball_y >= game_state.paddle1_y &&
        game_state.ball_y <= game_state.paddle1_y + PADDLE_HEIGHT) {
        game_state.ball_vx = -game_state.ball_vx;
    }

    // Botsing met paddel 2 (rechts)
    if (game_state.ball_x >= SCREEN_WIDTH - PADDLE_WIDTH - BALL_SIZE &&
        game_state.ball_y >= game_state.paddle2_y &&
        game_state.ball_y <= game_state.paddle2_y + PADDLE_HEIGHT) {
        game_state.ball_vx = -game_state.ball_vx;
    }

    // Score speler 2
    if (game_state.ball_x < 0) {
        game_state.score2++;
        game_state.ball_x = SCREEN_WIDTH / 2;
        game_state.ball_y = SCREEN_HEIGHT / 2;
    }

    // Score speler 1
    if (game_state.ball_x > SCREEN_WIDTH) {
        game_state.score1++;
        game_state.ball_x = SCREEN_WIDTH / 2;
        game_state.ball_y = SCREEN_HEIGHT / 2;
    }

    // Einde spel
    if (game_state.score1 >= MAX_SCORE || game_state.score2 >= MAX_SCORE) {
        game_state.game_running = 0;
    }
}

static void render_game(void)
{
    clear_screen();

    // Linker paddel (ROOD)
    draw_rect(10, game_state.paddle1_y, PADDLE_WIDTH, PADDLE_HEIGHT, 0xFF0000);

    // Rechter paddel (BLAUW)
    draw_rect(SCREEN_WIDTH - 10 - PADDLE_WIDTH, game_state.paddle2_y,
              PADDLE_WIDTH, PADDLE_HEIGHT, 0x0000FF);

    // Bal (WIT)
    draw_rect(game_state.ball_x, game_state.ball_y, BALL_SIZE, BALL_SIZE, 0xFFFFFF);

    // Middelijn
    for (int y = 0; y < SCREEN_HEIGHT; y += 40) {
        draw_rect(SCREEN_WIDTH / 2 - 2, y, 4, 20, 0x808080);
    }
}

/* ==================== UDP CALLBACKS ==================== */

static void udp_recv_callback_1(void *arg, struct udp_pcb *pcb,
                                 struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg; (void)pcb; (void)addr; (void)port;

    if (p != NULL && p->len > 0) {
        char buf[32] = {0};
        uint16_t len = (p->len < sizeof(buf) - 1) ? p->len : sizeof(buf) - 1;
        pbuf_copy_partial(p, buf, len, 0);
        buf[len] = '\0';

        int y = atoi(buf);
        if (y >= 0 && y <= SCREEN_HEIGHT - PADDLE_HEIGHT) {
            xSemaphoreTakeFromISR(xGameMutex, NULL);
            game_state.paddle1_y = y;
            xSemaphoreGiveFromISR(xGameMutex, NULL);
        }
        pbuf_free(p);
    }
}

static void udp_recv_callback_2(void *arg, struct udp_pcb *pcb,
                                 struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg; (void)pcb; (void)addr; (void)port;

    if (p != NULL && p->len > 0) {
        char buf[32] = {0};
        uint16_t len = (p->len < sizeof(buf) - 1) ? p->len : sizeof(buf) - 1;
        pbuf_copy_partial(p, buf, len, 0);
        buf[len] = '\0';

        int y = atoi(buf);
        if (y >= 0 && y <= SCREEN_HEIGHT - PADDLE_HEIGHT) {
            xSemaphoreTakeFromISR(xGameMutex, NULL);
            game_state.paddle2_y = y;
            xSemaphoreGiveFromISR(xGameMutex, NULL);
        }
        pbuf_free(p);
    }
}

/* ==================== NETWORK SETUP ==================== */

void network_thread(void *p)
{
    u8_t mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

    xil_printf("[NET] Adding network interface...\r\n");

    if (!xemac_add(&server_netif, NULL, NULL, NULL, mac_ethernet_address,
                   PLATFORM_EMAC_BASEADDR)) {
        xil_printf("[NET] ERROR: Failed to add network interface\r\n");
        return;
    }

    netif_set_default(&server_netif);
    netif_set_up(&server_netif);

    sys_thread_new("xemacif_input_thread",
                   (void(*)(void*))xemacif_input_thread, &server_netif,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    complete_nw_thread = 1;

    xil_printf("[NET] Network interface ready\r\n");
    vTaskDelete(NULL);
}

/* ==================== GAME TASKS ==================== */

void game_logic_thread(void *p)
{
    xil_printf("[GAME] Logic thread started\r\n");
    vTaskDelay(500 / portTICK_RATE_MS);

    while (1) {
        xSemaphoreTake(xGameMutex, portMAX_DELAY);
        update_game_logic();
        xSemaphoreGive(xGameMutex);

        vTaskDelay(16 / portTICK_RATE_MS);  // ~60 FPS
    }
}

void render_thread(void *p)
{
    xil_printf("[RENDER] Render thread started\r\n");
    vTaskDelay(1000 / portTICK_RATE_MS);

    int frame_count = 0;

    while (1) {
        xSemaphoreTake(xGameMutex, portMAX_DELAY);
        render_game();
        xSemaphoreGive(xGameMutex);

        frame_count++;
        if (frame_count % 60 == 0) {
            xil_printf("[RENDER] Frame: %d | Score: %d-%d\r\n",
                       frame_count, game_state.score1, game_state.score2);
        }

        vTaskDelay(16 / portTICK_RATE_MS);  // ~60 FPS
    }
}

/* ==================== MAIN ==================== */

int main_thread()
{
	xil_printf("\r\n");
	xil_printf("================================================\r\n");
	xil_printf("  Pynq-Z2 HDMI 1080p Pong Game - FreeRTOS\r\n");
	xil_printf("  UDP 2 Players | HDMI Output | FreeRTOS10\r\n");
	xil_printf("================================================\r\n\r\n");

    // Initialize lwIP
    lwip_init();

    // Create network thread
    sys_thread_new("nw_thread", network_thread, NULL,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    // Wait for network to be ready
    while (!complete_nw_thread)
        usleep(50);

    // Configure IP
    ip_addr_t ipaddr, netmask, gw;
    inet_aton(DEFAULT_IP_ADDRESS, &ipaddr);
    inet_aton(DEFAULT_IP_MASK, &netmask);
    inet_aton(DEFAULT_GW_ADDRESS, &gw);

    server_netif.ip_addr = ipaddr;
    server_netif.netmask = netmask;
    server_netif.gw = gw;

    xil_printf("[NET] IP: %d.%d.%d.%d\r\n",
               ip4_addr1(&ipaddr), ip4_addr2(&ipaddr),
               ip4_addr3(&ipaddr), ip4_addr4(&ipaddr));

    // Create mutex
    xGameMutex = xSemaphoreCreateMutex();
    if (xGameMutex == NULL) {
        xil_printf("[ERROR] Failed to create mutex\r\n");
        vTaskDelete(NULL);
        return -1;
    }

    // Setup UDP servers
    udp_pcb1 = udp_new();
    if (udp_pcb1) {
        udp_bind(udp_pcb1, IP_ADDR_ANY, UDP_PORT_PLAYER1);
        udp_recv(udp_pcb1, udp_recv_callback_1, NULL);
        xil_printf("[NET] UDP Player 1 on port %d\r\n", UDP_PORT_PLAYER1);
    }

    udp_pcb2 = udp_new();
    if (udp_pcb2) {
        udp_bind(udp_pcb2, IP_ADDR_ANY, UDP_PORT_PLAYER2);
        udp_recv(udp_pcb2, udp_recv_callback_2, NULL);
        xil_printf("[NET] UDP Player 2 on port %d\r\n", UDP_PORT_PLAYER2);
    }

    xil_printf("\r\n[GAME] Starting game threads...\r\n\r\n");

    // Create game threads
    sys_thread_new("game_logic", game_logic_thread, NULL,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO + 1);

    sys_thread_new("render", render_thread, NULL,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO + 1);

    vTaskDelete(NULL);
    return 0;
}

int main()
{
    sys_thread_new("main_thread", (void(*)(void*))main_thread, 0,
                   THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
    vTaskStartScheduler();
    while(1);
    return 0;
}
