#pragma once
#include <atomic>
#include <cstdint>
#include <vector>
using namespace std;

//요청오기 전까지 저장
//요청시 꺼내서 줄 변수
extern atomic<int>  g_client_fd;     // 결과를 보낼 client (-1 = 없음)

extern atomic<bool> g_cbit_result;   // CBIT 결과 
extern atomic<int>  g_sbit_result;   // SBIT 마지막 결과 


// [어떤 BIT][결과] 2칸을 client에게 보냄 (main, CBIT, SBIT가 같이 씀)
void SendBit(const vector<uint8_t>& data);

// CBIT 스레드: 2초마다 점검, 연속 5번 FAIL이면 알리고 멈춤
void CbitThread();

// SBIT 스레드: 60초 점검 (그동안 CBIT 멈춤), 끝나면 결과 보냄
void SbitThread();