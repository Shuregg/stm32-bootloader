#include <assert.h>
#include <stdio.h>
#include <stm32h7xx.h>
#include <vterm.h>
#include "fbank2lib.h"

#define APP_SRAM_OFFSET    0x24000000
#define APP_FLASH_SLOT1_OFFSET 0x08070000
#define AXISRAM_END 0x24080000

#define FIRMWARE_SIZE 6544

#define NUM_COMMANDS 6
#if NUM_COMMANDS > 9
#error NUM_COMMANDS must be less then 10 or change read_handler_index()
#endif

_Bool check_valid_BootSRAM();
void load_by_address(uint32_t addr);

__attribute__((optimize("-O0"))) // директива отключает оптимизацию кода компилятором для этой функции 
static void delay(int ms)
{
    volatile int counter = SystemCoreClock / 1000 / 6 * ms;
    while (counter > 0)
        counter -= 1;
}

extern void HardFault_Handler();
uint32_t bootloader_SP = 0;
static const char* gc_help_msg =
    u8"\n\r┌────────────┬──────────────┬───────┬─────────────┐"
    u8"\n\r│ 1:BootSRAM │ 2:TestFLASH2 │ 3:FB2 │ 4:BootFlash │"
    u8"\n\r└────────────┴──────────────┴───────┴─────────────┘"
    u8"\n\r Выбор [1-4] > ";
static void do_BootSRAM();
static void do_TestFlash2();
static void flashbank2_manage();
static void do_User();
typedef void (*handler_func_t)();
handler_func_t handlers[NUM_COMMANDS] = {do_BootSRAM, do_TestFlash2, flashbank2_manage, do_User};
uint8_t read_handler_index() {
    while (vterm_keypressed() != 0)
        ;
    char str[2];
    int sz = vterm_gets(str, sizeof(str), 1);
    if (sz < 1)
        return UINT8_MAX;
    return str[0] >= '1' ? str[0] - '1' : UINT8_MAX;
}
void enable_fault_handlers() {
    // Включить генерацию исключений для UsageFault; cм. PM0253, п. 4.3.7 на стр. 200
    // SCB->CCR ....
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
    // Разрешить генерацию исключений; см. PM0253, п. 4.3.9 на
    // стр. 204 SCB->SHCSR ...
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk;
}
void prepare_bootloader(){
    for (;;) {
        printf("\r\n System clock is %ld MHz %s", SystemCoreClock / 1000000, gc_help_msg);
        uint8_t handler_index = read_handler_index();
        if (handler_index < NUM_COMMANDS) {
            handlers[handler_index]();
        }
    }
}

int main() {
    vterm_init(115200);
    enable_fault_handlers();
    fb2_disable_wr_protection();

    if (check_valid_BootSRAM())
    {
        puts("\r\nFirmware in AXISRAM is valid");
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
        GPIOC->MODER &= ~GPIO_MODER_MODER13_Msk;
        puts("\r\nЗагрузка системы через...");
        for (uint16_t i = 3; i > 0; i--) {
            printf("\r\n%d", i);
            delay(1000);
            if (GPIOC->IDR & (1 << 13)) {
                prepare_bootloader();
                return 0;
            }
        }

        do_BootSRAM();
    }
    else
    {
        puts("\r\nNo valid firmware in AXISRAM!");
        prepare_bootloader();
    }

    return 0;
}
/***************************** Обработчики команд ************************************/
void do_BootSRAM() {
    printf("\nJumping to SRAM app at 0x%x....\n", APP_SRAM_OFFSET);

    load_by_address(APP_SRAM_OFFSET);
}

void do_TestFlash2() {
    fb2_write_word('C', 0);

    printf("\r\nRead from flash: 0x%lx", fb2_read_word(0));
}

void flashbank2_manage() {

    char firmware [FIRMWARE_SIZE];
    vterm_gets(firmware, FIRMWARE_SIZE, 1);

    // printf("\r\nRead from flash: %x", fb2_read_byte(0));

}
void do_User() {
    load_by_address(APP_FLASH_SLOT1_OFFSET);
}

_Bool check_valid_BootSRAM() {
    uint32_t sp_addr = *(uint32_t*)D1_AXISRAM_BASE;
    uint32_t rst_hdl_addr = *(uint32_t*)(D1_AXISRAM_BASE + 4);
    printf("\r\nSP: %08lx\tRST: %08lx", sp_addr, rst_hdl_addr);

    return (sp_addr > D1_AXISRAM_BASE) && (sp_addr <= AXISRAM_END) && (rst_hdl_addr >= D1_AXISRAM_BASE) && (rst_hdl_addr < AXISRAM_END);
}

void load_by_address(uint32_t addr){
    // 1) Определить ТВП приложения, адреса начала стека и точки входа приложения
    const uint32_t* app_IV = (uint32_t*)(addr);
    uint32_t app_end_stack = (*((uint32_t*)(app_IV)));
    void* app_entry = (void*)(*((uint32_t*)(addr + 4)));
    // Доп.1.) Признак того, что был запуск приложения bootloader_SP != 0
    bootloader_SP = __get_MSP();

    if (app_end_stack < 0x20000000 && app_end_stack > 0x20020000) {
        printf("\n[FATAL] Invalid SP address: %08lx....\n", app_end_stack);
        return;
    }

    // 2) Отключить все прерывания
    __disable_irq();
    // 3) заменить текущий адрес стека на начальный адрес стека приложения
    __set_MSP(app_end_stack);
    // 4) задать новый адрес таблицы векторов прерываний
    SCB->VTOR = (uint32_t)app_IV;
    // Доп.2) Заменили обработчика HardFault в ТВП на собственный
    NVIC_SetVector(HardFault_IRQn, (uint32_t)HardFault_Handler);
    // Инвалидация кеша инстуркций у ядра Cortex-M7
    SCB_InvalidateICache();
    // 5) Безусловный переход на точку входу
    __ASM volatile("bx %0" ::"r"(app_entry));
}