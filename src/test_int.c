/* 기법 3 (고정 폭 정수형) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_int.c -o test_int
 */
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

/* 3-1. 형의 크기와 char의 부호 */
int char_to_int(void)
{
    char c = (char)200;          /* char가 부호 있는 형인지에 따라 값이 달라진다 */
    return c;
}

static void demo_sizes(void)
{
    printf("[3-1] char=%zu short=%zu int=%zu long=%zu long long=%zu 포인터=%zu\n",
           sizeof(char), sizeof(short), sizeof(int), sizeof(long),
           sizeof(long long), sizeof(void *));
    printf("      uint8_t=%zu uint16_t=%zu uint32_t=%zu uint64_t=%zu\n",
           sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(uint64_t));
    printf("      CHAR_MIN=%d (char는 부호 %s), char_to_int()=%d\n",
           CHAR_MIN, (CHAR_MIN < 0) ? "있음" : "없음", char_to_int());
}

/* 3-2. 크기는 컴파일 단계에서 확인하고, 출력은 PRI 매크로로 */
_Static_assert(sizeof(uint32_t) == 4, "uint32_t는 4바이트여야 한다");
_Static_assert(sizeof(uintptr_t) >= sizeof(void *), "uintptr_t는 포인터를 담아야 한다");

static void demo_fmt(void)
{
    uint32_t x = 4000000000u;
    uint8_t  b = 0xA5u;

    printf("[3-2] x=%" PRIu32 " (0x%08" PRIX32 "), b=0x%02" PRIX8 "\n", x, x, b);
}

/* 3-3. 정수 승격과 잘림 */
static void demo_promote(void)
{
    uint8_t a = 200u, b = 100u;
    uint8_t c = (uint8_t)(a + b);     /* a + b는 int로 계산되어 300, 대입하면서 하위 8비트만 남아 44 */
    int     d = a + b;                /* int 그대로 300 */

    printf("[3-3] c=%u d=%d, (a + b > 255) = %d\n", (unsigned)c, d, (a + b) > 255);
}

/* 3-4. 부호 있는 값과 부호 없는 값의 비교 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"   /* 문제를 보이려고 경고를 일부러 끈다 */
static int less_mixed(int i, unsigned u)
{
    return i < u;                     /* i가 unsigned로 변환되어 비교된다 */
}
#pragma GCC diagnostic pop

static void demo_mixed(void)
{
    printf("[3-4] -1 < 1u ? %s, (unsigned)-1 = %u\n",
           less_mixed(-1, 1u) ? "참" : "거짓", (unsigned)-1);
}

/* 3-5. 틱 카운터가 되감길 때의 시간 비교 */
static uint32_t ticks_now;            /* 1 ms마다 1씩 늘어나는 가상 틱 */

static int timed_out_wrong(uint32_t start, uint32_t timeout)
{
    return ticks_now > start + timeout;                 /* 되감기면 틀린다 */
}

static int timed_out_right(uint32_t start, uint32_t timeout)
{
    return (uint32_t)(ticks_now - start) >= timeout;    /* 뺄셈은 되감김과 무관하다 */
}

static void demo_wrap(void)
{
    uint32_t start   = 0xFFFFFFF0u;   /* 되감기기 16틱 전에 시작 */
    uint32_t timeout = 0x20u;         /* 32틱 */
    static const uint32_t now_list[2] = { 0xFFFFFFF8u, 0x00000010u };   /* 8틱 경과, 32틱 경과 */

    printf("[3-5] start=0x%08" PRIX32 " timeout=%" PRIu32 "\n", start, timeout);
    for (int i = 0; i < 2; i++) {
        ticks_now = now_list[i];
        printf("      now=0x%08" PRIX32 " 경과=%2" PRIu32 "틱  틀린 식=%d  맞는 식=%d\n",
               ticks_now, (uint32_t)(ticks_now - start),
               timed_out_wrong(start, timeout), timed_out_right(start, timeout));
    }
}

int main(void)
{
    demo_sizes();
    demo_fmt();
    demo_promote();
    demo_mixed();
    demo_wrap();
    return 0;
}
