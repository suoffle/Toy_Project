#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define BUF_SIZE 1024		//메세지 최대 길이
#define CLNT_MAX 128		//listen 대기열 최대 크기
#define EPOLL_SIZE 50		//epoll_wait 최대 이벤트 수
#define TARGET_CLIENTS 1020	//동시 접속 client 수

void accept_client(int epoll_fd, int serv_sock);
void handle_client(int epoll_fd, int client_sock, unsigned int events);
int send_all(int sock, const char *buf, size_t len);
void close_client(int epoll_fd, int clinet_sock);
void error_handling(char *msg);	//에러 처리 함수

int client_socks[TARGET_CLIENTS];
int clnt_cnt = 0;
int start = 0;

struct timespec start_time, end_time;

int main(int argc, char *argv[]) {

	////////////////////////////////////////////시간 측정(start2)
	//if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1) error_handling("clock_gettime");

	int serv_sock;						//server 소켓, client 소켓
	struct sockaddr_in serv_adr;		//server 소켓 주소, client 소켓 주소

	struct epoll_event *ep_events;		//발생 이벤트 저장 배열	
	struct epoll_event event={0};		//소켓 등록 설정
	int fd, event_cnt;					//epoll 인스턴스 fd
	
	//입력 오류 시(입력: Port 번호)
	if(argc != 2) {
	        printf("Usage: %s <port>\n", argv[0]);
	        exit(1);
	}
	
	//연결 끊긴 소켓 처리
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) error_handling("signal error");
		
	//socket()
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	//socket error
	if(serv_sock == -1) error_handling("socket() error");

    //binding()
    int option = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) error_handling("setsockopt()");
	
	memset(&serv_adr, 0, sizeof(serv_adr));			//serv_adr 초기화
	serv_adr.sin_family = AF_INET;					//IPv4
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);	//아무 연결 허용
	serv_adr.sin_port = htons(atoi(argv[1]));		//Port 번호 입력

	if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) error_handling("bind() error");

	//listen()
	if(listen(serv_sock, CLNT_MAX) == -1) error_handling("listen() error");	

	//epoll 인스턴스 생성
	if((fd = epoll_create1(0)) == -1) error_handling("epoll_create1() error");
	
	//발생 이벤트 결과 저장 배열 할당
	if((ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE)) == NULL) error_handling("ep_events malloc");
	event.events = EPOLLIN;		//연결 수락 감시
	event.data.fd = serv_sock;	//server fd 저장
	
	//서버 소켓 epoll 감시 대상에 추가
	if(epoll_ctl(fd, EPOLL_CTL_ADD, serv_sock, &event) == -1) error_handling("epoll server ADD");
	printf("wait for %d/%d clients\n", clnt_cnt, TARGET_CLIENTS);
	
	//start == 0 -> client 연결 기다림
	//start == 1 -> 실제 메세지 송수신 시작
	while(!start || clnt_cnt > 0){
	        //이벤트 대기
	        event_cnt = epoll_wait(fd, ep_events, EPOLL_SIZE, -1);
	        //이벤트 대기 에러
	        if(event_cnt == -1){
	        	if(errno == EINTR) continue;
	        	error_handling("epoll_wait() error");
	        }

			//이벤트 발생 소켓 리스트 확인
	        for(int i=0; i<event_cnt; i++){
		        int current_sock = ep_events[i].data.fd;
		        unsigned int events = ep_events[i].events;
		       		
				//서버 소켓 이벤트 처리
		       	if(current_sock == serv_sock){	
					//서버 대기 소켓 오류
		       		if(events & (EPOLLERR | EPOLLHUP)) error_handling("listen socket error");
		       		//새로운 client 연결 수락
					if(events & EPOLLIN) accept_client(fd, serv_sock);
		       	} else{ //클라이언트 소켓에 이벤트 발생(메세지 수신 - 에코, 연결 종료 처리)
		       		handle_client(fd, current_sock, events);
		       	}
			}
         }
         
         //////////////////////////////////////시간 측정(end)
         if(clock_gettime(CLOCK_MONOTONIC, &end_time) == -1) error_handling("after while gettime");
         
         double total_time = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_nsec - start_time.tv_nsec)/1e9;
         printf("total time: %.6f s\n", total_time);
         
         free(ep_events);
         close(serv_sock);
         close(fd);
         
		return 0;
}

