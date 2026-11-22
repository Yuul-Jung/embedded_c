/* 기초 A·B 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_basic.c -o test_basic
 * 가정: little-endian 호스트(x86-64). RISC-V(MicroBlaze V)도 little-endian이다.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* ---------- 기초 A: 메모리와 주소 ---------- */

/* A-1. 값을 바이트 단위로 들여다보기 (엔디안 확인) */
static void demo_endian(void)
{
    uint32_t v = 0x12345678u;
    const uint8_t *p = (const uint8_t *)&v;   /* 같은 주소를 1바이트 단위로 읽는다 */

    printf("[A-1] v = 0x%08X\n", (unsigned)v);
    for (int i = 0; i < 4; i++) {
        printf("  바이트 %d (주소 +%d) = 0x%02X\n", i, i, (unsigned)p[i]);
    }
}

/* A-2. 변수 크기와 정렬 */
struct bad_order  { uint8_t a; uint32_t b; uint8_t c; };   /* 패딩이 생긴다 */
struct good_order { uint32_t b; uint8_t a; uint8_t c; };   /* 큰 것부터 */

static void demo_alignment(void)
{
    printf("[A-2] sizeof(uint8_t)=%zu sizeof(uint16_t)=%zu sizeof(uint32_t)=%zu\n",
           sizeof(uint8_t), sizeof(uint16_t), sizeof(uint32_t));
    printf("  bad_order : sizeof=%zu, offset a=%zu b=%zu c=%zu\n",
           sizeof(struct bad_order),
           offsetof(struct bad_order, a),
           offsetof(struct bad_order, b),
           offsetof(struct bad_order, c));
    printf("  good_order: sizeof=%zu, offset b=%zu a=%zu c=%zu\n",
           sizeof(struct good_order),
           offsetof(struct good_order, b),
           offsetof(struct good_order, a),
           offsetof(struct good_order, c));
}

/* A-3. 변수의 주소: 같은 배열 안에서 상대 주소만 본다 (절대 주소는 실행마다 다름) */
static void demo_addr(void)
{
    uint32_t arr[4] = {10, 20, 30, 40};
    uintptr_t base = (uintptr_t)&arr[0];

    printf("[A-3] 배열 원소 간 주소 차이\n");
    for (int i = 0; i < 4; i++) {
        printf("  &arr[%d] - &arr[0] = %zu 바이트\n", i, (size_t)((uintptr_t)&arr[i] - base));
    }
}

/* ---------- 기초 B: 포인터 ---------- */

/* B-1. 포인터 산술: 더하는 값은 "바이트"가 아니라 "원소 개수" */
static void demo_ptr_arith(void)
{
    uint32_t arr[4] = {10, 20, 30, 40};
    uint32_t *p32 = &arr[0];
    uint8_t  *p8  = (uint8_t *)&arr[0];

    printf("[B-1] (p32+1) - p32 주소 차이 = %zu 바이트, *(p32+1) = %u\n",
           (size_t)((uintptr_t)(p32 + 1) - (uintptr_t)p32), (unsigned)*(p32 + 1));
    printf("      (p8 +1) - p8  주소 차이 = %zu 바이트\n",
           (size_t)((uintptr_t)(p8 + 1) - (uintptr_t)p8));
    printf("      arr[2] == *(arr+2) ? %s\n", (arr[2] == *(arr + 2)) ? "참" : "거짓");
}

/* B-2. 정수를 주소로 해석해 읽고 쓰기 (레지스터 접근의 원형)
 * 실제 보드에서는 0x40000000 같은 고정 주소를 쓰지만,
 * PC에서는 그 주소가 유효하지 않으므로 배열을 "가짜 레지스터 블록"으로 쓴다. */
static uint32_t fake_regs[4];                 /* 0:DATA 1:TRI 2:CTRL 3:STATUS */

#define REG32(addr)  (*(volatile uint32_t *)(uintptr_t)(addr))

static void demo_reg_access(void)
{
    uintptr_t base = (uintptr_t)fake_regs;    /* 보드에서는 base = 0x40000000 */

    REG32(base + 0x0) = 0xA5u;                /* DATA 레지스터에 쓰기 */
    REG32(base + 0x4) = 0x00u;                /* TRI  레지스터에 쓰기 */
    REG32(base + 0x8) |= (1u << 3);           /* 읽기-수정-쓰기: 비트 3만 켬 */

    printf("[B-2] fake_regs[0]=0x%02X fake_regs[1]=0x%02X fake_regs[2]=0x%02X\n",
           (unsigned)fake_regs[0], (unsigned)fake_regs[1], (unsigned)fake_regs[2]);
    printf("      REG32(base+0x0) 읽기 = 0x%02X\n", (unsigned)REG32(base + 0x0));
}

/* B-3. 형 변환으로 같은 메모리를 다른 폭으로 보기 */
static void demo_cast_view(void)
{
    uint32_t v = 0xAABBCCDDu;
    uint16_t lo, hi;
    memcpy(&lo, (const uint8_t *)&v + 0, sizeof lo);
    memcpy(&hi, (const uint8_t *)&v + 2, sizeof hi);
    printf("[B-3] v=0x%08X 하위16=0x%04X 상위16=0x%04X (little-endian)\n",
           (unsigned)v, (unsigned)lo, (unsigned)hi);
}

/* B-4. 포인터를 인자로 넘겨 호출자의 값을 바꾸기 */
static void inc_by_ptr(uint32_t *x) { *x += 1; }
static void inc_by_val(uint32_t x)  { x += 1; (void)x; }

static void demo_pass(void)
{
    uint32_t a = 5, b = 5;
    inc_by_val(a);
    inc_by_ptr(&b);
    printf("[B-4] 값 전달 후 a=%u, 포인터 전달 후 b=%u\n", (unsigned)a, (unsigned)b);
}

/* B-5. const 위치에 따른 의미 */
static void demo_const(void)
{
    uint32_t x = 1, y = 2;
    const uint32_t *p1 = &x;        /* 가리키는 값 수정 불가, 포인터 변경 가능 */
    uint32_t *const p2 = &x;        /* 가리키는 값 수정 가능, 포인터 변경 불가 */
    p1 = &y;                        /* 가능 */
    *p2 = 7;                        /* 가능 */
    printf("[B-5] *p1=%u (y를 가리킴), x=%u (p2로 수정됨)\n", (unsigned)*p1, (unsigned)x);
}

int main(void)
{
    demo_endian();
    demo_alignment();
    demo_addr();
    demo_ptr_arith();
    demo_reg_access();
    demo_cast_view();
    demo_pass();
    demo_const();
    return 0;
}
