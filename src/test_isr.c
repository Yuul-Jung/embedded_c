/* 기법 5 (ISR 작성 규칙) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_isr.c -o test_isr
 * 인터럽트는 사람이 정한 순서로 ISR 함수를 직접 호출하는 방식으로 흉내 낸다.
 */
#include <stdint.h>
#include <stdio.h>

/* 5-1. 플래그와 카운터: 메인이 바쁜 사이 인터럽트가 겹쳤을 때 */
static volatile uint8_t  ev_flag;        /* ISR가 1로 올리고, 메인이 0으로 내린다 */
static volatile uint32_t ev_count;       /* ISR만 쓴다 (메인은 읽기만 한다) */
static uint32_t          ev_seen;        /* 메인만 쓴다 */

static void event_isr(void)              /* 인터럽트가 발생하면 호출되는 함수 */
{
    ev_count++;                          /* 발생 횟수를 센다 */
    ev_flag = 1u;                        /* 메인에 알린다 */
}

static uint32_t take_by_flag(void)       /* 플래그 방식: 몇 번 발생했든 한 번으로 보인다 */
{
    if (ev_flag != 0u) {
        ev_flag = 0u;
        return 1u;
    }
    return 0u;
}

static uint32_t take_by_count(void)      /* 카운터 방식: 발생한 횟수만큼 처리한다 */
{
    uint32_t now = ev_count;             /* 한 번만 읽어 변수에 담는다 */
    uint32_t n = now - ev_seen;          /* 부호 없는 뺄셈이라 카운터가 되감겨도 맞다 */

    ev_seen = now;
    return n;
}

static void demo_merge(void)
{
    event_isr();                         /* 메인이 바쁜 사이 인터럽트가 3번 발생 */
    event_isr();
    event_isr();
    printf("[5-1] 3번 발생 후 메인이 확인: 플래그 방식=%u회, 카운터 방식=%u회\n",
           (unsigned)take_by_flag(), (unsigned)take_by_count());

    ev_count = 0xFFFFFFFEu;              /* 카운터가 되감기기 직전으로 만든다 */
    ev_seen  = 0xFFFFFFFEu;
    event_isr();
    event_isr();
    event_isr();
    printf("      카운터가 0xFFFFFFFE에서 시작해 되감겨도: %u회\n",
           (unsigned)take_by_count());
}

/* 5-3. 인터럽트 원인을 지우지 않는 ISR (가상 인터럽트 컨트롤러) */
static int irq_pending;                  /* 장치가 올린 인터럽트 요청. 지우기 전까지 유지된다 */
static int isr_calls;

static void irq_clear(void)
{
    irq_pending = 0;
}

static void isr_without_clear(void)      /* 원인을 지우지 않는다 */
{
    isr_calls++;
}

static void isr_with_clear(void)         /* 원인을 가장 먼저 지운다 */
{
    irq_clear();
    isr_calls++;
}

static int run_irq(void (*isr)(void))    /* 요청이 남아 있으면 CPU가 곧바로 ISR에 다시 들어간다 */
{
    int guard = 0;

    isr_calls = 0;
    irq_pending = 1;
    while (irq_pending && guard < 5) {   /* 실제로는 guard 없이 끝없이 반복한다 */
        isr();
        guard++;
    }
    return isr_calls;
}

static void demo_clear(void)
{
    printf("[5-3] 원인을 지우지 않는 ISR: %d회 호출 후 중단 (실제로는 끝없이 반복)\n",
           run_irq(isr_without_clear));
    printf("      원인을 지우는 ISR: %d회 호출\n", run_irq(isr_with_clear));
}

int main(void)
{
    demo_merge();
    demo_clear();
    return 0;
}
