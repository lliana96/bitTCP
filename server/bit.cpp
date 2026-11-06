#include "bit.h"
#include "packet.h"
#include "fake.h"
#include <cstdio>
#include <thread>
#include <mutex>
#include <chrono>
#include <sys/socket.h>

atomic<int>  g_client_fd{-1};


atomic<bool> g_cbit_result{true};
atomic<int>  g_sbit_result{-1};


// 보내기 열쇠: main·CBIT·SBIT가 동시에 보내서 패킷이 섞이지 않게
static mutex g_send_lock;

// 점검 열쇠: SBIT가 60초 동안 잡고 있으면 CBIT는 기다림 (= 일시정지)
static mutex g_check_lock;



// [어떤 BIT][결과] 2칸을 client에게 보냄
// client가 없으면 보낼 곳이 없으니 그냥 끝냄
void SendBit(const vector<uint8_t>& data)
{
    int fd = g_client_fd;
    if (fd < 0)
    {
        return;
    }
    else
    {
        //pass
    }

    vector<uint8_t> pkt = MakePacket(CMD_BIT, data);

    lock_guard<mutex> lock(g_send_lock);              // lock_guard : 이 블록 끝날 때까지 잠금
    
    send(fd, pkt.data(), pkt.size(), MSG_NOSIGNAL);   // MSG_NOSIGNAL : client가 끊겨도 서버가 안 죽게
    PrintHex("TX", pkt);
}


// CBIT 스레드
// 1초 뒤 시작, 2초마다 점검
// FAIL이 연속 5번이면 고장 확정 → client에게 바로 알리고 멈춤
void CbitThread()
{
    //IBIT끝나고 1초 쉬기
    this_thread::sleep_for(chrono::seconds(1));   // sleep_for : 잠깐 쉬기

    int count     = 0;   // 몇 번째 점검인지
    int failcount = 0;   // 연속 FAIL 횟수

    while (true)
    {
         // 실제 점검 생기면 교체 
        this_thread::sleep_for(chrono::seconds(2));

        // SBIT동작시 CBIT는 일시정지
        lock_guard<mutex> lock(g_check_lock);

        // 가짜 점검 (실제 점검 생기면 교체)
        bool ok = C_Fake(count);  

        
        if (ok)
        {
            failcount = 0;
            printf("[CBIT] #%d PASS\n", count);
        }
        else
        {
            failcount++;
            printf("[CBIT] #%d FAIL (%d in a row)\n", count, failcount);
        }

        // 연속 5번 FAIL → 고장 확정, 알리고 멈춤
        if (failcount >= 5)
        {
            g_cbit_result = false;
            SendBit({CBIT, 0x00});
            printf("\n[CBIT] FAIL 5 in a row → stop\n\n");
            break;
        }
        else
        {
            //pass
        }

        count++;
    }
}


// SBIT 스레드
// 점검 열쇠를 60초 동안 잡고 점검 → 그동안 CBIT 자동 정지
// 끝나면 결과 보내고 열쇠 반납 → CBIT 자동 재개
void SbitThread()
{
    lock_guard<mutex> lock(g_check_lock);
    printf("[SBIT] start (CBIT paused)\n");

    this_thread::sleep_for(chrono::seconds(60));   // 점검에 걸리는 시간
    bool ok = S_Fake();                            // 가짜 점검 (실제 점검 생기면 교체)
    g_sbit_result = ok ? 1 : 0;                    // 결과 저장 (ALL 요청 때 꺼내 쓰려고)


    SendBit({SBIT, (uint8_t)(ok ? 0x01 : 0x00)});
    printf("[SBIT] %s (CBIT resume)\n", ok ? "PASS" : "FAIL");
}
