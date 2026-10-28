#pragma once
#include <cstdint>
#include <vector>
using namespace std;

// 패킷에 필요한 번호
const uint8_t STX = 0x02;        // 시작 표시
const uint8_t ETX = 0x03;        // 끝 표시

const int     PORT     = 20164;  // 접속 번호

const uint8_t ADDR = 0x01;   
const uint8_t CMD_BIT  = 0x01;   // 01 BIT 점검

const uint8_t IBIT = 0x01;   // 01 IBIT
const uint8_t RBIT = 0x02;   // 02 RBIT
const uint8_t SBIT = 0x04;   // 04 SBIT
const uint8_t CBIT = 0x08;   // 08 CBIT


// 패킷 구조체
//  STX | ADDR | CMD | LEN | DATA | CS | ETX
struct Packet
{
    uint8_t stx;            //  STX(02)
    uint8_t addr;           
    uint8_t cmd;            //  01 BIT
    uint8_t len;            
    vector<uint8_t> data; 
    uint8_t cs;             
    uint8_t etx;            //  ETX(03)
};

// 보낼 내용을 규칙대로 패킷으로 생성
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data);

// 1. 받은 패킷이 규칙에 맞는지 확인
// 2. 규칙에 어긋나지 않는 경우 내용을 꺼내는 기능도 포함
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out);

// 주고받은 패킷을 눈으로 확인하기 위해 사용
void PrintHex(const char* title, const vector<uint8_t>& buf);

// 들어온 패킷을 검사하여 알맞는 패킷으로 나눔 
void OnReceive(const vector<uint8_t>& chunk);

// 완성된 패킷이 수행할 활동을 함수로 표현 
void OnPacket(const Packet& p);

// 받은 데이터 모아두는 바구니
extern vector<uint8_t> rxBuf;