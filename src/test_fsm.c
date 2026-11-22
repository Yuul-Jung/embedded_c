/* 기법 8 (상태 머신) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_fsm.c -o test_fsm
 */
#include <stdint.h>
#include <stdio.h>

/* 8-1. 버튼 디바운스 상태 머신: 1 ms마다 입력을 한 번 넣는다 */
#define DEBOUNCE_MS  5u                       /* 같은 값이 이 횟수 연속이면 확정 */

typedef enum { BTN_UP, BTN_DOWN_WAIT, BTN_DOWN, BTN_UP_WAIT } btn_state_t;
typedef enum { EV_NONE, EV_PRESS, EV_RELEASE } btn_event_t;

typedef struct {
    btn_state_t state;
    uint32_t    count;                        /* 새 값이 연속으로 나온 횟수 */
} btn_t;

static btn_event_t btn_step(btn_t *b, int level)   /* level: 1=눌림, 0=떼어짐 */
{
    switch (b->state) {
    case BTN_UP:                              /* 떼어진 상태에서 눌림이 보이면 확인 시작 */
        if (level) { b->state = BTN_DOWN_WAIT; b->count = 1u; }
        break;
    case BTN_DOWN_WAIT:                       /* 눌림이 계속되면 확정, 끊기면 취소 */
        if (!level) { b->state = BTN_UP; break; }
        if (++b->count >= DEBOUNCE_MS) { b->state = BTN_DOWN; return EV_PRESS; }
        break;
    case BTN_DOWN:                            /* 눌린 상태에서 떼어짐이 보이면 확인 시작 */
        if (!level) { b->state = BTN_UP_WAIT; b->count = 1u; }
        break;
    case BTN_UP_WAIT:                         /* 떼어짐이 계속되면 확정, 끊기면 취소 */
        if (level) { b->state = BTN_DOWN; break; }
        if (++b->count >= DEBOUNCE_MS) { b->state = BTN_UP; return EV_RELEASE; }
        break;
    default:                                  /* 있을 수 없는 값: 안전한 상태로 복귀 */
        b->state = BTN_UP;
        break;
    }
    return EV_NONE;
}

static void demo_debounce(void)
{
    /* 1 ms마다 읽은 값: 눌릴 때와 뗄 때 튀는(바운싱) 신호 */
    static const int in[40] = {
        0,0,0, 1,0,1,1,0,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,1,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0
    };
    btn_t b = { BTN_UP, 0u };
    int raw_edges = 0;

    int presses = 0;
    int releases = 0;

    for (int i = 1; i < 40; i++) {
        if (in[i] != in[i - 1]) {
            raw_edges++;
        }
    }
    printf("[8-1] 입력 40개에서 입력이 바뀐 횟수=%d\n", raw_edges);
    for (int i = 0; i < 40; i++) {
        btn_event_t e = btn_step(&b, in[i]);

        if (e != EV_NONE) {
            printf("      %2d ms: %s\n", i, (e == EV_PRESS) ? "PRESS" : "RELEASE");
            if (e == EV_PRESS) { presses++; } else { releases++; }
        }
    }
    printf("      확정된 이벤트: 누름 %d회, 뗌 %d회\n", presses, releases);
}

/* 8-2. 표 방식 상태 머신: 상태 x 이벤트 -> (다음 상태, 동작) */
typedef enum { M_OFF, M_ON, M_BLINK, M_COUNT } led_mode_t;
typedef enum { E_SHORT, E_LONG, E_COUNT } key_ev_t;

typedef struct {
    led_mode_t next;
    void  (*action)(void);                    /* 없으면 NULL */
} trans_t;

static int led_on;
static int blinking;
static void act_on(void)    { led_on = 1; blinking = 0; }
static void act_off(void)   { led_on = 0; blinking = 0; }
static void act_blink(void) { blinking = 1; }

static const trans_t TABLE[M_COUNT][E_COUNT] = {
    /*            짧게 누름              길게 누름        */
    /* M_OFF   */ { { M_ON,    act_on    }, { M_BLINK, act_blink } },
    /* M_ON    */ { { M_OFF,   act_off   }, { M_BLINK, act_blink } },
    /* M_BLINK */ { { M_OFF,   act_off   }, { M_OFF,   act_off   } },
};

static led_mode_t mode = M_OFF;

static void mode_handle(key_ev_t ev)
{
    const trans_t *t;

    if ((unsigned)mode >= (unsigned)M_COUNT || (unsigned)ev >= (unsigned)E_COUNT) {
        mode = M_OFF;                         /* 범위 밖이면 안전한 상태로 복귀 */
        return;
    }
    t = &TABLE[mode][ev];
    if (t->action != NULL) {
        t->action();
    }
    mode = t->next;
}

static const char *mode_name(led_mode_t m)
{
    return (m == M_OFF) ? "OFF" : (m == M_ON) ? "ON" : "BLINK";
}

static void demo_table(void)
{
    static const key_ev_t seq[5] = { E_SHORT, E_SHORT, E_LONG, E_SHORT, E_LONG };
    static const char *const ev_name[E_COUNT] = { "짧게", "길게" };

    printf("[8-2] 입력 순서에 따른 상태 변화\n");
    for (int i = 0; i < 5; i++) {
        mode_handle(seq[i]);
        printf("      %s 누름 -> %-5s (LED=%d, 깜빡임=%d)\n",
               ev_name[seq[i]], mode_name(mode), led_on, blinking);
    }
    mode = (led_mode_t)7;                        /* 깨진 상태값을 일부러 넣는다 */
    mode_handle(E_SHORT);
    printf("      범위 밖 상태값 7 -> %s\n", mode_name(mode));
}

int main(void)
{
    demo_debounce();
    demo_table();
    return 0;
}
