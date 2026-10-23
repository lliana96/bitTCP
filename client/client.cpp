#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "packet.h"

#define SERVER_IP "127.0.0.1"   // 나중에 노트북에서 보드 IP로 변경
#define PORT      20164

// 응답 패킷이 완성되면 OnReceive가 불러줌
void OnPacket(const Packet& p)
{
    printf("[클라] 응답 수신: CMD=%02X\n", p.cmd);
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    // 연결 성공 = 정상, 실패 = 비정상
    if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        printf("[클라] 비정상 (연결 실패)\n");
        return 1;
    }
    printf("[클라] 정상 (연결됨 %s:%d)\n", SERVER_IP, PORT);

    // 응답 안 오면 1초 후 다음 입력으로 (없으면 잘못된 패킷 보낼 때 멈춤)
    timeval tv{1, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 지켜야 할 패킷 형식 안내
    printf("패킷 형식: STX(02) ADDR CMD LEN DATA... CS ETX(03)\n");
    printf("  LEN = DATA 개수, CS = ADDR~DATA 합(하위 1바이트)\n");
    printf("  예) BIT 요청: 02 01 01 00 02 03\n");
    printf("16진수 입력 (q = 종료)\n");

    std::string line;
    while (true)
    {
        printf("> ");
        if (!std::getline(std::cin, line) || line == "q") break;

        // 입력한 16진수 → 바이트
        vector<uint8_t> tx;
        std::istringstream iss(line);
        unsigned int v;
        while (iss >> std::hex >> v) tx.push_back((uint8_t)v);
        if (tx.empty()) continue;

        // 보내기
        send(sock, tx.data(), tx.size(), 0);
        PrintHex("TX", tx);

        // 받기
        uint8_t buf[1024];
        int n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0)       OnReceive(vector<uint8_t>(buf, buf + n));
        else if (n == 0) { printf("[클라] 서버가 연결을 끊음\n"); break; }
        else             printf("[클라] 응답 없음\n");
    }

    close(sock);
    return 0;
}