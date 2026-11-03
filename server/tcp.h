#pragma once

// TCP 서버 준비: 소켓 생성 → 주소(IP+포트) 연결 → 접속 대기
// 성공하면 소켓 번호, 실패하면 -1
int TcpServerOpen(int port);

// client 접속 받기: 접속이 올 때까지 기다림
// 성공하면 그 client 번호(fd), 실패하면 -1
int TcpServerAccept(int sock);