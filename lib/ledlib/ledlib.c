#include "ledlib.h"
#include <stm32h7xx.h>

#define LED_GREEN_PIN  (1U << 0)   // PB0
#define LED_YELLOW_PIN (1U << 1)   // PE1
#define LED_RED_PIN    (1U << 14)  // PB14

#define TOGGLE_BIT(REG, BIT)     ((REG) ^= (BIT))

static const uint32_t led_pins[] = { LED_GREEN_PIN, LED_YELLOW_PIN, LED_RED_PIN };
static GPIO_TypeDef* const led_ports[] = { GPIOB, GPIOE, GPIOB };
static const led_t led_supported[] = { led_green, led_yellow, led_red};
static const uint32_t led_modes_msk[] = { GPIO_MODER_MODE0_Msk, GPIO_MODER_MODE1_Msk, GPIO_MODER_MODE14_Msk};
static const uint32_t led_modes[] = { GPIO_MODER_MODE0_0, GPIO_MODER_MODE1_0, GPIO_MODER_MODE14_0};

void led_enable(led_t led)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOEEN;

    for (int i = 0; i < LEDS_COUNT; i++) {
        if (led & led_supported[i]) {
            led_ports[i]->MODER &= ~led_modes_msk[i]; // Выставить 00
            led_ports[i]->MODER |= led_modes[i]; // Выставить 01 (output mode)
        }
    }
}
void led_toggle(led_t led)
{
    for (int i = 0; i < LEDS_COUNT; i++)
        if (led & led_supported[i])
            TOGGLE_BIT(led_ports[i]->ODR, led_pins[i]); // ; "исключающее или" c единицей меняет 0->1 и 1->0
}
void led_on(led_t led)
{
    for (int i = 0; i < LEDS_COUNT; i++)
        if (led & led_supported[i])
            TOGGLE_BIT(led_ports[i]->BSRR, led_pins[i]); // BSRR SET (first 16 bits)
}
void led_off(led_t led)
{
    for (int i = 0; i < LEDS_COUNT; i++)
        if (led & led_supported[i])
            TOGGLE_BIT(led_ports[i]->BSRR, led_pins[i] << 16); // BSRR SET (last 16 bits)

}
void led_disable(led_t led)
{
    for (int i = 0; i < LEDS_COUNT; i++) {
        if (led & led_supported[i]) {
            led_ports[i]->MODER |= led_modes_msk[i]; // Выставить 11 (default)
        }
    }
    RCC->AHB1ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOEEN;
}