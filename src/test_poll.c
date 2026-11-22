/* 기법 4 (폴링 루프와 타임아웃) 예제 검증용 (PC에서 컴파일·실행)
 * 빌드: gcc -std=c11 -Wall -Wextra -O0 test_poll.c -o test_poll
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/* 4-1. 시험용 가상 장치 (보드에서는 status_read()를 REG32(STATUS 주소)로, ticks_now()를 타이머 틱으로 바꾼다) */
#define ST_READY  0x1u            /* STATUS bit0: 준비됨 */
#define ST_ERROR  0x2u            /* STATUS bit1: 오류 */

#define RC_OK          0
#define RC_TIMEOUT   (-1)
#define RC_DEVICE    (-2)

static uint32_t sim_tick;         /* 가상 틱 (상태를 읽을 때마다 1 증가) */
static uint32_t sim_reads;        /* 상태를 읽은 횟수 */
static uint32_t sim_ready_after;  /* 이 횟수째 읽을 때 준비 비트가 올라간다 */
static uint32_t sim_error_after;  /* 이 횟수째 읽을 때 오류 비트가 올라간다 (0이면 오류 없음) */

static uint32_t status_read(void)
{
    uint32_t st = 0u;

    sim_tick++;
    sim_reads++;
    if (sim_reads >= sim_ready_after) {
        st |= ST_READY;
    }
    if (sim_error_after != 0u && sim_reads >= sim_error_after) {
        st |= ST_ERROR;
    }
    return st;
}

static uint32_t ticks_now(void)
{
    return sim_tick;
}

static void sim_reset(uint32_t tick0, uint32_t ready_after, uint32_t error_after)
{
    sim_tick = tick0;
    sim_reads = 0u;
    sim_ready_after = ready_after;
    sim_error_after = error_after;
}

static const char *rc_name(int rc)
{
    return (rc == RC_OK) ? "OK" : (rc == RC_TIMEOUT) ? "TIMEOUT" : "DEVICE";
}

/* 4-2. 타임아웃 없는 대기: 장치가 응답하지 않으면 돌아오지 않는다 */
static void wait_ready_forever(void)
{
    while ((status_read() & ST_READY) == 0u) {
        /* 준비될 때까지 반복 */
    }
}

static void demo_forever(void)
{
    sim_reset(0u, 5u, 0u);                    /* 5번째 읽기에서 준비됨 */
    wait_ready_forever();
    printf("[4-2] 준비됨: 상태를 %u번 읽고 돌아옴\n", (unsigned)sim_reads);
}

/* 4-3. 반복 횟수로 제한 */
static int wait_ready_count(uint32_t max_loops)
{
    while (max_loops-- != 0u) {
        if ((status_read() & ST_READY) != 0u) {
            return RC_OK;
        }
    }
    return RC_TIMEOUT;
}

static void demo_count(void)
{
    int rc;

    sim_reset(0u, 5u, 0u);
    rc = wait_ready_count(10u);
    printf("[4-3] 5번째에 준비, 최대 10회: %s (읽은 횟수=%u)\n", rc_name(rc), (unsigned)sim_reads);

    sim_reset(0u, 50u, 0u);
    rc = wait_ready_count(10u);
    printf("      50번째에 준비, 최대 10회: %s (읽은 횟수=%u)\n", rc_name(rc), (unsigned)sim_reads);
}

/* 4-4. 틱 시간으로 제한 (경과는 뺄셈으로 구한다) */
static int wait_ready_ticks(uint32_t timeout_ticks)
{
    uint32_t start = ticks_now();

    for (;;) {
        if ((status_read() & ST_READY) != 0u) {          /* 상태를 먼저 보고 */
            return RC_OK;
        }
        if ((uint32_t)(ticks_now() - start) >= timeout_ticks) {   /* 시간은 나중에 본다 */
            return RC_TIMEOUT;
        }
    }
}

static void demo_ticks(void)
{
    static const struct { uint32_t tick0, ready_after; const char *note; } cases[4] = {
        { 0x00000000u,   5u, "5번째에 준비" },
        { 0x00000000u,  50u, "50번째에 준비" },
        { 0xFFFFFFFCu, 100u, "틱이 되감기는 구간에서 시작, 준비되지 않음" },
        { 0x00000000u,  10u, "제한과 같은 10번째에 준비" },
    };

    printf("[4-4] 제한 10틱\n");
    for (int i = 0; i < 4; i++) {
        sim_reset(cases[i].tick0, cases[i].ready_after, 0u);
        int rc = wait_ready_ticks(10u);
        printf("      %s: %s (읽은 횟수=%u)\n", cases[i].note, rc_name(rc), (unsigned)sim_reads);
    }
}

/* 4-5. 준비, 오류, 시간 초과를 함께 처리 */
static int wait_ready_or_error(uint32_t timeout_ticks)
{
    uint32_t start = ticks_now();

    for (;;) {
        uint32_t st = status_read();                    /* 한 번 읽어 변수에 담아 판단한다 */

        if ((st & ST_ERROR) != 0u) {
            return RC_DEVICE;
        }
        if ((st & ST_READY) != 0u) {
            return RC_OK;
        }
        if ((uint32_t)(ticks_now() - start) >= timeout_ticks) {
            return RC_TIMEOUT;
        }
    }
}

static void demo_status(void)
{
    int rc;

    sim_reset(0u, 5u, 0u);
    rc = wait_ready_or_error(10u);
    printf("[4-5] 5번째에 준비: %s (읽은 횟수=%u)\n", rc_name(rc), (unsigned)sim_reads);

    sim_reset(0u, 100u, 3u);
    rc = wait_ready_or_error(10u);
    printf("      3번째에 오류: %s (읽은 횟수=%u)\n", rc_name(rc), (unsigned)sim_reads);

    sim_reset(0u, 100u, 0u);
    rc = wait_ready_or_error(10u);
    printf("      준비되지 않음: %s (읽은 횟수=%u)\n", rc_name(rc), (unsigned)sim_reads);
}

/* 4-6. 시간 초과일 때만 다시 시도 */
static int wait_with_retry(uint32_t timeout_ticks, int max_try, int *tries)
{
    int rc = RC_TIMEOUT;

    *tries = 0;
    while (*tries < max_try && rc == RC_TIMEOUT) {      /* 장치 오류는 다시 시도하지 않는다 */
        rc = wait_ready_or_error(timeout_ticks);
        (*tries)++;
    }
    return rc;
}

static void demo_retry(void)
{
    int tries;
    int rc;

    sim_reset(0u, 25u, 0u);                              /* 느린 장치: 25번째에 준비 */
    rc = wait_with_retry(10u, 3, &tries);
    printf("[4-6] 느린 장치: %s (시도=%d, 읽은 횟수=%u)\n", rc_name(rc), tries, (unsigned)sim_reads);

    sim_reset(0u, 100u, 0u);                             /* 응답 없는 장치 */
    rc = wait_with_retry(10u, 3, &tries);
    printf("      응답 없음: %s (시도=%d, 읽은 횟수=%u)\n", rc_name(rc), tries, (unsigned)sim_reads);

    sim_reset(0u, 100u, 2u);                             /* 오류를 알리는 장치 */
    rc = wait_with_retry(10u, 3, &tries);
    printf("      장치 오류: %s (시도=%d, 읽은 횟수=%u)\n", rc_name(rc), tries, (unsigned)sim_reads);
}

int main(void)
{
    demo_forever();
    demo_count();
    demo_ticks();
    demo_status();
    demo_retry();
    return 0;
}
