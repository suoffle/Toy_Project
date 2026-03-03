# Network

### __요약:__
- Server-Client 구조의 숫자 야구 게임을 구현하였다.  
- Server는 select() 기반 다중 접속 TCP 서버이다.  
- Client가 3개의 숫자를 선택하는 방식을 process와 thread를 활용하여 구현하였다.  
  - 0-9 까지의 인자를 갖는 process/thread 10개를 생성하고 경쟁시켜 랜덤한 숫자 3개를 선택하게 하였다.  
  - 선택된 숫자 3개는 배열에 저장되며, 동시성 문제 해결을 위해 mutex를 사용하였다.  

=> TCP 통신, mutex, process, thread를 활용한 프로젝트 구현.
---
#1 Server.c:
- set_answer() -> 게임의 정답(3자리 숫자) 생성.  
- socket(), bind() -> 서버 소켓 생성 및 포트 번호와 소켓 연결.  
- listen() -> 연결 요청 client 대기.  
- select(), accept() -> client 감시, 연결 수락.  
- client로부터 수신된 정보 파싱 및 응답 전송(broadcast)  
- check_answer() -> clinet의 정답 확인(strike, ball, answer)  
- 연결 종료 처리  

#2 Client.c:
- socket(), connect() -> clinet 소켓 생성 및 서버에 연결 요청  
- client 닉네임 입력.    
- 숫자 생성 방법 -> 'thread 인자'로 0-9 값을 갖는 10개의 thread를 생성하여 먼저 수행되는 thread 3개를 선택한다.  
- server에 선택한 수를 보내고 응답을 받는다.  

#3 threadtime.c: 
- 10개의 thread(0-9)가 랜덤한 시간 동안 대기한 후, 경쟁하여 mutex를 획득한다. 가장 먼저 도착한 3개의 thread 번호를 배열에 기록한다. (pthread_create()의 인자 사용)

#4 processtime.c: 
- 0-9까지 총 10개의 자식 프로세스를 생성하여 랜덤한 시간 동안 대기한 후, 먼저 종료된 3개의 프로세스 반환 값을 배열에 기록한다. (exit(i) 사용)
