#include "packet.h"
#include <cstdio>

//체크섬   합으로 들어온데이터가 옳은지 판단
uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end)
{
    uint8_t sum = 0;
    for (size_t i = start; i < end; i++)
        sum += buf[i];
    return sum;
}

//패킷 생성
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data)
{
    vector<uint8_t> pkt;
    pkt.push_back(STX);
    pkt.push_back(addr);
    pkt.push_back(cmd);
    pkt.push_back((uint8_t)data.size());                 // LEN
    pkt.insert(pkt.end(), data.begin(), data.end());     // DATA
    pkt.push_back(CalcChecksum(pkt, 1, pkt.size()));     // ADDR ~ DATA 합
    pkt.push_back(ETX);
    return pkt;
}

// 패킷 해석하기
// 1) 길이/STX/LEN/ETX/체크섬 점검
// 2) 통과하면 ADDR, CMD, DATA를 out에 담고 true
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out)
{
    // 최소 길이:6바이트
    if (pkt.size() < 6)
    {
        printf("[ERROR] Length too short\n");
        return false;
    }
    else
    {
        // pass
    }

    if (pkt[0] != STX)
    {
        printf("[ERROR] Invalid STX\n");
        return false;
    }
    else
    {
        // pass
    }

    uint8_t len = pkt[3];
    if (pkt.size() != 6u + len)
    {
        printf("[ERROR] LEN mismatch\n");
        return false;
    }
    else
    {
        // pass
    }

    if (pkt.back() != ETX)
    {
        printf("[ERROR] Invalid ETX\n");
        return false;
    }
    else
    {
        //pass
    }

    uint8_t cs = CalcChecksum(pkt, 1, 4 + len);
    if (cs != pkt[4 + len])
    {
        printf("[ERROR] Checksum mismatch (calc %02X, recv %02X)\n", cs, pkt[4 + len]);
        return false;
    }
    else
    {
        //pass
    }

    out.addr = pkt[1];
    out.cmd = pkt[2];
    out.data.assign(pkt.begin() + 4, pkt.begin() + 4 + len);
    return true;
}

//16진수 표현
void PrintHex(const char* title, const vector<uint8_t>& buf)
{
    printf("%s: ", title);
    for (uint8_t b : buf) printf("%02X ", b);
    printf("\n");
}

vector<uint8_t> rxBuf;

//데이터 끊겨들어올때 처리 방안
void OnReceive(const vector<uint8_t>& chunk)
{
    PrintHex("RX chunk", chunk);
    rxBuf.insert(rxBuf.end(), chunk.begin(), chunk.end());

    while (true)
    {
        // 1) STX 찾기: 앞에 붙은 쓰레기 제거
        while (!rxBuf.empty() && rxBuf[0] != STX)
            rxBuf.erase(rxBuf.begin());

        // 2) LEN까지(4바이트) 안 왔으면 다음 조각 기다림
        if (rxBuf.size() < 4)
            return;

        // 3) 패킷 전체 길이만큼 안 왔으면 다음 조각 기다림
        size_t total = 6u + rxBuf[3];
        if (rxBuf.size() < total)
            return;

        // 4) 패킷 하나 잘라서 검사
        vector<uint8_t> pkt(rxBuf.begin(), rxBuf.begin() + total);
        Packet p;
        if (ParsePacket(pkt, p))
        {
            printf("  -> Packet OK: ADDR=%02X CMD=%02X LEN=%zu\n", p.addr, p.cmd, p.data.size());
            PrintHex("     DATA", p.data);
            OnPacket(p);                                          // ← 추가: 완성된 패킷 처리
            rxBuf.erase(rxBuf.begin(), rxBuf.begin() + total);   // 처리한 만큼 버퍼에서 삭제
        }
        else
        {
            rxBuf.erase(rxBuf.begin());   // 깨진 패킷: STX 1바이트만 버리고 다시 찾기
        }
    }
}