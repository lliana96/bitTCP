#pragma once
#include <cstdint>
#include <vector>
using namespace std;

const uint8_t STX = 0x02;
const uint8_t ETX = 0x03;

// 변동되는 것 모아서 구조체 형성
//  STX | ADDR | CMD | LEN | DATA | CS | ETX
struct Packet
{
    uint8_t addr;
    uint8_t cmd;
    vector<uint8_t> data;
};

// CS 계산 ADDR ~ DATA 합
uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end);

// 패킷 생성  STX ~ ETX 바이트로 포장
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data);

// 패킷 해석 
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out);

//16진수 표현
void PrintHex(const char* title, const vector<uint8_t>& buf);

//데이터 끊겨들어올때 처리 방안
//데이터 조각을 모아 패킷 단위로 자름
void OnReceive(const vector<uint8_t>& chunk);

//완성된 패킷 처리
void OnPacket(const Packet& p); 

extern vector<uint8_t> rxBuf;