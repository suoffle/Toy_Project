#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100		//메세지 최대 길이
#define NAME_SIZE 30		//이름 최대 길이
#define THREAD_COUNT 10		//스레드 개수(0~9)

void* thread_function(void* arg);	//get_number 함수에서의 thread가 실행할 함수
void get_number();			//숫자를 선택하는 함수
void * send_msg(void * arg);		//server에게 메세지 송신(작성)
void * recv_msg(void * arg);		//servre로부터 메세지 수신(읽기)
void error_handling(char * msg);	//에러 처리 함수
	
pthread_mutex_t mutex;		//mutex 생성
int select_num[3];		//답으로 선택한 숫자 3개
int entry;			//숫자를 구한 개수(select_num에 들어간 숫자 개수)
char name[NAME_SIZE+2];		//client 이름 
int name_len;			//client '[이름]' 길이
	
	
int main(int argc, char *argv[])
{
	int sock;				//파일 디스크립터(소켓) 변수
	struct sockaddr_in serv_addr;		//소켓 주소 저장 변수
	pthread_t snd_thread, rcv_thread;	//메세지를 보내는 스레드, 메세지를 받는 스레드
	
	//입력 내용(IP 번호, Port 번호) 오류 시
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	char nickname[NAME_SIZE];	//이름 변수
	printf("Your Nickname?(only eng letter accepted): ");
	scanf("%s",nickname);			//이름 입력 받기
	getchar();				//delete enter
	sprintf(name, "[%s]", nickname);	//'이름'을 '[이름]'으로 저장
	
	//소켓 생성(TCP 통신)
	sock=socket(PF_INET, SOCK_STREAM, 0);	
	
	//serv_addr 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));				
	serv_addr.sin_family=AF_INET;						//IPv4
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);				//IP 번호 입력
	serv_addr.sin_port=htons(atoi(argv[2]));				//port 번호 입력
	  
	//server에 연결
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)	
		error_handling("connect() error");
			
	name_len=write(sock, name, strlen(name)); //server에게 이름을 보내주고 name_len에 길이 저장
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);	//메세지를 보내는 스레드
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);	//메세지를 받는 스레드
	pthread_join(snd_thread, NULL);					//스레드 종료될 때까지 대기
	pthread_join(rcv_thread, NULL);					//스레드 종료될 때까지 대기
	
	//소켓 닫기(연결 종료)
	close(sock);  							
	return 0;
}


//숫자를 선택하는 함수
void get_number()
{
	pthread_t threads[THREAD_COUNT];	//사용할 스레드 생성
    	srand(time(NULL));			//랜덤 값을 생성하기 위한 srand 실행

	    pthread_mutex_init(&mutex, NULL); 			//mutex 초기화
	    select_num[0]=select_num[1]=select_num[2]=0;	//선택한 숫자 초기화
	    entry=0;						//입력 받은 숫자 개수 초기화
	    
	    //0~9번의 숫자를 받기 위해 0~9번 인자를 갖고 thread_function을 수행하는 thread 생성 및 실행
	    for (int i = 0; i < THREAD_COUNT; i++) {
		int *num=(int *)malloc(sizeof(int));				//스레드의 인자로 보낼 값 변수
		*num=i;
		pthread_create(&threads[i], NULL, thread_function, num);	//스레드 생성 및 실행
    		}

    	//0~9번 스레드가 모두 종료될 때까지 기다리기 위한 pthread_join 함수
    	for (int i = 0; i < THREAD_COUNT; i++) {
        	pthread_join(threads[i], NULL);
    	}
     	
	//스레드로부터 입력받은 3개의 값 문자열 타입으로 msg에 저장
	char msg[BUF_SIZE];
	sprintf(msg,"%d%d%d\n",select_num[0],select_num[1],select_num[2]);
	
    	pthread_mutex_destroy(&mutex);  //destroy mutex
	
	return;
}

//get_number 함수에서의 thread가 실행할 함수
void* thread_function(void* arg) {
    int thread_num = *((int*)arg);	//입력받은 인자를 숫자로 변환하여 thread_num에 저장
    
    usleep((rand()%6000));		//각 스레드마다 랜덤한 값만큼 대기
    pthread_mutex_lock(&mutex);		//mutex lock
    if(entry<3) select_num[entry++]=thread_num;	//입력받은 숫자가 아직 3개가 되지 않았으면 스레드 번호를 입력
    pthread_mutex_unlock(&mutex);	//mutex unlock
    
    return NULL;
}

//server에게 메세지 쓰기
void * send_msg(void * arg)
{
	int sock=*((int*)arg);			//소켓 번호
	char name_msg[NAME_SIZE+BUF_SIZE+2];	//client 이름
	char input_msg[BUF_SIZE];		//보내는 메세지(숫자 3개)
	
	while(1) 
	{
		printf("Press 'q' to quit or press Enter key\n");	
		fgets(input_msg, BUF_SIZE, stdin);	
		//프로그램 종료(q/Q)		
		if(!strcmp(input_msg,"q\n")||!strcmp(input_msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		
		//숫자 3개 구하기
		get_number();
		
		//구한 숫자 포맷팅해서 server에게 전송(파일 디스크립터에 작성)
		sprintf(name_msg,"%s %d%d%d", name, select_num[0],select_num[1],select_num[2]);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}
	
//servre로부터 메세지 읽기
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);			//소켓 번호
	char name_msg[NAME_SIZE+BUF_SIZE];	//server로부터 온 메세지(client 이름+메세지)
	int str_len;				//server로부터 온 메세지 길이
	
	//읽을 정보가 있을 때까지 수행
	while(1)
	{
		//sock으로부터 온 메세지를 읽어 저장 및 길이 저장
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		
		//읽을 정보가 없으면 return
		if(str_len==-1) return (void*)-1;
		
		name_msg[str_len]=0;
		printf("%s\n", name_msg);	//server에서 온 메세지 출력
		
		//만일 정답을 맞추었을 시 게임 종료
		if(!strncmp(name_msg, name, name_len) && !strncmp(name_msg+(name_len+1),"got the Answer!!",16)){ 
			printf("\nEnding Game..\n\n");
			exit(1);
		}
	}
	return NULL;
}

//에러 처리 함수	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
