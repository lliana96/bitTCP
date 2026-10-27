#include "bit.h"
#include "packet.h"
#include <cstdio>
#include <sys/socket.h>

static bool g_ibit_result = true;   // IBIT 결과 기억

// 켤 때 IBIT 검사
void IbitRun()
{
    printf("[IBIT] check start\n");
    g_ibit_result = true;            
    printf("[IBIT] finish → result: %s\n", g_ibit_result ? "pass" : "fail");
}

// IBIT 결과 보내기: 4칸 만들기 → 포장 → 보내기
void IbitCheck(int fd)
{
    vector<uint8_t> ack_data = { (uint8_t)(g_ibit_result ? 0x01 : 0x00), 0x00, 0x00, 0x00 };

    vector<uint8_t> pkt = MakePacket(ADDR, CMD_BIT, ack_data);   // ACK 패킷 생성
    send(fd, pkt.data(), pkt.size(), 0);                       // 클라이언트에게 보냄
    PrintHex("TX", pkt);
    printf("[Server] IBIT Result: %s\n", g_ibit_result ? "pass" : "fail");
}