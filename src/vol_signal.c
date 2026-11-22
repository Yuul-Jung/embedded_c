/* 기법 5: ISR과 공유하는 플래그의 volatile 유무를 PC에서 확인한다.
 * 시그널 핸들러가 ISR 역할을 한다 (실행 중인 코드를 끊고 비동기로 실행되는 함수).
 * 빌드 1: gcc -std=gnu11 -O2 vol_signal.c                 (volatile 없음)
 * 빌드 2: gcc -std=gnu11 -O2 -DUSE_VOLATILE vol_signal.c  (volatile 있음)
 */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/* 5-2. ISR(시그널 핸들러)과 공유하는 플래그 */
#ifdef USE_VOLATILE
static volatile sig_atomic_t done;        /* volatile 있음 */
#else
static sig_atomic_t done;                 /* volatile 없음 */
#endif

static void on_alarm(int sig)             /* ISR 역할: 플래그만 올린다 */
{
    (void)sig;
    done = 1;
}

int main(void)
{
    signal(SIGALRM, on_alarm);
    alarm(1);                             /* 1초 뒤에 "인터럽트"가 발생한다 */

    while (!done) {
        /* 메인 루프: 인터럽트를 기다리며 다른 일을 하는 중 */
    }
    printf("루프 종료\n");
    return 0;
}
