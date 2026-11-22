/* 기법 2 (volatile) 예제 검증용
 * 실행 검증: gcc -std=c11 -Wall -Wextra -O0 test_vol.c -o test_vol
 * 최적화 효과 확인: gcc -std=c11 -O2 -S -masm=intel test_vol.c   (생성된 어셈블리를 책에 싣는다)
 * 함수는 static이 아니다. static이면 호출되지 않는 함수가 -O2에서 통째로 사라진다.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* 2-1. 상태 비트를 기다리는 폴링 루프 */
uint32_t wait_plain(const uint32_t *status)
{
    while ((*status & 1u) == 0u) {
        /* 비트 0이 1이 될 때까지 반복 */
    }
    return *status;
}

uint32_t wait_volatile(const volatile uint32_t *status)
{
    while ((*status & 1u) == 0u) {
        /* 비트 0이 1이 될 때까지 반복 */
    }
    return *status;
}

/* 2-2. 같은 레지스터에 연속으로 쓰기 */
void send2_plain(uint32_t *tx)
{
    *tx = 0x41u;
    *tx = 0x42u;
}

void send2_volatile(volatile uint32_t *tx)
{
    *tx = 0x41u;
    *tx = 0x42u;
}

/* 2-3. 같은 레지스터를 연속으로 읽기 */
uint32_t read2_plain(const uint32_t *rx)
{
    uint32_t a = *rx;
    uint32_t b = *rx;
    return a + b;
}

uint32_t read2_volatile(const volatile uint32_t *rx)
{
    uint32_t a = *rx;
    uint32_t b = *rx;
    return a + b;
}

/* 2-5. 레지스터 접근 매크로와 포인터 선언 */
#define REG32(addr)  (*(volatile uint32_t *)(uintptr_t)(addr))

static uint32_t fake_regs[2];                    /* 0:STATUS(읽기 전용) 1:TXDATA */

static void demo_reg(void)
{
    uintptr_t base = (uintptr_t)fake_regs;
    const volatile uint32_t *status = (const volatile uint32_t *)base;  /* 읽기 전용 레지스터 */
    volatile uint32_t       *txdata = (volatile uint32_t *)(base + 4u); /* 읽고 쓰는 레지스터 */

    fake_regs[0] = 0x1u;                         /* 장치가 상태 비트를 올렸다고 가정 */
    while ((*status & 0x1u) == 0u) {
        /* 준비될 때까지 대기 */
    }
    *txdata = 0x41u;
    REG32(base + 4u) |= 0x100u;                  /* 읽기-수정-쓰기 */

    printf("[2-5] STATUS=0x%X TXDATA=0x%X\n", (unsigned)*status, (unsigned)*txdata);
}

/* 2-4. 읽으면 소모되는 레지스터는 한 번 읽어 변수에 담아 쓴다 (가상 장치) */
static const char rx_data[] = "ABC";
static size_t     rx_pos;

static uint32_t rx_reg_read(void)    /* 읽을 때마다 글자 하나가 빠지는 가상 RX 레지스터 */
{
    return (uint32_t)(unsigned char)rx_data[rx_pos < 3u ? rx_pos++ : 3u];
}

static void demo_pop(void)
{
    uint32_t used = 0u;

    rx_pos = 0u;
    if (rx_reg_read() >= 'A') {      /* 첫 읽기: 판단에만 쓰고 버려진다 */
        used = rx_reg_read();        /* 두 번째 읽기: 다음 글자가 나온다 */
    }
    printf("[2-4] 두 번 읽으면: 사용한 글자=%c, FIFO에 남은 글자=%zu\n",
           (char)used, 3u - rx_pos);

    rx_pos = 0u;
    uint32_t ch = rx_reg_read();     /* 한 번 읽어 변수에 담는다 */
    if (ch >= 'A') {
        used = ch;
    }
    printf("      한 번 읽으면: 사용한 글자=%c, FIFO에 남은 글자=%zu\n",
           (char)used, 3u - rx_pos);
}

int main(void)
{
    uint32_t s = 1u, t = 0u, r = 7u;

    printf("[2-1] wait_plain=%u wait_volatile=%u\n", (unsigned)wait_plain(&s), (unsigned)wait_volatile(&s));
    send2_plain(&t);
    send2_volatile(&t);
    printf("[2-2] t=0x%X\n", (unsigned)t);
    printf("[2-3] read2_plain=%u read2_volatile=%u\n", (unsigned)read2_plain(&r), (unsigned)read2_volatile(&r));
    demo_reg();
    demo_pop();
    return 0;
}
