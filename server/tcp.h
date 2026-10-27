#pragma once

// TCP 서버 준비 함수: 소켓 생성 → 주소(IP+포트) 연결 → 접속 대기
// 성공하면 소켓 번호, 실패하면 -1 반환
int TcpServerOpen(int port);


// client 접속 받기: 접속이 올 때까지 기다림
// 대기 줄에서 먼저 온 client부터 받음
// 성공하면 그 client 번호 :  fd, 실패 :  -1 반환
int TcpServerAccept(int sock);


// client가 보내는 데이터를 받는 함수 
// 데이터를 OnReceive로 넘겨서 패킷단위로 만듦 
// 정상 시 -> OnPacket에서 알맞는 행동 지정
void ClientLoop(int fd);