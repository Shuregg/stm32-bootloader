#include <stm32h7xx.h> // основной заголовочный файл CMSIS для МК серии H7
#include <stdio.h>
#include <vterm.h>
#include <ledlib.h>

__attribute__((optimize("-O0"))) // директива отключает оптимизацию кода компилятором для этой функции 
static void delay(int ms)
{
    volatile int counter = SystemCoreClock / 1000 / 6 * ms;
    while (counter > 0)
        counter -= 1;
}
int main()
{
    led_enable(led_all); // инициализация выввода светодиода
    vterm_init(115200);
    while (1)
    {
        static int counter = 0;
        printf("\r%s %d %s", "Светодиод был переключен", counter++, "раз(а)");

        led_toggle(led_green); // переключение «вкл <-> откл»
        delay(100);     // пауза между переключениями
        led_toggle(led_green); // переключение «вкл <-> откл»
    };

}