#include "packet.h"
#include <cstdio>

vector<uint8_t> rxBuf;


// 데이터가 깨지지 않았는지 확인하기 위한 SUM (ADDR ~ DATA 합)
static uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end)
{
    uint8_t sum = 0;
    for (size_t i = start; i < end; i++)
    {
        sum += buf[i];
    }
    return sum;
}


// 보낼 내용을 규칙대로 패킷으로 생성
vector<uint8_t> MakePacket(uint8_t addr, uint8_t cmd, const vector<uint8_t>& data)
{
    vector<uint8_t> pkt = {STX, addr, cmd, (uint8_t)data.size()};
    pkt.insert(pkt.end(), data.begin(), data.end());    // insert : 뒤에 이어 붙이기
    pkt.push_back(CalcChecksum(pkt, 1, pkt.size()));    // push_back : 한 개만 추가
    pkt.push_back(ETX);
    return pkt;
}


// 1. 받은 패킷이 규칙에 맞는지 확인
// 2. 규칙에 어긋나지 않는 경우 내용을 꺼냄
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out)
{
    size_t size = pkt.size();

    // 시작·끝 표시, 최소 길이, LEN과 실제 길이가 맞는가
    if (size < 6 || pkt[0] != STX || pkt[size - 1] != ETX || size != 6u + pkt[3])
    {
        printf("[Packet] bad frame\n");
        return false;
    }
    else
    {
        //pass
    }

    // 중간에 깨져서 들어온 값이 없는가
    uint8_t len = pkt[3];
    if (CalcChecksum(pkt, 1, 4 + len) != pkt[4 + len])
    {
        printf("[Packet] bad checksum\n");
        return false;
    }
    else
    {
        //pass
    }

    out.stx  = pkt[0];
    out.addr = pkt[1];
    out.cmd  = pkt[2];
    out.len  = len;
    out.data.assign(pkt.begin() + 4, pkt.begin() + 4 + len);   // assign : 범위만큼 복사
    out.cs   = pkt[4 + len];
    out.etx  = pkt[size - 1];
    return true;
}


// 주고받은 패킷을 눈으로 확인하기 위해 사용
void PrintHex(const char* title, const vector<uint8_t>& buf)
{
    printf("\n");
    printf("%s: ", title);
    for (size_t i = 0; i < buf.size(); i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}


// 바구니(rxBuf)에서 완성된 패킷 하나를 꺼냄
// 꺼내면 true, 아직 덜 왔으면 false
// 깨진 패킷은 1칸씩 버리면서 다음 STX를 찾음
bool PopPacket(Packet& out)
{
    while (true)
    {
        // 맨 앞이 STX가 아니면 하나씩 버림
        while (!rxBuf.empty() && rxBuf[0] != STX)
        {
            rxBuf.erase(rxBuf.begin());   // erase : 맨 앞 1칸 지우기
        }

        // 패킷 하나가 다 안 왔으면 다음 recv 기다림
        //패킷 길이 만큼 기다림
        if (rxBuf.size() < 4 || rxBuf.size() < 6u + rxBuf[3])
        {
            return false;
        }
        else
        {
            //pass
        }

        size_t total = 6u + rxBuf[3];
        vector<uint8_t> pkt(rxBuf.begin(), rxBuf.begin() + total);

        // 정상이면 바구니에서 빼고 꺼내줌, 아니면 1칸 버리고 다시
        //정상 체크
        if (ParsePacket(pkt, out))
        {
            rxBuf.erase(rxBuf.begin(), rxBuf.begin() + total);
            return true;
        }
        else
        {
            rxBuf.erase(rxBuf.begin());
        }
    }
}