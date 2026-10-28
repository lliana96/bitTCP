#include "packet.h"
#include <cstdio>

vector<uint8_t> rxBuf;


// 데이터가 깨지지 않았는지 확인하기 위한 SUM
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
    pkt.push_back(CalcChecksum(pkt, 1, pkt.size()));    // push_back: 한개만  추가 가능 
    pkt.push_back(ETX);
    return pkt;
}


// 1. 받은 패킷이 규칙에 맞는지 확인
// 2. 규칙에 어긋나지 않는 경우 내용을 꺼내는 기능도 포함
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out)
{
    size_t size = pkt.size();

    // 점검 1. 정해진 시작과 끝이 일치하는 가
    // 점검 2. 최소 길이는 넘는 가 
    if (size < 6 || pkt[0] != STX || pkt[size - 1] != ETX || size != 6u + pkt[3])
    {
        printf("[Packet] bad frame\n");
        return false;
    }
    else
    {
        //pass
    }

    // sum을 통해 중간에 깨져서 들어온 값이 없는 지 점검
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

    //옳은 패킷의 경우 활용 하기 위해 저장
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
    printf("%s: ", title);
    for (size_t i = 0; i < buf.size(); i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}


// 들어온 패킷을 검사하여 알맞는 패킷으로 나눔 
void OnReceive(const vector<uint8_t>& chunk)
{
    PrintHex("RX", chunk);
    rxBuf.insert(rxBuf.end(), chunk.begin(), chunk.end());

    while (true)
    {
        // 맨 앞이 이상할 경우 앞에서 부터 하나씩 버림
        // 제대로 들어온 시점부터 사용   
        while (!rxBuf.empty() && rxBuf[0] != STX)
        {
            rxBuf.erase(rxBuf.begin());   // erase : 맨 앞 1칸 지우기
        }

        // 최소 수보다 적을 경우 대기
        if (rxBuf.size() < 4 || rxBuf.size() < 6u + rxBuf[3])
        {
            return;
        }
        else
        {
            //pass
        }

        size_t total = 6u + rxBuf[3];
        vector<uint8_t> pkt(rxBuf.begin(), rxBuf.begin() + total);

        // 정상이면 처리후 바구니에서 그만큼 제거, 
        // 아니면 1칸 버리고 다시 찾기
        Packet p;
        if (ParsePacket(pkt, p))
        {
            OnPacket(p); //정상 패킷 처리 함수
            rxBuf.erase(rxBuf.begin(), rxBuf.begin() + total);
        }
        else
        {
            rxBuf.erase(rxBuf.begin());
        }
    }
}