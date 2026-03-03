#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define THREAD_COUNT 10
#define ALLOWED_THREADS 3

pthread_mutex_t mutex;
int select_num[3];
int entry;

//get_number 함수에서의 thread가 실행할 함수
void* thread_function(void* arg) {
    int thread_num = *((int*)arg);	//입력받은 인자를 숫자로 변환하여 thread_num에 저장
    
    usleep((rand()%1000*1000));		//각 스레드마다 랜덤한 값만큼 대기
    pthread_mutex_lock(&mutex);		//mutex lock
    if(entry<3) select_num[entry++]=thread_num;	//입력받은 숫자가 아직 3개가 되지 않았으면 스레드 번호를 입력
    pthread_mutex_unlock(&mutex);	//mutex unlock
    
    return NULL;
}


int main() {
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
     	
     	//thread로부터 나온 결과 출력
	//printf("selected number: %d%d%d\n",select_num[0],select_num[1],select_num[2]);
	
    	pthread_mutex_destroy(&mutex);  //destroy mutex
	
	return 0;
}
