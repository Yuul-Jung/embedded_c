#!/usr/bin/env bash
# 대상(타깃)별 기본 형의 크기와 char 부호를 clang 내장 매크로로 출력한다 (기법 3).
# 사용: bash tools/target_sizes.sh
set -euo pipefail
for triple in x86_64-unknown-linux-gnu thumbv7m-none-eabi riscv32-unknown-elf riscv64-unknown-elf avr msp430; do
  echo "== $triple"
  clang "--target=$triple" -x c -dM -E /dev/null \
    | grep -E "^#define (__SIZEOF_(SHORT|INT|LONG|LONG_LONG|POINTER)__|__UINT32_TYPE__|__CHAR_UNSIGNED__)( |$)" \
    | sort
done
