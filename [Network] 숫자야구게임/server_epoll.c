#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <time.h>

#define BUF_SIZE 100	//메세지 최대 길이
#define MAX_CLNT 3	//client 최대 4명까지
#define EPOLL_SIZE 50

void set_answer();		//답(숫자 3개) 설정
void check_answer(char*);	//client 정답 확인
void error_handling(char *msg);	//에러 처리 함수

void accept_client(int fd, int serv_sock, int clnt_socks[], int *clnt_count);
void disconnect_client(int fd, int clnt_sock, int clnt_socks[], int *clnt_count);
void broadcast_result(const char *name, const int clnt_socks[], int clnt_count);
void process_message(char *buf, int str_len, const int clnt_socks[], int clnt_count);
void client_message(int fd, int clnt_sock, int clnt_socks[], int *clnt_count);

int answer[3];		//정답(숫자 3개)
int client_num[3];	//client로부터 온 숫자 3개
char return_msg[2];	//client 답의 strike, ball 개수


int main(int argc, char *argv[]) {
    srand(time(NULL));	//rand() 사용을 위한 srand 수행

    int serv_sock;			//server 소켓
    int clnt_socks[MAX_CLNT], clnt_count=0;
    struct sockaddr_in serv_adr;		//server 소켓 주소

    struct epoll_event *ep_events;
    struct epoll_event event;
    int fd, event_cnt;

    //정답(숫자 3개) 설정
    set_answer();

    //입력 오류 시(입력: Port 번호)
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    //socket()
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    //socket error
    if(serv_sock == -1) error_handling("socket() error");

    //binding()
    int option = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));	//bind error 방지
    memset(&serv_adr, 0, sizeof(serv_adr));		//serv_adr 초기화
    serv_adr.sin_family = AF_INET;			//IPv4
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);	//아무 연결 허용
    serv_adr.sin_port = htons(atoi(argv[1]));	//Port 번호 입력

    if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) {perror("bind");exit(1);}

    //listen()
    if(listen(serv_sock, MAX_CLNT) == -1)
    error_handling("listen() error");	//에러 발생 시

    //epoll 인스턴스 생성
    if((fd = epoll_create1(0)) == -1) error_handling("epoll_create1() error");
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    if(epoll_ctl(fd, EPOLL_CTL_ADD, serv_sock, &event) == -1) error_handling("epoll_serv_ctl() error");

    while (1) {
        event_cnt = epoll_wait(fd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            puts("event_wait() error");
            break;
        }

        for (int i = 0; i < event_cnt; i++) {
            int event_sock = ep_events[i].data.fd;
            if (event_sock == serv_sock) {	//연결 요청 발생 시
                accept_client(fd, serv_sock, clnt_socks, &clnt_count);
            } else {	//메세지 송수신 처리
                client_message(fd, event_sock, clnt_socks, &clnt_count);
            }
        }
    }
    //close()
    free(ep_events);
    close(fd);
    close(serv_sock);
    return 0;
}    
    
//새 클라이언트 연결 수락 및 epoll 등록
void accept_client(int fd, int serv_sock, int clnt_socks[], int *clnt_count)
{
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;
    int clnt_sock;
    struct epoll_event event;
    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    if(clnt_sock == -1) error_handling("accept() error");

    event.events = EPOLLIN;
    event.data.fd = clnt_sock;

    if(epoll_ctl(fd, EPOLL_CTL_ADD, clnt_sock, &event) == -1)
    error_handling("epoll_clnt_ctl() error");
    printf("connect client: %d\n",clnt_sock);
    clnt_socks[(*clnt_count)++] = clnt_sock;
}

//클라이언트 연결 종료 및 목록에서 제거
void disconnect_client(int fd, int clnt_sock, int clnt_socks[], int *clnt_count)
{
    epoll_ctl(fd, EPOLL_CTL_DEL, clnt_sock, NULL);
    close(clnt_sock);

     //연결 종료: 종료 fd에 마지막 fd 대입
    for(int j = 0; j < *clnt_count; j++){
        if(clnt_socks[j] == clnt_sock){
            clnt_socks[j] = clnt_socks[--(*clnt_count)];
            break;
        }
    }
    //printf("close client: %d\n", clnt_sock);
}

