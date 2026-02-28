#include <stdio.h>
#include <windows.h>
#include <conio.h>

//줄 초기화 함수(줄의 중심: '+'/ 그 외: '-')
void input(char rope[],int num)
{
    for(int i=0;i<num;i++){
        rope[i]='-';
    }
    rope[num/2]='+';
}

//왼쪽으로 줄 이동
void move1(char rope[],int num){
    char imsi=rope[0];
    for(int i=0;i<num;i++){
        rope[i]=rope[i+1];
    }
    rope[num-1]=imsi;
}

//오른쪽으로 줄 이동
void move2(char rope[],int num){
    char imsi=rope[num-1];
    for(int i=num-2;i>=0;i--){
        rope[i+1]=rope[i];
    }
    rope[0]=imsi;
}

//줄 현재 상태 출력 함수
void output(char rope[],int num)
{
    printf("User1");
    for(int i=0;i<num;i++)
        printf("%c",rope[i]);

    printf("User2");
    printf("\n");
}

int main()
{
    char rope[81]={};
    input(rope,81);
    int center=40;    //줄 중심의 위치

    printf("==============줄다리기 게임 규칙================\n");
    printf("|                                              |\n");
    printf("|  1. User1: a키를 이용해 왼쪽으로 줄 이동     |\n");
    printf("|  2. User2: ;키를 이용해 오른쪽으로 줄 이동   |\n");
    printf("|                                              |\n");
    printf("|   *q: User1 항복 / p: User2 항복             |\n");
    printf("================================================\n");
    printf("|      ------ 게임종료 버튼: x   --------      |\n");
    printf("================================================\n");
    printf("\n<<게임을 시작합니다>>\n\n");
    printf("a 또는 ; 버튼을 연타하세요.\n");


    while(1){

    char button;
    button=getch();   //getch(): 입력 받는 함수

    system("cls");    //system("cls"): 화면을 지워주는 함수

    //게임 종료(x)
    if(button=='x'){
        printf("버튼 x 입력: 게임을 종료합니다.");

        break;
    }

    //user1 항복(q)
    else if(button=='q'){
        printf("[User1: Surrendered]\n[User2 Win!!]\n");
        system("pause");
        printf("\n");
        output(rope,81);
        printf("[Center: 40+(%d)]\n",center-40);
        break;
    }

    //user2 항복(p)
    else if(button=='p'){
        printf("[User2: surrendered]\n[User1 Win!!]\n");
        system("pause");
        printf("\n");
        output(rope,81);
        printf("[Center: 40+(%d)]\n",center-40);
        break;
    }

    //user1 줄 당기기(a)
    else if(button=='a'){
            move1(rope,81);
            center--;
    }

    //user2 줄 당기기(;)
    else if(button==';'){
            move2(rope,81);
            center++;
    }

    //user1 승리
    if(center==10){
        printf("[[User1 win!!]]\n\n");
        output(rope,81);
        printf("\n");

        system("pause");
        break;
    }

    //user2 승리
    if(center==70){
        printf("[[User2 win!!]]\n\n");
        output(rope,81);
        printf("\n");

        system("pause");
        break;
    }

    //줄다리기 상태 출력
    output(rope,81);

    }
}

