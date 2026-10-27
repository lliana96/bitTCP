#include "tcp.h"
#include "packet.h"      
#include <cstdio>
#include <unistd.h>      
#include <arpa/inet.h>   
#include <sys/socket.h>  

// TCP 서버 준비 함수: 소켓 생성 → 주소(IP+포트) 연결 → 접속 대기
// 성공하면 소켓 번호, 실패하면 -1 반환
int TcpServerOpen(int port)
{   
    //socket 생성
    //TCP : SOCK_STREAM  
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 소켓 생성 실패 시 -1 반환 -> main에서 프로그램 졸료
    if (sock < 0)
    {
        perror("socket");   // perror : 실패 이유까지 출력
        return -1;
    }
    else
    {
        printf("[server] socket created\n");
    }

    // 서버를 껐다가 바로 다시 켤 수 있도록 SO_REUSEADDR 옵션 설정
    // TCP 늦게 끊어진 연결 때문에 옛날 데이터와 새 연결이 섞이는 문제 방지
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //주소 구조체 초기화 후 채움 
    sockaddr_in addr{};                              
    addr.sin_family      = AF_INET;                 //IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY);       //IP
    addr.sin_port        = htons(port);             //port 번호
    //원하는 TCP 통신 주소 할당 (주소는 위에서 구조체로 지정)
    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)   
    {
        perror("bind");
        close(sock); 
        return -1;
    }
    else
    {
        printf("[server]  bind pass (port %d)\n", port);
    }

    //통신 대기 상태
    listen(sock, 1);                                  
    printf("[server] waiting (port %d)\n", port);
    return sock;
}

// client 접속 받기: 접속이 올 때까지 기다림
// 대기 줄에서 먼저 온 client부터 받음
// 성공하면 그 client 번호 :  fd, 실패 :  -1 반환
int TcpServerAccept(int sock)
{
    sockaddr_in cli{};                               
    socklen_t cliLen = sizeof(cli); //listen이 결정 

    int fd = accept(sock, (sockaddr *)&cli, &cliLen);   
    if (fd < 0) 
    { 
        perror("accept");
        return -1; 
    }
    else
    {
        printf("[server] accept pass\n");
    }
    return fd;
}

// client가 보내는 데이터를 받는 함수 
// 데이터를 OnReceive로 넘겨서 패킷단위로 만듦 
// 정상 시 -> OnPacket에서 알맞는 행동 지정
void ClientLoop(int fd)
{
    // rxBuf  받은 데이터 바구니
    // packet.cpp 에 존재  
    rxBuf.clear();    // 이전 데이터 비우기

    uint8_t buf[1024];
    while (1)
    {
        //받은 데이터를 buf에 저장 
        int n = recv(fd, buf, sizeof(buf), 0);   
        // 무한 루프 탈출구 
        // n은 받은 데이터 개수를 의미한다.
        // n이 0 이거나 그거보다 작다는 것은 더이상 데이터를 받을 수 없는 상태라는 것 의미
        // 원인을 알고 나가지기위해 추가
        // 무한 반복(while)의 출구
        if (n == 0)
        {
            printf("[Server] Client disconnected\n");   // client가 끊음
            break;
        }
        else if (n < 0)
        {
            perror("recv");                            // 에러 (실패 이유까지 출력)
            break;
        }
        else
        {
            //pass
        }                      
        //모아서 packet 단위로 나누기 위해 함수 호출
        OnReceive(vector<uint8_t>(buf, buf + n)); 
    }

    close(fd);                             
}