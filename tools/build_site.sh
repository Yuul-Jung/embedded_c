#!/usr/bin/env bash
# 원고(content/*.md)에서 HTML을 만들어 저장소 루트에 복사한다 (GitHub Pages: 루트 게시).
# 사용: bash tools/build_site.sh
set -euo pipefail
cd "$(dirname "$0")/.."
python3 -m venv .venv
. .venv/bin/activate
python -m pip install --quiet mistune
python build.py
# 구버전 장 HTML을 지우고 최신 생성본만 루트에 둔다 (원고·소스·Git 이력은 건드리지 않는다)
rm -f ./index.html ./ch[0-9][0-9]_*_v[0-9][0-9].html
cp dist/*.html .
echo "루트 HTML 갱신 완료"
