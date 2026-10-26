#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "packet.h"

#define PORT      20164
#define ADDR_BRD  0x01   // 보드 주소 (임시)
#define CMD_BIT   0x01   // BIT 점검 명령

static int g_fd = -1;    // 지금 연결된 클라이언트


// 검사 통과한 패킷의 명령(cmd)가 BIT점검과 동일한 지 체크 
void OnPacket(const Packet& p)
{
    //ParsePacket out: p.addr, p.cmd, p.data
    if (p.cmd == CMD_BIT) // BIT 점검 명령
    {
        //이미 옳다고 점검 끝낸 후라 전송  해당 패킷 잘 왔다고 응답
        vector<uint8_t> resp = MakePacket(ADDR_BRD, CMD_BIT, p.data);  
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
    int sock = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM : TCP 소켓
    if (sock < 0) 
    { 
        perror("socket"); //perror : 에러 출력
        return 1; 
    } 
    else
    {
        printf("[서버] 소켓 생성됨\n");
    }

    int opt = 1;
    //서버를 껐다가 바로 다시 켤 수 있도록 SO_REUSEADDR 옵션 설정
    //TCP 늦게 끊어진 연결 때문에 옛날 데이터와 새연결이 섞이는 문제 방지
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(PORT);
    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0) 
    {
        perror("bind"); 
        return 1; 
    }
    else
    {
        printf("[서버] 바인딩 성공 (포트 %d)\n", PORT);
    }

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
        rxBuf.clear();  

        uint8_t buf[1024];
        //들어오는 데이터 packet단위로 끊기 위해 한줄 붙이고 갯수 만큼 나눠 정상인지 해석 
        while (1)
        {
            int n = recv(fd, buf, sizeof(buf), 0);   
            if (n <= 0) break;                        
            OnReceive(vector<uint8_t>(buf, buf + n)); //들어온 데이터가 정상 패킷인지 점검
        }

        printf("[서버] 연결 끊김\n");
        close(fd);
        g_fd = -1;
    }
}