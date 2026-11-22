#!/usr/bin/env bash
# 모든 예제를 gcc로 빌드(경고를 오류로 처리)하고 실행한다.
# 사용: bash tools/run_all.sh
set -euo pipefail
cd "$(dirname "$0")/../src"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
CFLAGS="-std=c11 -Wall -Wextra -O0 -Werror"

# 단일 파일 예제: 기초 A~D, 기법 1~9
for t in test_basic test_struct test_func test_bits test_vol test_int test_poll test_isr test_init test_ring test_fsm test_time; do
  echo "== $t"
  gcc $CFLAGS "$t.c" -o "$TMP/$t"
  "$TMP/$t"
done

# 기법 2-6: 읽기 전용 레지스터에 쓰면 컴파일 오류가 나야 정상
echo "== ro_err (컴파일 오류가 나야 정상)"
if gcc -std=c11 -c ro_err.c -o /dev/null 2> "$TMP/err.txt"; then
  echo "실패: 컴파일 오류가 나야 한다" >&2
  exit 1
fi
grep -m1 "read-only" "$TMP/err.txt"

# 기법 5-2: volatile 유무에 따른 플래그 대기 실험 (volatile 없음은 3초 안에 끝나지 않아야 정상)
echo "== vol_signal"
gcc -std=gnu11 -Wall -Wextra -O2 vol_signal.c -o "$TMP/vs_plain"
gcc -std=gnu11 -Wall -Wextra -O2 -DUSE_VOLATILE vol_signal.c -o "$TMP/vs_volatile"
if timeout 3 "$TMP/vs_plain" > /dev/null; then
  echo "volatile 없음: 종료됨 (이 컴파일러/옵션에서는 최적화가 일어나지 않음)"
else
  echo "volatile 없음: 3초 안에 끝나지 않음 (무한 대기)"
fi
timeout 3 "$TMP/vs_volatile"

# 기법 10: 같은 응용 코드(main.c, app.c)에 HAL 구현만 바꿔 두 번 빌드
for impl in hal_regs hal_mock; do
  echo "== hal/$impl"
  gcc $CFLAGS -Ihal hal/main.c hal/app.c "hal/$impl.c" -o "$TMP/$impl"
  "$TMP/$impl"
done
echo "모든 예제 통과"