void accept_client(int epoll_fd, int serv_sock)
{
	int client_sock;
	
	//새로운 client 소켓 
	do{ client_sock = accept(serv_sock, NULL, NULL);
	} while(client_sock < 0 && errno == EINTR);
	
	if(client_sock == -1){ perror("accept"); return; }
	
	//TARGET_CLINETS보다 많은 연결 X
	if(start || clnt_cnt >= TARGET_CLIENTS){
		close(client_sock);
		return;
	}
	
	//새 client 소켓 읽기 이벤트 감시 등록
	struct epoll_event event = {0};
	event.events = EPOLLIN;
	event.data.fd = client_sock;
	
	//새 client epoll 인스턴스에 등록
	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event) == -1) {
		perror("epoll client ADD");
		close(client_sock);
		return;
	}

	//소켓 배열에 저장
	client_socks[clnt_cnt++] = client_sock;
	printf("connect client: %d/%d\n", clnt_cnt, TARGET_CLIENTS);
	
	//모든 client 연결 시 "START" broadcast
	if(clnt_cnt == TARGET_CLIENTS){
		start = 1;
		
		////////////////////////////////////////////시간 측정(start1)
		if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1) error_handling("clock_gettime");
		
		//모든 client 순회하며 "START" 메세지 전송
		for(int i=0; i<clnt_cnt; i++){
			if(send_all(client_socks[i], "START", 5) == -1){
				perror("START");
				error_handling("START CLIENT");
			}
		}
		printf("START sent to %d\n", clnt_cnt);
		}
}

//client 메세지 수신, 에코, 연결 종료 처리
void handle_client(int epoll_fd, int client_sock, unsigned int events)
{
	if(events & EPOLLERR){
		close_client(epoll_fd, client_sock);
		return;
	}
	
	if(!(events & (EPOLLIN | EPOLLHUP))) return;
	
	char buf[BUF_SIZE];
	ssize_t received;
	
	do{ received = recv(client_sock, buf, sizeof(buf), 0);	//메세지 수신
	} while(received < 0 && errno == EINTR);
	
	if(received == 0){
		close_client(epoll_fd, client_sock);		//client 연결 정상 종료
		return;
	} else if(received < 0){
		perror("received < 0");
		close_client(epoll_fd, client_sock);		//수신 오류 종료
		return;
	}
	
	//START 전에 client 요청 발생 시 오류 처리
	if(!start){	
		fprintf(stderr, "Client fd = %d sent data before start\n", client_sock);
		close_client(epoll_fd, client_sock);
		return;
	}
	
	//수신 받은 메세지 에코
	if(send_all(client_sock, buf, received) == -1){
		perror("send_all");
		close_client(epoll_fd, client_sock);
	}
	
}

//수신 받은 메세지 그대로 에코
int send_all(int sock, const char *buf, size_t length)
{
	size_t sent = 0;
	
	//바이트 스트림 방식 고려
	while(sent<length){
		ssize_t len = send(sock, buf+sent, length-sent,0);
		if(len <= 0) { return -1; }	//전송 실패
		sent += len;
	}
	return 0;
}

//epoll 인스턴스 감시 제거, 소켓 닫기, 소켓 배열에서 제거
void close_client(int epoll_fd, int client_sock)
{
	//epoll 감시 대상에서 제거
	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sock, NULL) == -1) perror("epoll_ctl");
	close(client_sock);		//소켓 닫기
	
	//소켓 배열에서 제
	for(int i=0; i < clnt_cnt; i++){
		if(client_socks[i] == client_sock){
			client_socks[i] = client_socks[clnt_cnt -1];
			clnt_cnt--;
			break;
		}
	}
}

//에러 처리 함수
void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

