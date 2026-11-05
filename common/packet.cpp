#include "packet.h"
#include <cstdio>

vector<uint8_t> rxBuf;


// 데이터가 깨지지 않았는지 확인하기 위한 SUM (ADDR ~ DATA 합)
// STX와 ETX는 변화가 없기 때문에 제외하고 ADDR부터 DATA 까지의 합
static uint8_t Checksum(vector<uint8_t> buf, int len)
{
    uint8_t sum = 0;
    for (int i = 1; i < 4 + len; i++)  // addr, cmd, len, data (DATA 앞 4칸이라 +4)
    {
        sum += buf[i];
    }
    return sum;
}


// 보낼 내용을 정해진 모양을 갖추게 해주는 함수
// 패킷의 기본 구조
// STX | ADDR | CMD | LEN | DATA | CS | ETX
// 시작  받는곳  분류   개수   내용   합    끝
vector<uint8_t> MakePacket( uint8_t cmd, vector<uint8_t> data)
{
    vector<uint8_t> pkt = {STX, ADDR, cmd, (uint8_t)data.size()};       //pkt 앞에 4개 STX,ADDR은 헤더에서 정의된거 그대로 불러오기
    pkt.insert(pkt.end(), data.begin(), data.end());                    // insert : 뒤에 이어 붙이기(여러 개 붙이기 가능)
    pkt.push_back(Checksum(pkt, data.size()));  // 데이터 합 계산하여 추가    // push_back : 한 개만 추가
    pkt.push_back(ETX);
    return pkt;
}



// 주고받은 패킷을 눈으로 확인하기 위해 사용
//"title : 16진수 " 
//title = > RX ,TX
void PrintHex(string title, vector<uint8_t> buf)
{
    printf("\n");         
    printf("%s: ", title.c_str());
    for (int i = 0; i < buf.size(); i++)
    {
        printf("%02X ", buf[i]); //16진수 
    }
    printf("\n");
}


// 바구니(rxBuf)에 다 모인 패킷들을 꺼내 점검하여 옳은 패킷 일 경우 저장
// 꺼내면 true (out에 담김), 아직 덜 왔으면 false
// 망가진 조각은 1칸씩 버리면서 STX를 찾음
bool PopPacket(Packet& out)
{
    while (true)
    {
        // 1. 맨 앞이 STX가 아니면 패킷 구조와 맞지 않으니 하나씩 버림
        while (!rxBuf.empty() && rxBuf[0] != STX)
        {
            rxBuf.erase(rxBuf.begin());
        }

        // 2. 패킷 한 통(6 + LEN칸)이 다 안 왔으면 다음 응답 기다림
        //recv 상대방이 보낸 데이터를 받아오는 함수
        if (rxBuf.size() < 4 || rxBuf.size() < 6 + rxBuf[3])
        {
            return false;
        }
        else
        {
            //pass
        }

        int len   = rxBuf[3]; 
        int total = 6 + len;

        // 3. 끝 표시가 ETX인가 (아니면 1칸 버리고 다시)
        if (rxBuf[total - 1] != ETX)
        {
            printf("[Packet] bad ETX\n");
            rxBuf.erase(rxBuf.begin());
            continue;
        }
        else
        {
            //pass
        }

        // 4. 합이 맞는지 (아니면 1칸 버리고 다시)
        if (Checksum(rxBuf, len) != rxBuf[4 + len])
        {
            printf("[Packet] bad checksum\n");
            rxBuf.erase(rxBuf.begin());
            continue;
        }
        else
        {
            //pass
        }

        // 5. 통과시 다음 사용을 위해 칸별로 out에 담음
        out.stx  = rxBuf[0];
        out.addr = rxBuf[1];
        out.cmd  = rxBuf[2];
        out.len  = len;
        out.data.assign(rxBuf.begin() + 4, rxBuf.begin() + 4 + len);   // assign : 시작과 끝 범위 부분을 복사 
        out.cs   = rxBuf[4 + len];
        out.etx  = rxBuf[total - 1];

        // 6. 꺼낸 만큼 바구니에서 지움
        rxBuf.erase(rxBuf.begin(), rxBuf.begin() + total);
        return true;
    }
}
