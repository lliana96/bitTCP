#pragma once
#include <atomic>
#include <cstdint>
using namespace std;

// 결과·FAIL을 보낼 client (-1 = 없음, main이 접속/끊김 때 바꿈)
extern atomic<int> g_client_fd;

// 켤 때 IBIT 검사 (main에서 한 번 부름)
void IbitRun();


// CBIT
// IBIT가 끝나고 1초후 장비가 켜져있는 동안 계속 상태를 지켜보기 위한 것
// CBIT 2초마다 수행될때 마다 main도 동작되어야하기 때문에 스레드로 진행
void CbitStart();

// client가 물어본 BIT의 저장된 결과를 알려주려고
void BitCheck(uint8_t sel);