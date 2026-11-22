/* const volatile 레지스터에 쓰면 컴파일 오류가 난다 (오류를 확인하려는 의도적 오류) */
#include <stdint.h>

void bad(const volatile uint32_t *status)
{
    *status = 0u;
}
