import socket, time

# 서버(보드 자신)에 접속
s = socket.create_connection(('127.0.0.1', 20164))
s.settimeout(0.5)

# 보낼 패킷 목록 (원하는 것만 남기기)
packets = [
    '02 01 01 01 01 04 03',   # IBIT
    '02 01 01 01 08 0B 03',   # CBIT
    '02 01 01 01 00 03 03',   # ALL
    # '02 01 01 01 04 07 03', # SBIT (60초)
    # '02 01 01 01 10 13 03', # CBIT 재시작
]

for h in packets:
    s.send(bytes.fromhex(h))
    print('TX:', h)
    time.sleep(0.3)
    try:
        print('RX:', s.recv(1024).hex(' '))
    except:
        print('RX: (none)')

s.close()
