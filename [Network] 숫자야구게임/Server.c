#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <time.h>

#define BUF_SIZE 100	//메세지 최대 길이
#define MAX_CLNT 4	//client 최대 4명까지

void set_answer();		//답(숫자 3개) 설정
void check_answer(char*);	//client 정답 확인
void error_handling(char *msg);	//에러 처리 함수

int answer[3];		//정답(숫자 3개)
int client_num[3];	//client로부터 온 숫자 3개
char return_msg[2];	//client 답의 strike, ball 개수


int main(int argc, char *argv[]) {
    srand(time(NULL));	//rand() 사용을 위한 srand 수행
    
	int serv_sock, clnt_sock;			//server 소켓, client 소켓
	struct sockaddr_in serv_adr, clnt_adr;		//server 소켓 주소, client 소켓 주소
	socklen_t clnt_adr_sz;				//clnt_adr size
	int fd_max;					//가장 큰 clnt_sock 번호
	char buf[BUF_SIZE];				//client에서 온 메세지를 담은 버퍼
	fd_set reads, cpy_reads;			//select에서 사용할 파일 디스크립터 집합, 이전 집합 저장
	int clnt_socks[MAX_CLNT];			//client socket 저장 배열
	int clnt_count = 0;				//client 수

	//답(숫자 3개) 설정
 	set_answer();

	//입력 오류 시(입력: Port 번호)
	if (argc != 2) {
	printf("Usage: %s <port>\n", argv[0]);
	exit(1);
	}

	//소켓 생성
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	//오류 시
	if (serv_sock == -1)
	error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));		//serv_adr 초기화
	serv_adr.sin_family = AF_INET;			//IPv4
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);	//아무 연결 허용
	serv_adr.sin_port = htons(atoi(argv[1]));	//Port 번호 입력

	//serv_sock에 serv_adr 바인딩(client로부터 연결을 받을 소켓)
	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");		//에러 발생 시

	//listen(연결 대기)
	if (listen(serv_sock, 3) == -1)
		error_handling("listen() error");	//에러 발생 시

	FD_ZERO(&reads);		//reads 집합 초기화
	FD_SET(serv_sock, &reads);	//serv_sock reads에 추가
	fd_max = serv_sock;

	while (1) {
	cpy_reads = reads;	//select 전 이전 reads 정보 저장
	
	//select로 수신된 데이터가 있는지 확인
	if ((select(fd_max + 1, &cpy_reads, 0, 0, NULL)) == -1) break;

	for (int i = 0; i < fd_max + 1; i++) {
	    
	    //select로부터의 결과에 존재하는 디스크립터인 경우
	    if (FD_ISSET(i, &cpy_reads)) {
	    	
	    	//server에게 연결을 시도하는 client가 있는 경우
		if (i==serv_sock && clnt_count<5) {
		    clnt_adr_sz = sizeof(clnt_adr);
		    //client 연결 수락
		    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
		    
		    //연결 수락 과정에서 오류 발생 시
		    if (clnt_sock == -1)
			error_handling("accept() error");
		    
		    //reads 집합에 새로운 client 소켓 추가
		    FD_SET(clnt_sock, &reads);
		    if (fd_max < clnt_sock)
			fd_max = clnt_sock;

		    clnt_socks[clnt_count++] = clnt_sock;	//clnt_socks 배열에도 저장
		    
		    //어떤 client가 연결되었는지 이름 출력
		    memset(buf, 0, BUF_SIZE);
		    int readcount=read(clnt_sock, buf, BUF_SIZE);
		    printf("connected client: %s\n", buf); 
		} 
		else { //server 소켓이 아닐 시 
		    int str_len = read(i, buf, BUF_SIZE);	//client로부터의 메세지 buf에 저장 및 길이 저장
		    if (str_len == 0) {		//메세지가 없을 시 
			FD_CLR(i, &reads);	//reads 집합에서 제거
			close(i);		//연결 종료
			printf("closed client: %d\n", i);
			
			//clnt_socks 배열 정리
			for (int j = 0; j < clnt_count; j++) {
			    if (clnt_socks[j] == i) {
			        clnt_socks[j] = clnt_socks[--clnt_count];
		       		
		       	//모든 client 연결 종료 시 프로그램 종료
				if(clnt_count==0) exit(1);
			        
			        break;
			    }
			}
		       
		    } else {	//메세지가 있을 시
		    	char name[30]="";	//client 이름
		    	char number[6]="";	//client가 보낸 숫자 저장 배열
		    	char *pos_start=strchr(buf,'[');//client 이름 시작 부분 찾기
		    	char *pos_end=strchr(buf,']');	//client 이름 마지막 부분 찾기
		    	
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
		    	
		    	//모든 client에게 메세지 전달
			for (int j = 0; j < clnt_count; j++) {
			    	char buffer[BUF_SIZE]; 	//client에게 보낼 메세지 저장 버퍼
			    	
			    	if(return_msg[0]==3) //client가 보낸 답이 정답일 시
			    		sprintf(buffer,"%s got the Answer!!\n",name);
			    	else	//client가 보낸 답이 정답이 아닐 시
		       			sprintf(buffer,"%s num: %d%d%d strike: %d ball: %d\n",name,client_num[0],client_num[1],client_num[2],return_msg[0],return_msg[1]);
		       		
		       		//메세지 전송	
				write(clnt_socks[j], buffer, strlen(buffer));
			   
			}
		    }
		}
	    }
	}
	}
	//연결 종료
	close(serv_sock);
	return 0;
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


