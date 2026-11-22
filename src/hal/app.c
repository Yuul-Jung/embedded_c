/* app.c: 응용과 드라이버 (hal.h만 사용한다. 레지스터 주소가 없다) */
#include "hal.h"

static uint32_t led_shadow;          /* 지금까지 출력한 LED 값 */

static void led_set(unsigned n, int on)
{
    if (on) {
        led_shadow |= (1u << n);
    } else {
        led_shadow &= ~(1u << n);
    }
    hal_led_write(led_shadow);       /* 하드웨어 접근은 HAL을 통해서만 한다 */
}

void app_run(void)
{
    led_set(0u, 1);                  /* LED0 켜기 */
    led_set(3u, 1);                  /* LED3 켜기 */
    led_set(0u, 0);                  /* LED0 끄기 */
}
