![header](https://capsule-render.vercel.app/api?type=waving&color=gradient&height=280&section=header&text=Computer%20Structures&fontSize=70&fontColor=ffffff&fontAlign=50&fontAlignY=45)

## 컴퓨터구조 프로젝트
컴퓨터구조 프로젝트는 중앙처리장치(CPU)의 `싱글 사이클`, `파이프라인`, 그리고 `캐시`를 C로 시뮬레이션하는 `세 가지 프로그램`을 개발합니다. 명령어집합구조(ISA)를 처리하는 싱글 사이클, 파이프라인 마이크로아키텍처를 설계하고 CPU의 성능을 개선할 방법을 분석합니다. 또한, 캐시 계층구조를 파악하고 캐시의 효율성을 높일 방법을 분석합니다.
<br>

## 🔎 프로필
- 이름 `컴퓨터구조 프로젝트`
- 기간 `2021-03 ~ 2021-06`
- 인원 `1명`
<br>

## 💻  개발 환경
- 언어 `C`
- 통합개발환경(IDE) `Visual Studio 2019`
- 운영체제(OS) `Windows 10`
<br>

## 🛠️ ️기술
- `C`
<br>

## 📜 내용
#### 1. Single Cycle MIPS Emulator
- 싱글 사이클 마이크로아키텍처 시뮬레이터 개발
- 싱글 사이클 데이터 패스, 제어 신호 설계 후 구현
- MIPS 32 명령어 집합 구조의 26가지 명령어 처리
- [Single Cycle MIPS Emulator 프로젝트 기술서](https://drive.google.com/file/d/1zyk8-MizmJOe0FOyVeie-xuXMom2Rkfg/view?usp=sharing)

#### 2. Pipelined MIPS Emulator
- 파이프라인 마이크로아키텍처 시뮬레이터 개발
- 파이프라인 데이터 패스, 제어 신호 설계 후 구현
- 데이터 종속성을 해결하고 명령어 처리량 개선
- [Pipelined MIPS Emulator 프로젝트 기술서](https://drive.google.com/file/d/1GjLizVIn7MX95kGoVE_-u4c7f7VCmu34/view?usp=sharing)

#### 3. Single Cycle MIPS Emulator with Cache
- 캐시를 도입한 싱글 사이클 마이크로아키텍처 시뮬레이터 개발
- 직접 사상 방식, 연관 사상 방식 구현
- 쓰기 정책, SCA 교체 정책 구현 
- [Single Cycle MIPS Emulator with Cache 프로젝트 기술서](https://drive.google.com/file/d/19drSg42HgVjcu7jO4sz7axk5QgKDQUi9/view?usp=sharing)
<br>

## ⓒ 2021-2023. Tak Junseok all rights reserved.
이 리포지토리에 기재된 코드와 리포트에 대한 저작권은 탁준석과 단국대학교 모바일시스템공학과 유시환 교수에게 있습니다. 사전에 합의되지 않은 내용을 무단으로 도용(URL 연결 등), 불법으로 복사(캡처)하여 사용할 경우 사전 경고 없이 저작권법에 의한 처벌을 받을 수 있습니다.

The copyright for the codes and reports listed in this repository belongs to Tak Junseok and Yoo Sihwan, a professor of Mobile System Engineering at Dankook University. If you steal (such as URL connection) or illegally copy (capture) anything that is not agreed in advance, you may be punished by copyright law without prior warning.
