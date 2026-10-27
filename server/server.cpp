#include <cstdio>
#include "packet.h" //패킷 구조 및 패킷 관련 함수 존재
#include "bit.h"    //BIT 관련 함수 존재
#include "tcp.h" 

static int  g_fd     = -1;     // 연결 상태 확인을 위한 전역 변수 (-1 : 연결 X)

//어떤 활동을 할지 지정하기 위한 함수
//packet.cpp에서 onreceive 패킷을 모아서 점검
//정상 패킷일 경우 onpacket동작
//상황에 맞는 활동 지정해주는 함수
//현재는 BIT 점검만 존재
void OnPacket(const Packet& p)
{
    // IBIT 조건 설정 1. CMD 01 = BIT 점검 2. DATA 01 IBIT
    if (p.cmd == CMD_BIT && !p.data.empty() && p.data[0] == IBIT)
    {
        // IBIT 점검 결과를 패킷을 패킷으로 형성하여 client에게 전송
        IbitCheck(g_fd);     
    }
    else
    {
        if (p.cmd != CMD_BIT)
        {
            printf("[Server] CMD not BIT %02X\n", p.cmd);
        }
        else if (p.data.empty())
        {
            printf("[Server] DATA empty\n");
        }
        else if (p.data[0] != IBIT)
        {
            printf("[Server] DATA not IBIT %02X\n", p.data[0]);
        }
        else
        {
            printf("[Server] FAIL \n");
        }
    }
}


int main()
{
    //IBIT 점검
    IbitRun();

    // TCP 서버 open
    int sock = TcpServerOpen(PORT);
    if (sock < 0)
    { 
        //프로그램 전체 종료
        return 1;                          
    }
    else
    {
        //pass
    }


    while (1)
    {
        //연결될 client를 지정하고 
        //반복적으로 들어오는 packet을 처리
        //ClientLoop가 끝나야 다음 클라이언트를 받음 → 한 번에 1명만 연결
        int fd = TcpServerAccept(sock);            
        if (fd < 0) continue;             

        g_fd = fd;                          
        ClientLoop(fd);                    
        g_fd = -1; //연결 종료                         
    }
}