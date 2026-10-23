#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "packet.h"

#define PORT      20164
#define ADDR_BRD  0x01   // 보드 주소 (임시)
#define CMD_BIT   0x01   // BIT 점검 명령

static int g_fd = -1;    // 지금 연결된 클라이언트

// 완성된 패킷이 들어오면 OnReceive가 불러주는 함수
void OnPacket(const Packet& p)
{
    if (p.cmd == CMD_BIT)
    {
        vector<uint8_t> resp = MakePacket(ADDR_BRD, CMD_BIT, {0x00});   // 0x00 = 정상
        send(g_fd, resp.data(), resp.size(), 0);
        PrintHex("TX", resp);
        printf("[서버] BIT 응답 전송 (정상)\n");
    }
    else
    {
        printf("[서버] 알 수 없는 CMD %02X\n", p.cmd);
    }
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(PORT);
    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }

    listen(sock, 1);
    printf("[서버] 대기중 (포트 %d)\n", PORT);

    while (1)
    {
        sockaddr_in cli{};
        socklen_t cliLen = sizeof(cli);
        int fd = accept(sock, (sockaddr *)&cli, &cliLen);
        if (fd < 0) { perror("accept"); continue; }
        printf("[서버] 연결됨: %s\n", inet_ntoa(cli.sin_addr));

        g_fd = fd;
        rxBuf.clear();   // 이전 연결에서 남은 데이터 비우기

        uint8_t buf[1024];
        while (1)
        {
            int n = recv(fd, buf, sizeof(buf), 0);   // 들어온 만큼 받기
            if (n <= 0) break;                        // 0: 끊김, -1: 에러
            OnReceive(vector<uint8_t>(buf, buf + n)); // 조각을 OnReceive에 넘김
        }

        printf("[서버] 연결 끊김\n");
        close(fd);
        g_fd = -1;
    }
}