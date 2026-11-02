#pragma once

// ===== 디버깅용 가짜 점검 =====

// 가짜 IBIT 결과 (true = PASS)
bool I_Fake();

// 가짜 CBIT 결과 (count = 몇 번째 점검인지, true = PASS)
bool C_Fake(int count);