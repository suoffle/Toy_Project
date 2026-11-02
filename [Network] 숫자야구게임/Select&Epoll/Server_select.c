#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define BUF_SIZE 1024		//메세지 최대 길이
#define CLNT_MAX 128		//listen 수락 대기열 크기
#define TARGET_CLIENTS 512  //동시 실행 Client 수

void accept_client(int serv_sock, fd_set *reads, int *fd_max);
void handle_client(int client_sock, fd_set *reads, int *fd_max);
int send_all(int sock, const char *buf, size_t length);
void close_client(int client_sock, fd_set *reads, int *fd_max);
void error_handling(char *msg);	//에러 처리 함수

int client_socks[TARGET_CLIENTS];
int clnt_cnt = 0;	//현재 연결 수락된 client 수
int start = 0;		//모든 client 연결 여부

struct timespec start_time, end_time;

int main(int argc, char *argv[]) {

	int serv_sock, clnt_sock;			//server 소켓, client 소켓
	struct sockaddr_in serv_adr;		//server 소켓 주소
	fd_set reads, cpy_reads;			//select에서 사용할 파일 디스크립터 집합, 이전 집합 저장
	int fd_max;				//file discriptor count

	//입력 오류 시(입력: Port 번호)
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	//연결 끊긴 소켓 처리
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) error_handling("signal error");

	//소켓 생성
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1) error_handling("socket() error");

	//Enable port reuse
	int option = 1;
	if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))==-1)
		error_handling("setsockopt error");


	memset(&serv_adr, 0, sizeof(serv_adr));			//serv_adr 초기화
	serv_adr.sin_family = AF_INET;					//IPv4
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);	//아무 연결 허용
	serv_adr.sin_port = htons(atoi(argv[1]));		//Port 번호 입력

	//serv_sock에 serv_adr 바인딩
	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");		

	//listen
	if (listen(serv_sock, CLNT_MAX) == -1)
		error_handling("listen() error");	//에러 발생 시

	FD_ZERO(&reads);		//reads 집합 초기화
	FD_SET(serv_sock, &reads);	//serv_sock reads에 추가
	fd_max = serv_sock;

	printf("wait for %d/%d clients\n", clnt_cnt, TARGET_CLIENTS);

	//모든 client 연결 대기 후 실행(동시 실행)
	while (!start || clnt_cnt > 0) {
		cpy_reads = reads;	//select 전 이전 reads 정보 저장

		//select로 수신된 데이터가 있는지 확인
		if ((select(fd_max + 1, &cpy_reads, 0, 0, NULL)) == -1){
			if(errno == EINTR) continue;
			error_handling("select error");
		}

		for (int i = 0; i < fd_max + 1; i++) {

			//select로부터의 결과에 존재하는 디스크립터인 경우
			if (!FD_ISSET(i, &cpy_reads)) continue;

			//server에게 연결을 시도하는 client가 있는 경우
			if (i == serv_sock) {
				accept_client(serv_sock, &reads, &fd_max);
			} 
			else { //server 소켓이 아닐 시 
				handle_client(i, &reads, &fd_max);
			}

		}
	}

	//////////////////////////////////////시간 측정
	if(clock_gettime(CLOCK_MONOTONIC, &end_time) == -1) error_handling("after while gettime");

	double total_time = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_nsec - start_time.tv_nsec)/1e9;
	printf("total time: %.6f s\n", total_time);

	//연결 종료
	close(serv_sock);
	return 0;
}

//새로운 client 연결 수락 & 감시 집합 등록
void accept_client(int serv_sock, fd_set *reads, int *fd_max)
{
	int client_sock;

	//accept client
	do { client_sock = accept(serv_sock, NULL, NULL);
	} while(client_sock < 0 && errno == EINTR);

	if(client_sock == -1){
		perror("accept_client");
		return;
	}

	//Client 수 초과 시
	if(start || clnt_cnt >= TARGET_CLIENTS){
		close(client_sock);
		return;
	}

	//Client_sock 번호 초과 시
	if(client_sock >= FD_SETSIZE){
		fprintf(stderr, "client_sock >= fd_size");
		close(client_sock);
		return;
	}

	//감시 지합에 추가
	FD_SET(client_sock, reads);

	if(client_sock > *fd_max) 
		*fd_max = client_sock;
	
	//소켓 배열에 저장
	client_socks[clnt_cnt++] = client_sock;

	printf("connect client: %d/%d\n", clnt_cnt, TARGET_CLIENTS);
	
	//모든 client 연결 완료 시 "START" Client에 전송
	if(clnt_cnt == TARGET_CLIENTS){
		start = 1;

		////////////////////////////////////////////시간 측정
		if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1) error_handling("clock_gettime");

		//모든 client에게 broadcast
		for(int i=0; i<clnt_cnt; i++){
			if(send_all(client_socks[i], "START", 5) ==-1){
				fprintf(stderr, "sent data before START");
				error_handling("START CLIENT");
			}
		}
		printf("START sent to %d\n", clnt_cnt);
	}

}

//client로부터 수신 받은 메세지 다시 전송(에코)
void handle_client(int client_sock, fd_set *reads, int *fd_max)
{
	char buf[BUF_SIZE];
	ssize_t received;

	//메세지 수신(client -> server)
	do{ received = recv(client_sock, buf, sizeof(buf), 0);
	} while(received < 0 && errno == EINTR);

	//client 정상 종료
	if(received == 0){
		close_client(client_sock, reads, fd_max);
		return;
	}

	//수신 오류 -> 연결 종료
	if(received < 0){
		perror("recv < 0");
		close_client(client_sock, reads, fd_max);
		return;
	}
	
	//START 전에는 client에게 메세지 전송 불가
	if(!start){
		perror("sent data before START");
		close_client(client_sock, reads, fd_max);
		return;
	}
	
	//메세지를 송신한 해당 client에게 그대로 전송(에코)
	if(send_all(client_sock, buf, received) == -1){
		perror("send_all == -1");
		close_client(client_sock, reads, fd_max);
	}
}

//메세지 전송
int send_all(int sock, const char *buf, size_t length)
{
	size_t sent = 0;
	
	//메세지 전송(바이트 스트림 방식 고려)
	while(sent < length){
		ssize_t len = send(sock, buf+sent, length-sent, 0);

		if(len < 0){
			if(errno == EINTR) continue;
			return -1;
		} else if(len == 0){
			errno = EPIPE;
			return -1;
		}
		sent += len;
	}
	return 0;
}

//client 연결 해제(소켓 배열에서 제거, 소켓 닫기, 파일디스크립터 집합에서 제거)
void close_client(int client_sock, fd_set *reads, int *fd_max)
{
	FD_CLR(client_sock, reads);		//파일 디스크립터 집합에서 제거
	close(client_sock);				//소켓 닫기

	//소켓 배열에서 제거
	for(int i=0; i < clnt_cnt; i++){
		if(client_socks[i] == client_sock){
			client_socks[i] = client_socks[clnt_cnt-1];
			clnt_cnt--;
			break;
		}
	}

	if(client_sock == *fd_max){
		while(*fd_max >= 0 && !FD_ISSET(*fd_max, reads)) (*fd_max)--;
	}
}

//에러 처리 함수
void error_handling(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}


