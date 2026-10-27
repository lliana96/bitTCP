#include "packet.h"
#include <cstdio>

// CS 계산 ADDR ~ DATA 합
// 합으로 들어온데이터가 옳은지 판단
// ADDR ~ DATA 합 계산 STX와 ETX는 동일하므로 제외
// buf[start] ~ buf[end-1]까지 합산 후 그 값 반환 
uint8_t CalcChecksum(const vector<uint8_t>& buf, size_t start, size_t end)
{
    uint8_t sum = 0;
    for (size_t i = start; i < end; i++)
        sum += buf[i];
    return sum;
}

// 패킷 생성  STX ~ ETX 바이트로 포장
// 지정해 놓은  패킷 STX | ADDR | CMD | LEN | DATA | CS | ETX 순으로 push_back으로 만듦

// CS 계산은 CalcChecksum() 호출
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

// 패킷 해석 
// 1) 길이/STX/LEN/ETX/체크섬 점검
// 2) 통과하면 ADDR, CMD, DATA를 out에 담고 true
// // out: 파싱 결과 상자 → OnPacket에서 명령(cmd) 판단할 때 사용
bool ParsePacket(const vector<uint8_t>& pkt, Packet& out)
{
    // 최소 길이:6바이트
    //STX | ADDR | CMD | LEN | CS | ETX  == data가 없는 경우 최소
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

    //길이가 같은지 점검
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


    //cs: 직접 계산 pkt[4 + len] : 수신한 체크섬
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

    //저장의 이유? : out은 점검을 모두 통과한 애들만 쓰기 위해 사용
    //
    out.stx  = pkt[0];
    out.addr = pkt[1];
    out.cmd  = pkt[2];
    out.len  = len;
    out.data.assign(pkt.begin() + 4, pkt.begin() + 4 + len);
    out.cs   = pkt[4 + len];
    out.etx  = pkt.back();
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
//데이터 조각을 모아 패킷 단위로 자름
//데이터가 잘못 들어와도 패킷 단위로 찾을 수 있게 도와줌
void OnReceive(const vector<uint8_t>& chunk)
{
    PrintHex("RX ", chunk);
    //끊겨서 들어올수 있으니 오는 족족 rxBuf에 붙임 (한줄)
    rxBuf.insert(rxBuf.end(), chunk.begin(), chunk.end());

    while (true)
    {
        // 1) STX 찾기: 앞에 붙은 쓰레기 제거
        while (!rxBuf.empty() && rxBuf[0] != STX)
            rxBuf.erase(rxBuf.begin()); //erase 제거 함수

        // 2) LEN까지(4바이트) 안 왔으면 다음 조각 기다림
        if (rxBuf.size() < 4) //최소 4바이트: STX | ADDR | CMD | LEN 
            return;

        // 3) 패킷 전체 길이만큼 안 왔으면 다음 조각 기다림
        size_t total = 6u + rxBuf[3]; // LEN + 6(STX,ADDR,CMD,LEN,CS,ETX) 
                                    //데이터의 길이를 더해 리얼 패킷 길이 통과 위한 기다림
        if (rxBuf.size() < total)
            return;

        // 4) 패킷 하나 잘라서 검사
        // 처음 -> total까지 자른다. (total: 6 + LEN)  
        // 그 하나가  하나의 패킷인지 보기 위해
        vector<uint8_t> pkt(rxBuf.begin(), rxBuf.begin() + total);
        Packet p;

        //만들어진 패킷 정상 해석 (점검)
        if (ParsePacket(pkt, p))
        {
            printf("  -> Packet OK: ADDR=%02X CMD=%02X LEN=%zu\n", p.addr, p.cmd, p.data.size());
            PrintHex("     DATA", p.data);
            OnPacket(p);                                         
            rxBuf.erase(rxBuf.begin(), rxBuf.begin() + total);   // 처리한 만큼 버퍼에서 삭제
        }
        else
        {
            rxBuf.erase(rxBuf.begin());   // 해석에 통과하지 못하면 
                                          // 깨진 패킷: STX 1바이트만 버리고 다시 찾기
        }
    }
}

