#include "bit.h"
#include "packet.h"
#include <cstdio>
#include <thread>
#include <mutex>
#include <chrono>
#include <sys/socket.h>
#include  "fake.h" //가짜 점검용

// main CBIT스레드 둘다 사용하므로 
// 동시에 써도 안전한 atomic 사용 
atomic<int> g_client_fd{-1};

// IBIT 결과 저장 (main만 사용)
static bool g_ibit_result = true;

// CBIT 결과 저장 
// CBIT 스레드로 저장되지만 main이 
// client에 요청에 따라 보낼 때 읽기 때문에 atomic
static atomic<bool> g_cbit_result{true};

// main과 CBIT 스레드가 동시에 보내면 패킷이 섞임
// 뮤텍스로 lock
static mutex g_send_lock;




// 결과 전송
// bitcheck 통해 만들어지 data를 포함한 packet 생성
// 저장된 data [어떤 BIT][결과] 2칸으로 client에게 보내기 위한 것
// client가 요청할 때 전체 결과를 출력하여 최근 pass,fail을 표현하기 위함  
// 이는 client가 호출시 동작한다 그때 CBIT출력과 겹치면 패킷이 손상되어 출력되기 때문에
// 이 함수가 끝날때 까지 CBIT 출력은 대기하기 위해 뮤텍스 사용 
static void SendBit(const vector<uint8_t>& data)
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

    vector<uint8_t> pkt = MakePacket(ADDR, CMD_BIT, data);

    lock_guard<mutex> lock(g_send_lock);              // lock_guard : 이 블록 동안 잠금
    send(fd, pkt.data(), pkt.size(), MSG_NOSIGNAL);   // MSG_NOSIGNAL : client가 끊겨도 서버가 안 죽게
    PrintHex("TX", pkt);
}


// 켤 때 IBIT 검사 (main에서 한 번 부름)
// 지금은 점검 내용이 없어서 PASS로 저장
void IbitRun()
{
    g_ibit_result = true; //알맞는 점검 기능 생길시 삽입 일단은 true로 연결 체크

    // g_ibit_result = I_Fake();  //가짜 점검

    printf("[IBIT] %s\n", g_ibit_result ? "PASS" : "FAIL");
}


// CBIT 스레드가 하는 일
// IBIT가 끝나고 1초후 시작
//  2초마다 장비 상태를 계속 확인
// PASS → FAIL로 바뀌는 순간에는 client가 묻지 않아도 바로 알림
static void CbitThread()
{
    this_thread::sleep_for(chrono::seconds(1));   // sleep_for : 잠깐 쉬기

    int  count  = 0;
    int failcount = 0;   // fail이 연속으로 몇 번 나왔는지 확인하기 위한 것
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(2));

        // bool ok = true;

        bool ok = C_Fake(count); //가짜 점검       

        if (ok)
        {
            failcount =0;
            printf("[CBIT] #%d pass\n", count);
        }
        else
        {
            failcount++;
            printf("[CBIT] #%d FAIL (%d in a row)\n", count, failcount);
        }

        //연속 5번 일시 고장 판단 -> break
        if (failcount >= 5)
        {
                g_cbit_result = false;  
                SendBit({CBIT, 0x00});
                printf("[CBIT] FAIL 5 in a row → stop\n");
                break;
        }
        else 
        {
               //pass
        
        }

        count++;
    }
}


// CBIT
// 단독 스레드로 진행
void CbitStart()
{
    thread t(CbitThread);   // thread : 함수를 따로 돌리기 시작
    t.detach();             // detach : 끝날 때까지 안 기다림
    printf("[CBIT] start\n");
}


// 운용자가 고장 조치 후 CBIT를 다시 시작 (client 요청)
// 이미 돌고 있으면 또 띄우지 않음 (CBIT 두 개가 동시에 돌면 안 되니까)
// 끝나면 지금 CBIT 상태를 client에게 알려줌
static void CbitRestart()
{
    if (g_cbit_result)
    {
        // pass로 돌고 있는데 동작을 재 요청하는 경우 
        // print로 알리고 기존 동작 유지 
        printf("[CBIT] already running\n");
    }
    else
    {
        // fail 이후 재시작 요청
        // 고장 기록 지우고 PASS로 다시 시작
        g_cbit_result = true;   
        printf("[CBIT] restart\n");
        CbitStart();
    }

    SendBit({CBIT, (uint8_t)(g_cbit_result ? 0x01 : 0x00)});
}


// client가 물어본 BIT의 저장된 결과를 알려주기 위함
void BitCheck(uint8_t sel)
{                      //  BIT  결과
    vector<uint8_t> data = {sel, 0x00};

    if (sel == IBIT)
    {
        data[1] = g_ibit_result ? 0x01 : 0x00;
        SendBit(data);
        printf("[Server] IBIT %s\n", g_ibit_result ? "PASS" : "FAIL");
    }
    else if (sel == CBIT)
    {
        data[1] = g_cbit_result ? 0x01 : 0x00;
        SendBit(data);
        printf("[Server] CBIT %s\n", g_cbit_result ? "PASS" : "FAIL");
    }
     else if (sel ==CBIT_RESTART)
     {
        CbitRestart();
     }
    else
    {
        printf("[Server] unsupported BIT %02X\n", sel);
    }
}



