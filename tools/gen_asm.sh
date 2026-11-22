#!/usr/bin/env bash
# RISC-V(rv32imc) 대상으로 clang이 만든 어셈블리를 출력한다 (기법 2, 3).
# 사용: bash tools/gen_asm.sh [O0|O2]   (기본 O2)
set -euo pipefail
cd "$(dirname "$0")/../src"
OPT="${1:-O2}"
for f in test_vol char_sign; do
  echo "### $f.c (-$OPT)"
  clang --target=riscv32-unknown-elf -march=rv32imc -mabi=ilp32 "-$OPT" \
    -Wall -Wextra -ffreestanding -nostdlibinc -Istubs -std=c11 -S \
    -fno-asynchronous-unwind-tables "$f.c" -o -
done
