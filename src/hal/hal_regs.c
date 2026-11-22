/* hal_regs.c: 레지스터에 쓰는 HAL 구현 (PC에서는 배열이 레지스터 역할) */
#include <stdint.h>
#include <stdio.h>
#include "hal.h"

/* 보드에서는 이 배열 대신 REG32(주소) 매크로로 실제 주소를 접근한다 */
static volatile uint32_t gpio_led_reg;

void hal_led_write(uint32_t mask)
{
    gpio_led_reg = mask;
}

void hal_report(void)
{
    printf("[10-1] 레지스터 구현: LED 레지스터=0x%02X\n", (unsigned)gpio_led_reg);
}
