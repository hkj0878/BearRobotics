# BearRobotics
# Simple ATM Controller(C++)

**기능 흐름**: 카드 삽입 -> PIN 인증 -> 계좌 선택 -> 잔액 조회 / 입금 / 출금 -> 카드 반환
실제 은행 / 하드웨어 연동 없이 Mock 데이터로 동작하며, 추후 확장(인터페이스 추가, 실제 API/기기 연결)을 염두해 둔 구조 입니다.

## 프로젝트 개요

이 프로젝트는 간단한 ATM 로직(컨트롤러)을 C++단일 파일('atm.cpp')로 구현하였습니다.
UI(그래픽/콘솔) 없이 내부 함수 호출 + 테스트 시나리오 출력으로 동작 검증을 수행합니다.

**기능**
- 카드 삽입 / 배출
- PIN 인증(3회 실패 시 카드 잠금)
- 계좌 목록 조회
- 계좌 선택
- 잔액 조회 / 입급 / 출금
- 테스트 시나리오 3종 자동 실행
