#include <cstdio>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include "packet.h"
#include "tcp.h"
#include "bit.h"
#include "fake.h"


int main()
{
    //1. IBIT (켜자마자 1번)
    
   // 실제 점검 생기면 교체 
    printf("\n");
    bool ibit_result = I_Fake();   // 가짜 점검   
    printf("[IBIT] %s\n", ibit_result ? "PASS" : "FAIL");

    printf("\n\n");

    //2. CBIT 시작 (계속, 따로 스레드)
    //IBIT 끝나고 CBIT 시작까지 1초 쉬고, 그 뒤로 2초마다 점검
    //cbit 동작하는 동시에 MAIN동작
    thread cbit(CbitThread);   // thread : 함수를 따로 돌리기 시작
    cbit.detach();             // detach : 끝날 때까지 안 기다림
    printf("[CBIT] start\n");


    //3. TCP 서버 열기 
    int sock = TcpServerOpen(PORT);
  
    if (sock < 0)
    {
        return 1;   // 서버를 못 열면 종료
    }
    else
    {
        //pass
    }


    int rbit_result = -1;   // RBIT 결과 저장 (0=FAIL, 1=PASS, -1=아직 안 함)



    //4. client 접속 받기 (반복)
    while (true)
    {
        int fd = TcpServerAccept(sock);
        if (fd < 0)
        {
            continue;
        }
        else
        {
            //pass
        }

        g_client_fd = fd;   // 연결된 client 소켓 저장
        rxBuf.clear();      // 이전 client가 남긴 데이터 비우기


        //5. client가 끊을 때까지 받기
        uint8_t buf[1024];
        while (true)
        {
            int n = recv(fd, buf, sizeof(buf), 0);   // recv : 올 때까지 기다림
           
            //연결되지 않고 끊김
            if (n <= 0)
            {
                printf("[Server] client disconnected\n");   // 0: 끊음, -1: 에러
                break;
            }
            else
            {
                //pass
            }

            PrintHex("RX", vector<uint8_t>(buf, buf + n));
            rxBuf.insert(rxBuf.end(), buf, buf + n);   // 바구니에 붙이기


            //6. 완성된 패킷 하나씩 꺼내서 처리
            Packet p;
            while (PopPacket(p))
            {
                if (p.cmd != CMD_BIT || p.data.empty())
                {
                    printf("[Server] unknown request\n");
                    continue;
                }
                else
                {
                    //pass
                }

                // DATA[0] = 어떤 BIT인지
                switch (p.data[0])
                {
                    case IBIT:
                        {// 켤 때 저장해 둔 결과 보내기
                        SendBit({IBIT, (uint8_t)(ibit_result ? 0x01 : 0x00)});
                        printf("[IBIT] sent: %s\n", ibit_result ? "PASS" : "FAIL");
                        break;
                    }

                    case CBIT:
                       { // CBIT 스레드가 저장해 둔 결과 보내기
                        SendBit({CBIT, (uint8_t)(g_cbit_result ? 0x01 : 0x00)});
                        printf("[CBIT] sent: %s\n", g_cbit_result ? "PASS" : "FAIL");
                        break;
                    }

                    case SBIT:
                     {   // 60초 걸리니까 따로 돌림
                        // (결과는 SBIT 스레드가 끝날 때 보내고 저장함)
                        printf("[SBIT] request\n");
                        thread sbit(SbitThread);
                        sbit.detach();
                        break;
                    }

                    case RBIT:
                    {
                        bool rbit_result= R_Fake();  // 가짜 점검
                        SendBit({RBIT, (uint8_t)(rbit_result  ? 0x01 : 0x00)});
                        printf("[RBIT] %s\n", rbit_result  ? "PASS" : "FAIL");
                        break;
                    }
                    

                    case BIT_ALL:
                    {
                        // 새로 점검하지 않고, 저장된 결과를 전부 보냄
                        SendBit({IBIT, (uint8_t)(ibit_result ? 0x01 : 0x00)});
                        SendBit({CBIT, (uint8_t)(g_cbit_result ? 0x01 : 0x00)});
                        SendBit({RBIT, (uint8_t)(rbit_result  ? 0x01 : 0x00)});
                        SendBit({SBIT, (uint8_t)(g_sbit_result ? 0x01 : 0x00)});

                        
                        break;
                    }

                    case CBIT_RESTART:
                    {
                        // 멈춰 있을 때만 다시 시작 (돌고 있으면 하나 더 띄우지 않음)
                        if (g_cbit_result)
                        {
                            printf("[CBIT] already running\n");
                        }
                        else
                        {
                            g_cbit_result = true;   // 고장 기록 지우고 다시 시작
                            printf("[CBIT] restart\n\n");
                            thread cbit2(CbitThread);
                            cbit2.detach();
                        }
                        SendBit({CBIT, 0x01});      // 지금 CBIT 동작 중이라고 알림
                        break;
                    }

                    default:
                        printf("[Server] unsupported BIT %02X\n", p.data[0]);
                        break;
                }
            }
        }

        close(fd);
        g_client_fd = -1;   // client 나감
    }
}