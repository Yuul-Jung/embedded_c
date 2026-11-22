/* 기초 C 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_struct.c -o test_struct
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* ---------- 기초 C-1: 구조체의 메모리 배치 ---------- */

/* C-1. 중첩 구조체의 크기와 멤버 위치 */
struct header {
    uint8_t  type;
    uint8_t  len;
    uint16_t seq;
};

struct message {
    uint8_t       id;
    struct header hdr;          /* 정렬은 4가 아니라 2 (가장 큰 멤버가 uint16_t) */
    uint32_t      payload[2];
    uint8_t       crc;
};

static void demo_layout(void)
{
    printf("[C-1] header : sizeof=%zu alignof=%zu\n",
           sizeof(struct header), _Alignof(struct header));
    printf("      message: sizeof=%zu alignof=%zu\n",
           sizeof(struct message), _Alignof(struct message));
    printf("      offset id=%zu hdr=%zu payload=%zu crc=%zu\n",
           offsetof(struct message, id),
           offsetof(struct message, hdr),
           offsetof(struct message, payload),
           offsetof(struct message, crc));
}

/* C-2. 멤버 주소 = 구조체 시작 주소 + 오프셋, 그리고 -> 접근 */
static void demo_member_addr(void)
{
    struct message m = {0};
    const uint8_t *base = (const uint8_t *)&m;
    int same = ((const uint8_t *)&m.payload[1] ==
                base + offsetof(struct message, payload) + sizeof(uint32_t));

    struct message *pm = &m;              /* 구조체 포인터 */
    pm->hdr.seq = 0x1234u;                /* (*pm).hdr.seq 와 같은 표기 */
    pm->payload[1] = 0xCAFEu;

    printf("[C-2] &m.payload[1] == base + offset + 4 ? %s\n", same ? "참" : "거짓");
    printf("      m.hdr.seq=0x%04X m.payload[1]=0x%04X\n",
           (unsigned)m.hdr.seq, (unsigned)m.payload[1]);
}

/* C-3. 초기화, 대입(복사), 포인터 전달 */
static void bump(struct header *h)
{
    h->seq += 1;
}

static void demo_init_pass(void)
{
    struct header h = { .type = 1, .len = 8, .seq = 0x0100u };  /* 지정 초기화 */
    struct header copy = h;                                      /* 대입은 바이트 복사 */
    struct header zero = { .type = 9 };                          /* 나머지 멤버는 0 */

    bump(&h);

    printf("[C-3] h   : type=%u len=%u seq=0x%04X\n",
           (unsigned)h.type, (unsigned)h.len, (unsigned)h.seq);
    printf("      copy: type=%u len=%u seq=0x%04X\n",
           (unsigned)copy.type, (unsigned)copy.len, (unsigned)copy.seq);
    printf("      zero: type=%u len=%u seq=0x%04X\n",
           (unsigned)zero.type, (unsigned)zero.len, (unsigned)zero.seq);
}

/* ---------- 기초 C-2: 레지스터 맵 구조체 ---------- */

/* C-4. 가상 타이머 장치의 레지스터 블록 (예시용 장치) */
typedef struct {
    volatile uint32_t CTRL;          /* 0x00 제어 */
    volatile uint32_t STATUS;        /* 0x04 상태 */
    volatile uint32_t LOAD;          /* 0x08 초기값 */
    volatile uint32_t COUNT;         /* 0x0C 현재값 */
    uint32_t          reserved[4];   /* 0x10~0x1C 예약 영역 */
    volatile uint32_t INTMASK;       /* 0x20 인터럽트 마스크 */
} timer_regs_t;

/* 데이터시트의 오프셋과 다르면 컴파일이 멈춘다 */
_Static_assert(offsetof(timer_regs_t, STATUS)  == 0x04, "STATUS 오프셋");
_Static_assert(offsetof(timer_regs_t, LOAD)    == 0x08, "LOAD 오프셋");
_Static_assert(offsetof(timer_regs_t, COUNT)   == 0x0C, "COUNT 오프셋");
_Static_assert(offsetof(timer_regs_t, INTMASK) == 0x20, "INTMASK 오프셋");
_Static_assert(sizeof(timer_regs_t) == 0x24, "블록 크기");

static timer_regs_t fake_timer;                    /* PC: 가짜 레지스터 블록 */
#define TIMER ((timer_regs_t *)&fake_timer)        /* 보드: ((timer_regs_t *)기준주소) */

static void demo_regmap(void)
{
    TIMER->LOAD     = 1000u;
    TIMER->CTRL    |= 0x1u;                        /* 읽기-수정-쓰기 */
    TIMER->INTMASK  = 0x3u;

    printf("[C-4] offset STATUS=0x%02zX LOAD=0x%02zX INTMASK=0x%02zX sizeof=0x%02zX\n",
           offsetof(timer_regs_t, STATUS), offsetof(timer_regs_t, LOAD),
           offsetof(timer_regs_t, INTMASK), sizeof(timer_regs_t));
    printf("      LOAD=%u CTRL=0x%X INTMASK=0x%X\n",
           (unsigned)TIMER->LOAD, (unsigned)TIMER->CTRL, (unsigned)TIMER->INTMASK);
}

/* C-5. 예약 영역을 빼먹은 잘못된 정의 (offset이 어긋난다) */
struct bad_timer_regs {
    uint32_t CTRL;
    uint32_t STATUS;
    uint32_t LOAD;
    uint32_t COUNT;
    uint32_t INTMASK;                              /* 0x20 이어야 하는데 */
};

static void demo_bad_regmap(void)
{
    printf("[C-5] bad INTMASK offset=0x%02zX (데이터시트: 0x20), sizeof=0x%02zX\n",
           offsetof(struct bad_timer_regs, INTMASK), sizeof(struct bad_timer_regs));
}

int main(void)
{
    demo_layout();
    demo_member_addr();
    demo_init_pass();
    demo_regmap();
    demo_bad_regmap();
    return 0;
}
