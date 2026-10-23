#pragma once
#include <cstdint>
#include <vector>
using namespace std;

const uint8_t STX = 0x02;
const uint8_t ETX = 0x03;

struct Packet
{
    uint8_t addr;
    uint8_t cmd;
    vector<uint8_t> data;
};

uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end);
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data);
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out);
void PrintHex(const char* title, const vector<uint8_t>& buf);
void OnReceive(const vector<uint8_t>& chunk);
void OnPacket(const Packet& p);   // ← 추가: 완성된 패킷 처리 (서버/클라이언트가 각자 구현)

extern vector<uint8_t> rxBuf;