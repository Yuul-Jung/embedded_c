/* 기법 7 (링 버퍼) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_ring.c -o test_ring
 * 인터럽트는 ISR 함수를 직접 호출하는 방식으로 흉내 낸다.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* 7-1. 링 버퍼: 쓰는 쪽(head)과 읽는 쪽(tail)이 각자 자기 인덱스만 고친다 */
#define RING_SIZE  8u                         /* 2의 거듭제곱 */
#define RING_MASK  (RING_SIZE - 1u)

typedef struct {
    uint8_t           data[RING_SIZE];
    volatile uint32_t head;                   /* 쓴 개수 (쓰는 쪽만 고친다) */
    volatile uint32_t tail;                   /* 읽은 개수 (읽는 쪽만 고친다) */
} ring_t;

_Static_assert((RING_SIZE & (RING_SIZE - 1u)) == 0u, "RING_SIZE는 2의 거듭제곱이어야 한다");

static uint32_t ring_count(const ring_t *r)   /* 들어 있는 개수. 부호 없는 뺄셈이라 되감겨도 맞다 */
{
    return r->head - r->tail;
}

static bool ring_push(ring_t *r, uint8_t v)   /* 쓰는 쪽에서 호출 */
{
    if (ring_count(r) >= RING_SIZE) {
        return false;                         /* 가득 참: 새 값을 버린다 */
    }
    r->data[r->head & RING_MASK] = v;         /* 값을 먼저 쓰고 */
    r->head = r->head + 1u;                   /* 인덱스를 나중에 올린다 */
    return true;
}

static bool ring_pop(ring_t *r, uint8_t *v)   /* 읽는 쪽에서 호출 */
{
    if (ring_count(r) == 0u) {
        return false;                         /* 비어 있음 */
    }
    *v = r->data[r->tail & RING_MASK];        /* 값을 먼저 읽고 */
    r->tail = r->tail + 1u;                   /* 인덱스를 나중에 올린다 */
    return true;
}

static void demo_ring(void)
{
    ring_t r = { {0}, 0u, 0u };
    uint8_t v;
    int ok = 0;

    for (uint8_t i = 0; i < 10u; i++) {       /* 크기 8인 버퍼에 10개를 넣는다 */
        if (ring_push(&r, (uint8_t)(0x40u + i))) {
            ok++;
        }
    }
    printf("[7-1] 10개를 넣음: 들어간 개수=%d, 버퍼 안=%u개\n", ok, (unsigned)ring_count(&r));
    printf("      꺼낸 순서:");
    while (ring_pop(&r, &v)) {
        printf(" %02X", (unsigned)v);
    }
    printf("\n");

    r.head = 0xFFFFFFFCu;                     /* 인덱스가 되감기기 직전에서 시작 */
    r.tail = 0xFFFFFFFCu;
    for (uint8_t i = 0; i < 6u; i++) {
        (void)ring_push(&r, (uint8_t)(0x50u + i));
    }
    printf("      인덱스 되감김: head=0x%08X, tail=0x%08X, 개수=%u\n",
           (unsigned)r.head, (unsigned)r.tail, (unsigned)ring_count(&r));
    printf("      꺼낸 순서:");
    while (ring_pop(&r, &v)) {
        printf(" %02X", (unsigned)v);
    }
    printf("\n");
}

/* 7-2. 수신 ISR이 넣고 메인이 꺼낸다. 가득 차서 버린 개수를 센다 */
static ring_t   rx;
static volatile uint32_t rx_dropped;          /* ISR만 고친다 */

static void rx_isr(uint8_t byte)              /* 바이트가 도착하면 호출되는 함수 */
{
    if (!ring_push(&rx, byte)) {
        rx_dropped++;
    }
}

static unsigned drain(void)                   /* 메인에서 쌓인 바이트를 모두 처리 */
{
    uint8_t v;
    unsigned n = 0;

    while (ring_pop(&rx, &v)) {
        n++;
    }
    return n;
}

static void demo_rx(void)
{
    unsigned got = 0;

    for (uint8_t i = 0; i < 6u; i++) {        /* 메인이 바쁜 사이 6바이트 도착 */
        rx_isr(i);
    }
    got += drain();
    for (uint8_t i = 0; i < 12u; i++) {       /* 메인이 더 오래 바쁜 사이 12바이트 도착 */
        rx_isr(i);
    }
    got += drain();
    printf("[7-2] 총 18바이트 도착, 크기 %u: 처리=%u, 버림=%u\n",
           (unsigned)RING_SIZE, got, (unsigned)rx_dropped);
}

int main(void)
{
    demo_ring();
    demo_rx();
    return 0;
}
