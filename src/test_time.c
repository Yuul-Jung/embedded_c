/* 기법 9 (논블로킹 시간 처리) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_time.c -o test_time
 * 시간은 가상 틱(ms)으로 흉내 낸다. 보드에서는 now_ms()를 타이머 틱으로 바꾼다.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* 9-1. 기다리는 방식과 확인하는 방식 */
static uint32_t sim_ms;                       /* 가상 시간 (ms) */

static uint32_t now_ms(void)
{
    return sim_ms;
}

static bool time_due(uint32_t now, uint32_t deadline)   /* 기한이 지났는가 (되감김에도 맞다) */
{
    return (int32_t)(now - deadline) >= 0;
}

#define EVENT_AT  120u                        /* 버튼이 눌리는 시각 */

static bool button_down(void)
{
    return time_due(now_ms(), EVENT_AT);
}

static void delay_ms(uint32_t ms)             /* 블로킹: 시간이 찰 때까지 다른 일을 못 한다 */
{
    uint32_t start = now_ms();

    while ((uint32_t)(now_ms() - start) < ms) {
        sim_ms++;                             /* 가상 시간이 1 ms 흐른다 */
    }
}

static uint32_t run_blocking(void)
{
    sim_ms = 0u;
    for (;;) {
        /* LED 반전 */
        delay_ms(500u);                       /* 500 ms 동안 버튼을 보지 못한다 */
        if (button_down()) {
            return now_ms();
        }
    }
}

static uint32_t run_nonblocking(void)
{
    uint32_t next_blink = 500u;

    sim_ms = 0u;
    for (;;) {
        sim_ms++;                             /* 메인 루프 한 바퀴 (1 ms) */
        if (button_down()) {
            return now_ms();
        }
        if (time_due(now_ms(), next_blink)) {
            /* LED 반전 */
            next_blink += 500u;
        }
    }
}

static void demo_block(void)
{
    uint32_t a = run_blocking();
    uint32_t b = run_nonblocking();

    printf("[9-1] 버튼은 %u ms에 눌림: 블로킹 방식이 처리한 시각=%u ms, 논블로킹 방식=%u ms\n",
           (unsigned)EVENT_AT, (unsigned)a, (unsigned)b);
}

/* 9-2. 주기 작업: 다음 기한을 어떻게 정하는가 */
typedef struct {
    uint32_t period;
    uint32_t next;
} soft_timer_t;

static bool timer_fixed(soft_timer_t *t, uint32_t now)   /* 기한에 주기를 더한다 */
{
    if (!time_due(now, t->next)) {
        return false;
    }
    t->next += t->period;
    return true;
}

static bool timer_drift(soft_timer_t *t, uint32_t now)   /* 지금 시각에 주기를 더한다 */
{
    if (!time_due(now, t->next)) {
        return false;
    }
    t->next = now + t->period;
    return true;
}

static unsigned count_fires(bool (*f)(soft_timer_t *, uint32_t), uint32_t tick0)
{
    soft_timer_t t;
    uint32_t now = tick0;
    unsigned n = 0;

    t.period = 100u;
    t.next = tick0 + 100u;
    while ((uint32_t)(now - tick0) < 1000u) {
        if (f(&t, now)) {
            n++;
        }
        now += 30u;                           /* 한 바퀴에 30 ms 걸린다 */
    }
    return n;
}

static bool naive_due(uint32_t now, uint32_t deadline)   /* 되감김을 고려하지 않은 비교 */
{
    return now >= deadline;
}

static void demo_period(void)
{
    uint32_t start = 0xFFFFFFF0u;             /* 틱이 되감기기 직전 */
    uint32_t deadline = start + 100u;         /* 0x00000054로 되감긴다 */

    printf("[9-2] 100 ms 주기, 1000 ms 동안 (한 바퀴 30 ms): 기한에 더함=%u회, 지금 시각에 더함=%u회\n",
           count_fires(timer_fixed, 0u), count_fires(timer_drift, 0u));
    printf("      틱 0xFFFFFFF0에서 시작: 시작 직후 기한이 지났나? 부호 있는 비교=%d, 단순 비교=%d\n",
           (int)time_due(start, deadline), (int)naive_due(start, deadline));
}

int main(void)
{
    demo_block();
    demo_period();
    return 0;
}
