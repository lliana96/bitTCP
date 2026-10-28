#include <cstdio>
#include "packet.h" //패킷 구조 및 패킷 관련 함수 존재
#include "bit.h"    //BIT 관련 함수 존재
#include "tcp.h"    //TCP 서버 관련 함수 존재


//어떤 활동을 할지 지정하기 위한 함수
//packet.cpp에서 OnReceive가 패킷을 모아서 점검
//정상 패킷일 경우 OnPacket 동작
//상황에 맞는 활동 지정해주는 함수
//현재는 BIT 점검(IBIT, CBIT)만 존재
void OnPacket(const Packet& p)
{
    // BIT 조건 설정 
    // CMD 01 = BIT 점검 
    if (p.cmd == CMD_BIT && !p.data.empty())
    {
        // 요청한 BIT의 저장된 결과를 패킷으로 형성하여 client에게 전송
        BitCheck(p.data[0]);
    }
    else
    {
        if (p.cmd != CMD_BIT)
        {
            printf("[Server] CMD not BIT %02X\n", p.cmd);
        }
        else
        {
            printf("[Server] DATA empty\n");
        }
    }
}


int main()
{
    //연결시 IBIT 무조건 시작 
    IbitRun();

    // CBIT 시작 
    // IBIT 끝나고 1초 후, 2초마다 계속 
    // 반복진행을 위해 main과 따로 스레드로 진행)
    CbitStart();

    // TCP 서버 open
    int sock = TcpServerOpen(PORT);
    if (sock < 0)
    {
        
        return 1;
    }
    else
    {
        //pass
    }


    while (1)
    {
        // 연결될 client를 지정하고
        // 반복적으로 들어오는 packet을 처리
        // ClientLoop가 끝나야 다음 클라이언트를 받음 → 한 번에 1명만 연결
        int fd = TcpServerAccept(sock);
        if (fd < 0)
        {
            continue;
        }
        else
        {
            //pass
        }

        //CBIT FAIL시 즉시 client에게 보내기 위해 저장
        g_client_fd = fd;    
        ClientLoop(fd);
        g_client_fd = -1;    //연결 종료
    }
}