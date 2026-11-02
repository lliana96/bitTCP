#include "fake.h"
#include <cstdlib>   // rand

// ===== 디버깅용 가짜 점검 =====


const bool TEST_IBIT_FAIL = false;

// CBIT 모드
//  0 : 항상 PASS
//  1 : 5번째마다 1번 FAIL         → 연속 아님, 고장 확정 안 됨
//  2 : 4번째부터 계속 FAIL         → #8에서 연속 5번 → 고장 확정, 멈춤
//  3 : FAIL 3번 → PASS 1번 반복    → 연속이 계속 끊겨서 확정 안 됨
//  4 : 랜덤 30% FAIL               → 실제 간헐 고장 흉내
//  5 : 시나리오
//      처음      : PASS 4번 → 연속 FAIL 5번 → 멈춤
//      재시작 후 : 5번째마다 1번 FAIL × 3번 → 연속 FAIL 5번 → 멈춤
const int TEST_CBIT_MODE = 5;


// 몇 번째 CBIT 실행인지 (모드 5에서 처음/재시작 구분용)
static int g_run = 0;

// 가짜 IBIT 결과
bool I_Fake()
{
    return !TEST_IBIT_FAIL;
}


// 가짜 CBIT 결과 
bool C_Fake(int count)
{
    if (TEST_CBIT_MODE == 1)
    {
        return (count % 5 != 4);
    }
    else if (TEST_CBIT_MODE == 2)
    {
        return (count < 4);
    }
    else if (TEST_CBIT_MODE == 3)
    {
        return (count % 4 == 3);    // 0,1,2 FAIL / 3 PASS / 4,5,6 FAIL / 7 PASS ...
    }
    else if (TEST_CBIT_MODE == 4)
    {
        return (rand() % 10 >= 3);  // 30% 확률로 FAIL
    }
    else if (TEST_CBIT_MODE == 5)
    {
        // count가 0이면 CBIT가 새로 시작한 것 (처음 또는 재시작)
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
            return (count % 5 != 4);     // 재시작 후 : #4, #9, #14만 FAIL (3번)
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
