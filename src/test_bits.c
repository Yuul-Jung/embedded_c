/* 기법 1 (비트 연산) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_bits.c -o test_bits
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* 1-1. 비트 켜기·끄기·뒤집기·검사 */
#define BIT(n)          (1u << (n))
#define BITS_SET(x, m)  ((x) |=  (m))
#define BITS_CLR(x, m)  ((x) &= ~(m))
#define BITS_TOG(x, m)  ((x) ^=  (m))
#define BITS_TST(x, m)  (((x) & (m)) != 0u)

static void demo_bits(void)
{
    uint32_t x = 0u;

    BITS_SET(x, BIT(0) | BIT(3));
    printf("[1-1] set(0,3) : x=0x%02X\n", (unsigned)x);

    BITS_CLR(x, BIT(0));
    printf("      clr(0)   : x=0x%02X\n", (unsigned)x);

    BITS_TOG(x, BIT(1) | BIT(3));
    printf("      tog(1,3) : x=0x%02X\n", (unsigned)x);

    printf("      tst(1)=%d tst(3)=%d\n", BITS_TST(x, BIT(1)), BITS_TST(x, BIT(3)));
}

/* 1-2. 정수 승격과 부호 */
static void demo_promote(void)
{
    uint8_t m = 0x01u;
    int inv = ~m;                       /* m이 int로 승격된 뒤에 반전된다 */

    printf("[1-2] ~m = 0x%X, (uint8_t)~m = 0x%02X, ~m == 0xFE ? %s\n",
           (unsigned)inv, (unsigned)(uint8_t)inv, (inv == 0xFE) ? "참" : "거짓");
    printf("      BIT(31) = 0x%08X\n", (unsigned)BIT(31));
}

/* 1-3. 여러 비트 필드 읽기·쓰기 */
#define FIELD_MASK(shift, width)  (((1u << (width)) - 1u) << (shift))
#define FIELD_GET(x, shift, width)  (((x) >> (shift)) & ((1u << (width)) - 1u))
#define FIELD_SET(x, shift, width, v) \
    (((x) & ~FIELD_MASK(shift, width)) | (((uint32_t)(v) << (shift)) & FIELD_MASK(shift, width)))

#define CTRL_ENABLE_SHIFT  0     /* [0]    ENABLE */
#define CTRL_MODE_SHIFT    1     /* [2:1]  MODE   */
#define CTRL_MODE_WIDTH    2
#define CTRL_DIV_SHIFT     8     /* [15:8] DIV    */
#define CTRL_DIV_WIDTH     8

static uint32_t fake_ctrl;       /* 가상 CTRL 레지스터 */

static void demo_field(void)
{
    uint32_t v = fake_ctrl;                                        /* 읽고 */
    v = FIELD_SET(v, CTRL_MODE_SHIFT, CTRL_MODE_WIDTH, 2u);        /* 필드만 바꾸고 */
    v = FIELD_SET(v, CTRL_DIV_SHIFT,  CTRL_DIV_WIDTH,  0x64u);
    v |= BIT(CTRL_ENABLE_SHIFT);
    fake_ctrl = v;                                                 /* 한 번에 쓴다 */
    printf("[1-3] CTRL=0x%04X MODE=%u DIV=0x%02X\n", (unsigned)fake_ctrl,
           (unsigned)FIELD_GET(fake_ctrl, CTRL_MODE_SHIFT, CTRL_MODE_WIDTH),
           (unsigned)FIELD_GET(fake_ctrl, CTRL_DIV_SHIFT, CTRL_DIV_WIDTH));

    v = fake_ctrl;
    v = FIELD_SET(v, CTRL_MODE_SHIFT, CTRL_MODE_WIDTH, 1u);        /* MODE만 2 -> 1 */
    fake_ctrl = v;
    printf("      MODE를 1로: CTRL=0x%04X (ENABLE, DIV 유지)\n", (unsigned)fake_ctrl);

    fake_ctrl = FIELD_SET(0u, CTRL_MODE_SHIFT, CTRL_MODE_WIDTH, 3u);   /* 읽지 않고 바로 쓰면 */
    printf("      읽지 않고 MODE만 쓰면: CTRL=0x%04X (DIV, ENABLE이 사라짐)\n", (unsigned)fake_ctrl);
}

int main(void)
{
    demo_bits();
    demo_promote();
    demo_field();
    return 0;
}
