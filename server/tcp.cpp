#include "tcp.h"
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// TCP 서버 준비: 소켓 생성 → 주소(IP+포트) 연결 → 접속 대기
// 성공하면 소켓 번호, 실패하면 -1
int TcpServerOpen(int port)
{
    // 소켓 만들기 (SOCK_STREAM = TCP)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return -1;
    }
    else
    {
        //pass
    }

    // 서버를 껐다 바로 켜도 같은 포트를 다시 쓰려고
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 보드의 어떤 IP로든 port번으로 오는 접속을 받도록 주소 지정
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)   // bind : 소켓에 주소 붙이기
    {
        perror("bind");
        close(sock);
        return -1;
    }
    else
    {
        //pass
    }

    listen(sock, 1);   // listen : 접속 받을 준비 (대기 1명)
    printf("[Server] listening on port %d\n\n", port);
    return sock;
}


// client 접속 받기: 접속이 올 때까지 기다림
// 성공하면 그 client 번호(fd), 실패하면 -1
int TcpServerAccept(int sock)
{
    sockaddr_in cli{};
    socklen_t cliLen = sizeof(cli);

    int fd = accept(sock, (sockaddr *)&cli, &cliLen);   // accept : 접속 올 때까지 기다림
    if (fd < 0)
    {
        perror("accept");
        return -1;
    }
    else
    {
        printf("[Server] client connected: %s\n", inet_ntoa(cli.sin_addr));
    }
    return fd;
}