//결과를 모든 클라이언트에게 전송
void broadcast_result(const char *name, const int clnt_socks[], int clnt_count)
{
    //모든 client에게 메세지 전달
    for(int j = 0; j < clnt_count; j++){
        char buffer[BUF_SIZE]; 	//client에게 보낼 메세지 저장 버퍼

        if(return_msg[0]==3){ //client가 보낸 답이 정답일 시
            sprintf(buffer,"%s got the Answer!!\n",name);
        }else{	//client가 보낸 답이 정답이 아닐 시
            sprintf(buffer,"%s num: %d%d%d strike: %d ball: %d\n",name,client_num[0],client_num[1],client_num[2],return_msg[0],return_msg[1]);
        }
        //메세지 전송
        send(clnt_socks[j], buffer, strlen(buffer), 0);
    }
}

//클라이언트가 보낸 이름과 숫자 읽고 정답 판정
void process_message(char *buf, int str_len, const int clnt_socks[], int clnt_count)
{
    char name[30]="";	//client 이름
    char number[6]="";	//client가 보낸 숫자 저장 배열
    char *pos_start=strchr(buf,'[');//client 이름 시작 부분 찾기
    char *pos_end=strchr(buf,']');	//client 이름 마지막 부분 찾기
    if(pos_start==NULL || pos_end==NULL) return;
    if(str_len<5){ puts("clnt_msg_len < 5");return;}

    strncpy(name, pos_start, pos_end-pos_start+1);	//찾은 위치로부터 이름 저장
    name[pos_end - pos_start + 1]='\0';		//마지막에 널 추가

    //atoi를 사용하기 위해 숫자 하나씩 널 문자를 포함해 저장
    number[0]=*(pos_end+2);
    number[1]='\0';
    number[2]=*(pos_end+3);
    number[3]='\0';
    number[4]=*(pos_end+4);
    number[5]='\0';

    //client의 답이 맞는지 확인
    check_answer(number);

    broadcast_result(name, clnt_socks, clnt_count);
}   
    

//클라이언트 수신 이벤트 처리
void client_message(int fd, int clnt_sock, int clnt_socks[], int *clnt_count)
{
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    int str_len = recv(clnt_sock, buf, BUF_SIZE, 0);

    if (str_len == 0) {
        disconnect_client(fd, clnt_sock, clnt_socks, clnt_count);
    } else if (str_len > 0) {
        process_message(buf, str_len, clnt_socks, *clnt_count);
    } else {
        error_handling("read() error");
    }
}

//정답 설정
void set_answer()
{
    int available_num[10]={};	//중복 값 제거를 위한 0~9번 사용 여부 저장 배열

    //1번째 숫자
    answer[0]=rand()%10; available_num[answer[0]]=1;

    //2번째 숫자
    do{ answer[1]=rand()%10; } while(available_num[answer[1]]!=0); //중복 제거
    available_num[answer[1]]=1;

    //3번째 숫자
    do{ answer[2]=rand()%10; } while(available_num[answer[2]]!=0);	//중복 제거
    available_num[answer[2]]=1;

    //선택된 답 출력
    printf("selected number:%d%d%d\n\n",answer[0],answer[1],answer[2]);
}

//client 정답 확인
void check_answer(char* msg)
{
    client_num[0]=atoi(&msg[0]);	//client가 선택한 1번째 숫자
    client_num[1]=atoi(&msg[2]);	//client가 선택한 2번째 숫자
    client_num[2]=atoi(&msg[4]);	//client가 선택한 3번째 숫자

    //client가 보낸 숫자 3개 출력(확인용)
    //printf("client number: %d%d%d\n",client_num[0],client_num[1],client_num[2]);

    //ball, strike 변수 초기화
    int ball=0,strike=0;

    //strike, ball 개수 확인
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            if(answer[i]==client_num[j] && i==j)strike++;		//숫자와 위치가 같은 시 strike
            else if(answer[i]==client_num[j] && i!=j)ball++;	//숫자는 같지만 위치가 다르면 ball
        }
    }

    return_msg[0]=strike; 	//return_msg[0]에 strike 개수 저장
    return_msg[1]=ball;	//return_msg[1]에 ball 개수 저장

    return;
}

//에러 처리 함수
void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

    
