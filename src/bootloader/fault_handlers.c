#include <stdio.h>
#include <vterm.h>
#include "stm32h7xx.h"

extern uint32_t bootloader_SP;

void UsageFault_Handler() {
    puts("\r\nUsage Fault Exception!");
    uint32_t ufsr = (SCB->CFSR) & SCB_CFSR_USGFAULTSR_Msk;
    printf("UFSR = 0x%02lx\n\r", ufsr);

    NVIC_SystemReset();
}

void BusFault_Handler() {
    puts("\r\nBus Fault Exception!");
    uint32_t bfsr = (SCB->CFSR) & SCB_CFSR_BUSFAULTSR_Msk;
    uint32_t bfaddr = 0;

    if ((bfsr & SCB_CFSR_BFARVALID_Msk) | (bfsr & SCB_CFSR_PRECISERR_Msk))
        bfaddr = (SCB->BFAR);

    printf("BFSR = 0x%02lx\n\r", bfsr);
    printf("BFADDR = 0x%02lx\n\r", bfaddr);

    NVIC_SystemReset();
}

void MemManage_Handler() {
    puts("\r\nMemory Management Fault exception!");
    uint32_t mmfsr = (SCB->CFSR) & SCB_CFSR_MEMFAULTSR_Msk;
    printf("MMFSR = 0x%02lx\n\r", mmfsr);
    if (mmfsr & 0x01) {
        puts(
            "The processor attempted an instruction fetch from a location that "
            "does not permit execution");
    }
    if (mmfsr & 0x80)
        printf("MMFAR = 0x%lx\n\r", (SCB->MMFAR));
    NVIC_SystemReset();
}

void HardFault_Handler() {
    if (bootloader_SP) {
        __set_MSP(bootloader_SP);
        bootloader_SP = 0;
        vterm_init(115200);
        puts("\r\nApplication HardFault exception\r\n");
    } else {
        puts("\r\nBootloader HardFault exception\r\n");
    }
    NVIC_SystemReset();
}