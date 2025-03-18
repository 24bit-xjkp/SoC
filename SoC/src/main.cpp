#include "../include/init.hpp"

int main()
{
    using namespace ::SoC::literal;
    ::SoC::system_clock_init();
    ::LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
    ::LL_GPIO_ReadOutputPort(GPIOF);
    ::LL_GPIO_InitTypeDef init{LL_GPIO_PIN_10,
                               LL_GPIO_MODE_OUTPUT,
                               LL_GPIO_SPEED_FREQ_LOW,
                               LL_GPIO_OUTPUT_PUSHPULL,
                               LL_GPIO_PULL_NO,
                               0};
    ::LL_GPIO_Init(GPIOF, &init);

    while(true)
    {

        ::SoC::detail::wait_for(16'000'000_cycle);
        ::LL_GPIO_TogglePin(GPIOF, LL_GPIO_PIN_10);
    }
}
