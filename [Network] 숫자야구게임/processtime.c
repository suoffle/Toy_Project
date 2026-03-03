#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int select_num[3];
int entry;

int main() {
    	
	pid_t pid;		//process 번호
	for(int i=0; i<10; i++) {
		pid = fork();	//자식 process 생성

		//process 생성 오류 시
		if (pid == -1) {
		    perror("fork failed");
		    exit(1);
		} else if (pid == 0) {		//자식 process일 경우
		    usleep(rand()%1000*1000);	//각 process에 랜덤 값만큼 기다리게 하기
		    exit(i);	//n번째로 실행된 process 번호 리턴
		} 
	}
	
	//자식 process 기라디기
	for(int i=0; i<10; i++){
		int status;	
		
		//waitpid로 자식 process 종료 상태 저장
		if(waitpid(-1,&status,0)==-1){
			perror("waitpid failed");	//오류 발생 시 처리
			exit(1);
		}
		
		//종료된 상태가 존재하고 1번째~3번째로 먼저 종료된 경우
		if(WIFEXITED(status) && entry<3){
			select_num[entry++]=WEXITSTATUS(status);	//select_num 배열에 숫자(process 번호/리턴 값) 저장
		}
	}
    	   	
    	
     	//결과 출력
	//printf("selected number: %d%d%d\n",select_num[0],select_num[1],select_num[2]);
	
	
	return 0;
}
