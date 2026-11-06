#include "fake.h"
#include <cstdlib>   // rand

// true/false 설정 (true면 FAIL)
const bool TEST_IBIT = true;
const bool TEST_SBIT = true;
const bool TEST_RBIT = true;

// CBIT 모드
//  0 : 항상 PASS
//  1 : 5번째마다 1번 FAIL         → 연속 아님, 고장 확정 안 됨
//  2 : 처음      : PASS 4번 → 연속 FAIL 5번 → 멈춤
//      재시작 후 : 5번째마다 1번 FAIL × 3번 → 연속 FAIL 5번 → 멈춤
// 연속 5번 fail시 멈춤
const int TEST_CBIT_MODE = 2;

// 몇 번째 CBIT 실행인지 (모드 5에서 처음/재시작 구분용)
static int g_run = 0;


// 가짜 IBIT 결과
bool I_Fake()
{
    return TEST_IBIT;
}


// 가짜 SBIT 결과
bool S_Fake()
{
    return TEST_SBIT;
}

bool R_Fake()
{
    return TEST_RBIT;
}

// 가짜 CBIT 결과 
bool C_Fake(int count)
{
    if (TEST_CBIT_MODE ==1)
    {
        return (count % 5 != 4);
    }
   
    else if (TEST_CBIT_MODE == 2)
    {
      
        if (count == 0)
        {
            g_run++;
        }
        else
        {
            //pass
        }

        if (g_run == 1)
        {
            return (count < 4);          // 처음 : PASS 4번 → 계속 FAIL 
        }
        else if (count < 15)
        {
            return (count % 5 != 4);     // 재시작 후 : #4, #9, #14만 FAIL
        }
        else
        {
            return false;                // 그 뒤 계속 FAIL
        }
    }
    else
    {
        return true;
    }
}