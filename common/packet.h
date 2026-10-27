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

// CS 계산 ADDR ~ DATA 합
uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end);

// 패킷 생성 
// STX ~ ETX 구조로 포장 
// 끊겨서 들어오거나 하는 것을 대비하여 02시작 03끝 하나의 패킷 생성  
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data);

// 패킷 검사
// 검사 통과하면 out에 칸 전부 담음
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out);

// 16진수 표현
void PrintHex(const char* title, const vector<uint8_t>& buf);

// 데이터 끊겨들어올때 처리 방안
// 데이터 조각을 모아 패킷 단위로 자름
void OnReceive(const vector<uint8_t>& chunk);

// 완성된 패킷 처리 (서버/클라이언트가 각자 만듦)
void OnPacket(const Packet& p);

extern vector<uint8_t> rxBuf;