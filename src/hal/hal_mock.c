/* hal_mock.c: 호출을 기록하는 시험용 HAL 구현 (하드웨어 없이 응용을 시험한다) */
#include <stdint.h>
#include <stdio.h>
#include "hal.h"

#define LOG_MAX  8u

static uint32_t log_val[LOG_MAX];
static unsigned log_n;

void hal_led_write(uint32_t mask)
{
    if (log_n < LOG_MAX) {
        log_val[log_n++] = mask;
    }
}

void hal_report(void)
{
    printf("[10-2] 시험용 구현: hal_led_write 호출=%u회, 값:", log_n);
    for (unsigned i = 0; i < log_n; i++) {
        printf(" 0x%02X", (unsigned)log_val[i]);
    }
    printf("\n");
}
