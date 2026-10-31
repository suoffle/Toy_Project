🔎 Select와 Epoll의 클라이언트 수에 따른 실행 시간을 비교하는 실험을 진행했습니다.

Select와 Epoll은 여러 파일 디스크립터의 입출력 이벤트를 처리하는 I/O multiplexing 방법입니다.
클라이언트 수가 증가할 때 두 방식의 실행 시간에 어떤 차이가 나타나는지 직접 확인해보고자 하였습니다.

[프로젝트 구성]   

Server: 
1) 목표 Client 수만큼 클라이언트 연결 요청 수락
2) 목표 Client 수 도달 시 모든 클라이언트에게 "START" 메세지 전송
3) Client 메세지 수신 및 에코 응답

Client: 
1) Server에 연결 요청
2) "START" 메세지 수신 이후: 64byte의 메세지를 10번 송신 및 에코 메세지 수신

<br>

🖥 실험 구성  
- 비교 대상: Select 서버 / Epoll 서버  
- Client 수: [3, 4, 5, 10, 20, 64, 128, 256, 512, 1020]  
- 측정 범위: START 이후부터 서버 종료까지(메세지 송수신, 이벤트 감지 및 처리, 연결 종료 처리 등의 시간)  
- 측정 횟수: 각 조건별 5회  

```
실행 방법(예시)
gcc Server_epoll.c -o server
gcc Client.c -o client
./server 8888 > server.log 2>&1 & sleep 1; for i in $(seq 1 512); do ./client 127.0.0.1 8888 "client$i" & done; wait
```
--- 
### [프로젝트 결과]
#### 1. Client 수에 따른 Select/Epoll 실행 시간 비교
<img width="526" height="257" alt="Image" src="https://github.com/user-attachments/assets/3e5abb3e-b36d-46d8-8afc-832e8c0f9204" />

#### 2. 평균 실행 시간 차이(Select - Epoll)
<img width="526" height="262" alt="Image" src="https://github.com/user-attachments/assets/f252f012-ce16-417c-9fc7-c1f49e677351" />

#### 3. 평균 실행 시간 차이(표)
<img width="300" height="150" alt="Image" src="https://github.com/user-attachments/assets/1fef58cc-4c56-4a32-93dc-82638b2b82ad" />


1) 모든 클라이언트 수 조건에서 Epoll의 평균 실행 시간이 Select보다 짧게 측정되었습니다. 다만 클라이언트 수가 적을 때는 시간 차이가 작았고, 개별 측정에서는 결과가 달라지는 경우도 있었습니다.

2) 클라이언트 수가 늘어날수록 Epoll과 Select의 평균 실행 시간의 차이가 커지는 경향이 존재했습니다. 128개에서는 약 1.12초, 512개에서는 약 3.59초, 1020개에서는 약 8.72초 정도 차이가 나타났습니다.
    
3) 128개에서는 Epoll의 실행 시간이 Select보다 약 47% 짧았지만, 1020개에서는 약 6% 짧았습니다. 따라서 클라이언트 수가 많아질수록 Epoll의 성능이 비례해서 증가한다고 해석하기는 어려웠습니다.

<br>
💡처음에는 구조가 단순한 Select가 작은 Client 수의 환경에서는 Epoll보다 빠르지 않을까 생각해 프로젝트를 select 기반으로 진행했습니다. 하지만, 이번 실험 결과를 통해 작은 Client 수에서도 Epoll의 평균 실행 시간이 더 짧게 나타났고, 이를 계기로 Epoll 기반으로도 프로젝트를 구현해보았습니다.
<br><br>
이번 실험으로 Select와 Epoll의 성능을 직접 측정해 확인하고, 절대적인 시간 차이와 상대적인 시간 차이를 살펴보았습니다. 다만 이번 결과가 다른 환경과 통신 상황에서도 동일한지는 확인이 필요할 것 같습니다.   

<br><br>
🔬 추가 실험  
한 번의 실행 과정에서 main의 시작 지점, START 지점, 종료 지점을 각각 기록해 전체 실행 시간과 START 이후 구간의 시간을 비교할 예정입니다.   

1) 클라이언트 연결 수락과 소켓 fd 감시 대상 등록에 걸리는 시간을 각각 측정하여, select와 epoll의 연결 준비 과정에서 발생하는 시간 차이를 비교하고자 합니다.  
2) 각 방식의 전체 실행 시간을 '클라이언트 연결 대기 및 수락, fd 등록을 포함한 준비 구간'과 'START 이후 메세지 이벤트 처리 구간'으로 나누어, 각 구간이 전체 실행 시간에서 차지하는 비율을 확인하고자 합니다.  
<br><br>

#BBG #OS #Linux #Network #Socket #Select #Epoll
