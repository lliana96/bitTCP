#pragma once
#include <cstdint>
#include <vector>
using namespace std;

// 패킷에 필요한 번호
const uint8_t STX = 0x02;        // 시작 표시
const uint8_t ETX = 0x03;        // 끝 표시

const int     PORT = 20164;      // 접속 번호

const uint8_t ADDR    = 0x01;    // 보드 주소
const uint8_t CMD_BIT = 0x01;    // 01 BIT 점검

const uint8_t BIT_ALL      = 0x00;   // 00 저장된 결과 전부 (IBIT, CBIT, SBIT)
const uint8_t IBIT         = 0x01;   // 01 IBIT
const uint8_t RBIT         = 0x02;   // 02 RBIT
const uint8_t SBIT         = 0x04;   // 04 SBIT
const uint8_t CBIT         = 0x08;   // 08 CBIT
const uint8_t CBIT_RESTART = 0x10;   // 10 CBIT 재시작


// 패킷 구조체
//  STX | ADDR | CMD | LEN | DATA | CS | ETX
struct Packet
{
    uint8_t stx;
    uint8_t addr;
    uint8_t cmd;
    uint8_t len;
    vector<uint8_t> data;
    uint8_t cs;
    uint8_t etx;
};

// 보낼 내용을 규칙대로 패킷으로 생성
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data);

// 받은 패킷이 규칙에 맞는지 확인하고 내용을 꺼냄
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out);

// 주고받은 패킷을 눈으로 확인하기 위해 사용
void PrintHex(const char* title, const vector<uint8_t>& buf);

// 바구니(rxBuf)에서 완성된 패킷 하나를 꺼냄 (꺼내면 true, 아직 덜 왔으면 false)
bool PopPacket(Packet& out);

// 받은 데이터 모아두는 바구니
extern vector<uint8_t> rxBuf;