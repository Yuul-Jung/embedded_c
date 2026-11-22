/* hal.h: 응용과 드라이버가 보는 하드웨어 인터페이스 (주소와 CPU 명령이 나오지 않는다) */
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

void hal_led_write(uint32_t mask);   /* LED 출력 값을 쓴다 (비트 n = LED n) */
void hal_report(void);               /* 시험용: 결과를 출력한다 */

#endif
