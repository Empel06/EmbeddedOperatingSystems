#include "xil_io.h"
#include "xil_cache.h"
#include "xparameters.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define FRAME_BUFFER_1 0x10000000
#define FRAME_BUFFER_2 0x118C0000
#define FRAME_BUFFER_3 0x13180000

void vdma_init(void)
{
    xil_printf("Initializing VDMA...\r\n");

    /* Stop VDMA */
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x00000000);

    /* Start VDMA MM2S */
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x00000003);

    /* Frame buffer addresses */
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, FRAME_BUFFER_1);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, FRAME_BUFFER_2);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, FRAME_BUFFER_3);

    /* Stride */
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, SCREEN_WIDTH * 3);
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, SCREEN_WIDTH * 3);

    /* Vertical size */
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, SCREEN_HEIGHT);

    xil_printf("VDMA started\r\n");
}

void framebuffer_init(void)
{
    Xil_DCacheDisable();

    uint8_t *buffers[] = {
        (uint8_t *)FRAME_BUFFER_1,
        (uint8_t *)FRAME_BUFFER_2,
        (uint8_t *)FRAME_BUFFER_3
    };

    for (int i = 0; i < 3; i++) {
        memset(buffers[i], 0x00,
               SCREEN_WIDTH * SCREEN_HEIGHT * 3);
    }

    Xil_DCacheEnable();
}

