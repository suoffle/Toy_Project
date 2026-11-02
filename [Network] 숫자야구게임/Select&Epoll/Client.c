#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MSG_SIZE 64			//메세지 최대 길이
#define NAME_SIZE 30		//이름 최대 길이
#define CLIENT_NUM 512		//동시 실행 client 수

void wait_clnt(int);
void send_msg(int);		//server에게 메세지 송신(작성)
void recv_msg(int);		//servre로부터 메세지 수신(읽기)
void error_handling(char * msg);	//에러 처리 함수

char name[NAME_SIZE+1];		//client 이름 
char message[MSG_SIZE];		//message(64 byte)

int main(int argc, char *argv[])
{
	int sock;				//파일 디스크립터(소켓) 변수
	struct sockaddr_in serv_addr;		//소켓 주소 저장 변수

	//입력 내용(IP 번호, Port 번호, name) 오류 시
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	if(strlen(argv[3]) == 0 || strlen(argv[3]) > NAME_SIZE) error_handling("name length");	

	//이름 저장
	snprintf(name, sizeof(name), "%s", argv[3]);

	//메세지 구성하기
	char prefix[NAME_SIZE + 4];
	int prefix_len = snprintf(prefix, sizeof(prefix), "[%s] ", name);  

	memset(message, 'A', sizeof(message));
	memcpy(message, prefix, prefix_len);
	message[MSG_SIZE-1] = '\n';

	//소켓 생성(TCP 통신)
	sock=socket(AF_INET, SOCK_STREAM, 0);	
	if(sock == -1) error_handling("socket create level error");

	//serv_addr 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));				
	serv_addr.sin_family=AF_INET;						//IPv4
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);				//IP 번호 입력
	serv_addr.sin_port=htons(atoi(argv[2]));					//port 번호 입력

	//server에 연결
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)	
		error_handling("connect() error");

	//10개 client 연결까지 대기
	wait_clnt(sock);

	//메세지 전송(동일 메세지 송수신)
	for(int i=0;i<CLIENT_NUM;i++){
		send_msg(sock);
		recv_msg(sock);
	}

	printf("[%s] received completed\n", name);
	//소켓 닫기(연결 종료)
	close(sock);  							
	return 0;
}

//서버로부터 오는 "START" 신호 수신
void wait_clnt(int sock){
	char buf[5];
	size_t accepted = 0;	
	
	//START 수신(바이트 스트림 방식 고려)
	while(accepted < sizeof(buf)){
		ssize_t len = recv(sock, buf+accepted, sizeof(buf)-accepted, 0);
		if(len <= 0) error_handling("wait_clnt");

		accepted += len;
	}
	if(memcmp(buf, "START", sizeof(buf)) != 0) error_handling("START not received");
}

//server에게 메세지 전송
void send_msg(int sock)
{
	size_t sent = 0;

	//메세지 전송(바이트 스트림 방식 고려 -> 총 64바이트 고정 길이)
	while(sent < MSG_SIZE){
		ssize_t len = write(sock, message+sent, MSG_SIZE-sent);

		if(len <= 0) error_handling("send error");

		sent += len;
	}
}

//servre로부터 메세지 읽기
void recv_msg(int sock)  
{
	char received[MSG_SIZE];
	size_t accepted = 0;	 

	//서버로부터 오는 메세지 수신(바이트 스트림 방식 고려)
	while(accepted < MSG_SIZE){
		ssize_t len = recv(sock, received+accepted, MSG_SIZE-accepted, 0);

		if(len <= 0) error_handling("recv error");

		accepted += len;
	}
	fwrite(received, 1, accepted, stdout);
	if(memcmp(message, received, MSG_SIZE) != 0) error_handling("message mismatch");

	return;
}

//에러 처리 함수	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
