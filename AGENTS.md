# GitHub 교육용 HTML 작성 규칙

이 저장소에서 새 HTML 교재를 만들거나 기존 교재를 수정할 때는 먼저 다음 문서를 확인한다.
- https://github.com/Yuul-Jung/rule_md/blob/main/web/education-html/STANDARD.md
- https://github.com/Yuul-Jung/rule_md/blob/main/web/education-html/INTEGRATION.md
- 코드: https://github.com/Yuul-Jung/rule_md/blob/main/common/code-style-rules.md
- 도식: https://github.com/Yuul-Jung/rule_md/blob/main/common/flowchart-rules.md

적용 사항:
1. 책장 배너 이미지는 중앙 표준의 hero-bookshelf.jpg와 동일하게 유지한다.
2. 좌측 전체 장 목차와 현재 페이지 절 목차 및 모바일 메뉴를 구현한다.
3. 기존 학습 내용·코드·이미지는 보존하고, 차이가 있는 부분만 수정한다.
4. 수정 시 장별 파일명을 _v01 → _v02로 올리고 이전 생성본은 결과 폴더에서 정리한다.
5. 코드 문법, 내부 링크, 개인정보, 접근성과 브라우저 표시를 검증한다.
6. 확인하지 못한 실행 결과는 미검증으로 표기한다.
7. 원격 문서를 읽을 수 없으면 추정 적용하지 않고 로컬 규칙 사본을 요구한다.

이 저장소 고유 규칙:
- 원고는 `content/*.md`, 예제 소스는 `src/`가 원본이다. 생성된 HTML(`index.html`, `ch*_v*.html`)은 `bash tools/build_site.sh`로만 만든다.
- 교재의 모든 코드 박스는 `src/`의 파일에서 가져온 부분이며, 실행 결과는 `bash tools/run_all.sh`의 실제 출력이다. 추정한 출력을 넣지 않는다.
- 예제는 PC(gcc)에서 가상 장치로 확인한 것이다. 보드에서 실행하지 않은 사항에는 "보드 미검증"을 표시한다.
- MicroBlaze V·AXI 등 특정 하드웨어의 레지스터 주소·동작은 공식 문서로 확인하기 전에는 단정하지 않는다.
- 도면은 `content/figures/*.svg`에 두고 렌더링해 글자·선 겹침을 눈으로 확인한 뒤 게시한다.
- 커밋은 하나의 논리적 목적만 담고, 메시지는 한글로 변경 목적을 쓴다.
