#pragma once

// 켤 때 IBIT 검사 (main에서 한 번 부름)
void IbitRun();

// IBIT 결과 보내기: 4칸 만들기 → 포장 → fd(client)에게 보내기
void IbitCheck(int fd);