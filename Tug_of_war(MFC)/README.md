#줄다리기게임_MFC.ver

### __설명:__
Tug_of_war(C) 프로젝트에서 만들었던 줄다리기 게임 MFC 버전이다.   
이전의 줄다리기 게임(키보드 입력으로 줄을 당겨 승리하는 방식)에 과외하던 학생의 아이디어를 추가하였다:partying_face:

#### __<추가된 아이디어>__
1. 게임은 총 3라운드로 진행된다.  
2. 인원 수는 최대 8명까지 가능하다.  
3. 게임 시작 전 '메뉴바'에서 플레이어의 인원 수와 각 플레이어의 몸무게를 설정한다.  
   -> 플레이어의 몸무게에 비례해 줄다리기의 줄을 당기는데 더 많이 당길 수 있다.  
5. 패널티 적용(넘어짐 효과)  
   -> 각 플레이어는 키보드 입력 시 20% 확률로 패널티(넘어짐)를 받을 수 있다.  
   -> 패널티는 3번의 턴동안 팀의 줄을 당기는 힘이 -3 감소된다.  
     (각 플레이어에게 적용되어 팀이 누적으로 패널티를 받을 수 있으며, 만약 4명(전원)이 넘어진 상태면 누적되지 않는다.)  

---
#### __<인원 수 설정 화면>__
<img width="1090" height="448" alt="Image" src="https://github.com/user-attachments/assets/7685d1d2-add6-4aae-b40f-534202e42aec" />
-> 화면의 윗 부분(메뉴바)에서 인원 수와 몸무게 설정을 할 수 있다.(필수)  

---
#### __<몸무게 설정 화면>__
<img width="1087" height="443" alt="Image" src="https://github.com/user-attachments/assets/c960baaa-a207-444b-90fe-446b6a827b22" />  

---
#### __<몸무게별 힘 환산표>__
<img width="132" height="197" alt="Image" src="https://github.com/user-attachments/assets/e0892099-4b55-4b4a-8bd9-b3dddea22c7a" />  

---
#### __<게임 시작 화면>__
<img width="1088" height="446" alt="Image" src="https://github.com/user-attachments/assets/dec0d95d-5e8a-4f6e-8058-6402f40f24ab" />
-> 화면의 왼쪽과 오른쪽에 각 플레이어에게 할당된 키보드 키를 확인할 수 있다.  

---
#### __<게임 승리 화면>__
<img width="1088" height="443" alt="Image" src="https://github.com/user-attachments/assets/6f43009a-5170-456f-9a3a-2eecb7f4664a" />
-> 양 팀의 플레이어들이 키보드를 연타하여 줄을 좌우로 이동시킬 수 있으며, 양 팀의 깃발에 닿으면 상대팀의 승리.  
-> 각 라운드 승리자는 화면 중앙의 ROUND 아래 원 안에 팀의 색상(노랑 혹은 파랑)으로 표시된다.  
-> 3라운드 진행 후 최종 승리팀을 보여준다.  
