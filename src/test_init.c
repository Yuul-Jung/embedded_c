/* 기법 6 (초기화 순서) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_init.c -o test_init
 * 장치와 인터럽트는 가상 모델로 흉내 낸다.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* 6-1. 클록이 꺼진 주변장치는 레지스터 쓰기를 받지 못한다 (가상 UART) */
#define SYS_CLK_HZ  100000000u    /* 이 시리즈 SoC의 시스템 클록 */
#define BAUD        115200u

static int      uart_clk_on;      /* 주변장치 클록 공급 여부 */
static uint32_t uart_baud_div;    /* BAUD 분주 레지스터 */

static void clk_enable_uart(void)
{
    uart_clk_on = 1;
}

static void uart_write_baud(uint32_t v)     /* 클록이 꺼져 있으면 쓰기가 무시된다 */
{
    if (uart_clk_on != 0) {
        uart_baud_div = v;
    }
}

static void uart_setup_baud(void)
{
    uart_write_baud(SYS_CLK_HZ / BAUD);
}

static void init_wrong(void)      /* 설정 먼저, 클록 나중 */
{
    uart_setup_baud();
    clk_enable_uart();
}

static void init_right(void)      /* 클록 먼저, 설정 나중 */
{
    clk_enable_uart();
    uart_setup_baud();
}

static void demo_order(void)
{
    uart_clk_on = 0;
    uart_baud_div = 0u;
    init_wrong();
    printf("[6-1] 설정 먼저, 클록 나중: BAUD 분주=%u (쓰기가 무시됨)\n", (unsigned)uart_baud_div);

    uart_clk_on = 0;
    uart_baud_div = 0u;
    init_right();
    printf("      클록 먼저, 설정 나중: BAUD 분주=%u (%u / %u)\n",
           (unsigned)uart_baud_div, (unsigned)SYS_CLK_HZ, (unsigned)BAUD);
}

/* 6-2. 단계마다 확인하고, 실패하면 다음 단계로 가지 않는다 */
#define INIT_OK            0
#define INIT_ERR_CLOCK   (-1)

#define STEP_CLOCK  0x1u
#define STEP_PIN    0x2u
#define STEP_UART   0x4u
#define STEP_IRQ    0x8u

static unsigned steps_done;       /* 실행을 마친 단계 */
static uint32_t pll_lock_after;   /* 이 횟수째 확인할 때 클록 잠금 비트가 올라간다 (시험용) */
static uint32_t pll_reads;

static int pll_locked(void)       /* 보드에서는 클록 상태 레지스터의 잠금 비트를 읽는다 */
{
    pll_reads++;
    return pll_reads >= pll_lock_after;
}

static int clock_init(uint32_t max_loops)
{
    while (max_loops-- != 0u) {   /* 클록 안정 대기에도 제한을 둔다 (기법 4) */
        if (pll_locked()) {
            steps_done |= STEP_CLOCK;
            return INIT_OK;
        }
    }
    return INIT_ERR_CLOCK;
}

static void pin_init(void)  { steps_done |= STEP_PIN; }
static void uart_init(void) { steps_done |= STEP_UART; }
static void irq_init(void)  { steps_done |= STEP_IRQ; }

static int system_init(void)
{
    int rc = clock_init(100u);

    if (rc != INIT_OK) {
        return rc;                /* 클록이 없으면 다음 단계로 가지 않는다 */
    }
    pin_init();
    uart_init();
    irq_init();
    return INIT_OK;
}

static void print_steps(unsigned s)
{
    static const char *const names[4] = { "클록", "핀", "UART", "인터럽트" };
    int any = 0;

    for (int i = 0; i < 4; i++) {
        if ((s & (1u << i)) != 0u) {
            printf("%s%s", any ? " " : "", names[i]);
            any = 1;
        }
    }
    if (!any) {
        printf("없음");
    }
}

static void demo_init(void)
{
    int rc;

    steps_done = 0u;
    pll_reads = 0u;
    pll_lock_after = 10u;
    rc = system_init();
    printf("[6-2] 클록이 10번째에 안정: rc=%s, 실행된 단계=", (rc == INIT_OK) ? "OK" : "CLOCK");
    print_steps(steps_done);
    printf("\n");

    steps_done = 0u;
    pll_reads = 0u;
    pll_lock_after = 1000u;
    rc = system_init();
    printf("      클록이 안정되지 않음: rc=%s, 실행된 단계=", (rc == INIT_OK) ? "OK" : "CLOCK");
    print_steps(steps_done);
    printf("\n");
}

/* 6-3. 핸들러가 준비된 뒤에 인터럽트를 허용한다 (가상 시스템) */
typedef void (*handler_t)(void);

static handler_t irq_handler;     /* 등록된 핸들러 (등록 전에는 NULL) */
static int       irq_enabled;     /* 전역 인터럽트 허용 */
static int       irq_pending;     /* 장치가 이미 올려 둔 인터럽트 요청 (부팅 중에 눌린 버튼 등) */
static int       handled;         /* 핸들러가 처리한 횟수 */

static void my_handler(void)
{
    irq_pending = 0;              /* 원인 해제 */
    handled++;
}

static void deliver(void)         /* 허용된 상태에서 요청이 있으면 CPU가 핸들러로 들어간다 */
{
    int guard = 0;

    while (irq_enabled && irq_pending && guard < 3) {   /* guard는 시험용 안전장치 */
        if (irq_handler != NULL) {
            irq_handler();
        }
        guard++;
    }
}

static void irq_enable_all(void)
{
    irq_enabled = 1;
    deliver();
}

static void boot_wrong(void)      /* 허용 먼저, 핸들러 나중 */
{
    irq_enable_all();
    irq_handler = my_handler;
}

static void boot_right(void)      /* 핸들러 먼저, 허용 나중 */
{
    irq_handler = my_handler;
    irq_enable_all();
}

static void boot_reset(void)
{
    irq_handler = NULL;
    irq_enabled = 0;
    irq_pending = 1;
    handled = 0;
}

static void demo_irq_order(void)
{
    boot_reset();
    boot_wrong();
    printf("[6-3] 인터럽트를 먼저 허용: 처리=%d회, 요청 남음=%d\n", handled, irq_pending);

    boot_reset();
    boot_right();
    printf("      핸들러를 먼저 등록: 처리=%d회, 요청 남음=%d\n", handled, irq_pending);
}

int main(void)
{
    demo_order();
    demo_init();
    demo_irq_order();
    return 0;
}
