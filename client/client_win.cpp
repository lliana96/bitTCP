#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include "packet.h"

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "192.168.10.1"   // 보드 IP로 변경해서 사용
#define PORT      20164

static int Packet_Count = 0;     // 받은 응답 패킷 수
static Packet save; // cmd data 저장 (비교용)

//비트 점검을 위해 CMD확인
void OnPacket(const Packet& p)
{
    printf(" [Client] 응답 수신: CMD=%02X\n", p.cmd);
    Packet_Count++;
    save = p; // 받은 패킷 저장 (비교용)
}

int main()
{
    SetConsoleOutputCP(65001);   // 콘솔 한글 깨짐 방지

    // 윈도우 소켓 시작
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("[ client ] WSAStartup 실패\n");
        return 1;
    }
    else
    {
        //pass
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //소켓 생성 실패 시 에러 출력
    if (sock == INVALID_SOCKET)
    {
        printf("[ client ] 소켓 생성 실패 (에러 %d)\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    else
    {
        //pass
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);

    //컴퓨터가 읽을 수 있는 IP주소로 변환
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    // 연결
    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("[ client ] 연결 실패 (에러 %d)\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else
    {
        printf("[ client] 연결됨 %s:%d\n", SERVER_IP, PORT);
    }

    // 대기를 막기 위해 => 응답 대기 시간 1초 설정
    DWORD tv = 1000; // 1초 (밀리초)
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    //패킷 형식 출력
    printf("패킷 형식: STX(02) ADDR CMD LEN DATA CS ETX(03)\n");
    printf("EX) DATA AA BB   : 02 01 01 02 AA BB 69 03\n");
    printf("q = 종료\n");

    string line;

    while (true)
    {
        printf("> ");
        if (!getline(cin, line) || line == "q") break;

        // 입력 → 바이트
        istringstream iss(line);
        vector<uint8_t> tx;
        unsigned int v;
        while ((iss >> hex >> v)) tx.push_back((uint8_t)v);
        if (tx.empty()) continue;

        // 패킷 구조가 옳은 지 ParsePacket로 판단
        Packet chk;
        if (!ParsePacket(tx, chk))
        {
            printf("[client] 규칙에 맞지 않는 패킷, 전송 안 함\n");
            continue;
        }
        else
        {
            //pass
        }

        // 통과한 패킷 그대로 전송
        send(sock, (const char*)tx.data(), (int)tx.size(), 0);
        PrintHex("TX", tx);

        // 받기: 응답 1개 올 때까지 반복 (끊겨 와도 OnReceive가 모음)
        Packet_Count = 0;
        bool closed = false;
        while (Packet_Count < 1)
        {
            uint8_t buf[1024];
            int n = recv(sock, (char*)buf, sizeof(buf), 0);
            if (n > 0)
            {
                OnReceive(vector<uint8_t>(buf, buf + n));
            }
            else if (n == 0)
            {
                printf("[client] 서버와의 연결이 끊어짐\n");
                closed = true;
                break;
            }
            else
            {
                printf("[client] 응답 없음\n");
                break;
            }
        }
        if( Packet_Count >= 1 && save.cmd == chk.cmd && save.data== chk.data)
        {
            printf("[BIT] 정상\n");
        }
        else
        {
            printf("[BIT] 비정상 CMD / DATA 다름\n");
        }
        if (closed) break;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
