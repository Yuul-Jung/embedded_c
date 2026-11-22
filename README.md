# 📘 임베디드 C 공통 교과

MCU가 바뀌어도 그대로 쓰는 **임베디드 C 프로그래밍 기법**을 정리한 공통 교과입니다.
C 문법은 알지만 주소·메모리 중심으로 생각해 본 적이 없는 독자를 대상으로 합니다.

- 기초편(A~D): 메모리와 주소, 포인터, 구조체 배치, 함수 포인터
- 기법편: 비트 연산, `volatile`, 고정 폭 정수, 폴링과 타임아웃, ISR 작성 규칙, 초기화 순서, 링 버퍼, 상태 머신, 논블로킹 시간 처리, 계층 분리(HAL)
- 기법 11~15(임계 구역, 구조체·비트필드, 링커 스크립트, 정적 할당, 방어적 코딩)와 부록(실습 시리즈 대응표)은 작성 중입니다.

## 📘 교육용 HTML 공통 제작 규칙

이 저장소의 교육자료(웹페이지·강의 HTML)는 [Yuul-Jung 교육용 HTML 공통 표준](https://github.com/Yuul-Jung/rule_md/tree/main/web/education-html)을 적용합니다.

- [제작 규칙](https://github.com/Yuul-Jung/rule_md/blob/main/web/education-html/STANDARD.md)
- [다른 저장소 적용 안내](https://github.com/Yuul-Jung/rule_md/blob/main/web/education-html/INTEGRATION.md)
- [기준 디자인 사례](https://github.com/Yuul-Jung/ai-agent-engineering)

공통 책장 이미지, 컴팩트한 상단 배너, 좌측 전체 장/절 탐색, 모바일 메뉴, 파일 버전 및 검증 기준을 따릅니다.

## 구성

| 경로 | 설명 |
|---|---|
| `index.html`, `chNN_*_vNN.html` | 생성된 교재(GitHub Pages 진입점은 `index.html`) |
| `content/chNN.md` | 장별 원고(Markdown). `##` 제목이 절이 됩니다 |
| `content/figures/*.svg` | 장별 도면. 원고에서 `{{svg:figures/파일.svg\|캡션}}`으로 삽입합니다 |
| `course.json` | 강좌 제목, 장 목록, 파일명, 버전 |
| `build.py`, `assets/` | 표준 생성기와 디자인 자원(표준 사본 + 로컬 확장, 아래 참고) |
| `src/` | 교재의 모든 예제 소스(C). 교재의 코드 박스는 이 파일에서 가져온 부분입니다 |
| `tools/` | 검증·빌드 스크립트 |

## 검증 방법

```bash
# 모든 예제를 gcc로 빌드(경고를 오류로 처리)하고 실행합니다. 교재의 실행 결과는 이 출력입니다.
bash tools/run_all.sh

# RISC-V(rv32imc) 대상 clang 어셈블리 (기법 2, 3). clang이 필요합니다.
bash tools/gen_asm.sh O2

# 대상별 형 크기 표(기법 3)의 원본 매크로 값. clang이 필요합니다.
bash tools/target_sizes.sh
```

| 항목 | 판정 | 근거 |
|---|---|---|
| 예제 컴파일·실행 (PC, gcc) | 통과 | `tools/run_all.sh` |
| RISC-V 어셈블리·형 크기 (clang) | 통과 | `tools/gen_asm.sh`, `tools/target_sizes.sh` |
| 실제 보드(MicroBlaze V 등) 실행 | **미검증** | 예제는 가상 장치·가상 레지스터로 PC에서 실행 |
| 생성 HTML의 화면·모바일·링크 | 통과 | Playwright로 확인 (아래 점검 기록 참고) |
| GitHub Pages 배포 확인 | **미검증** | 저장소 설정 후 확인 필요 |

## HTML 다시 만들기

```bash
bash tools/build_site.sh   # 가상환경 생성 → mistune 설치 → build.py → 루트에 HTML 복사
```

`build.py`와 `assets/site.css`는 표준 사본에 아래 **로컬 확장**만 더했습니다. 표준 본문은 바꾸지 않았습니다.

- 원고의 `{{svg:경로|캡션}}` 줄을 인라인 SVG 도면으로 삽입(`content/figures/` 아래 파일만 허용)
- 인용문(`>`)을 안내 상자로, 표를 가로 스크롤 래퍼로 감싸기
- `site.css` 12절: 도면(SVG) 색상·글자 클래스

## 수정할 때

1. 원고는 `content/*.md`와 `src/`를 고칩니다. 생성된 HTML은 직접 고치지 않습니다.
2. 코드를 바꾸면 `tools/run_all.sh`를 다시 실행하고, 교재의 실행 결과 블록을 새 출력으로 바꿉니다.
3. 장 파일명 버전(`_v01`)은 내용이 바뀌면 `course.json`에서 올립니다.
4. 실행하지 않은 검증은 **미검증**으로 표시합니다.
