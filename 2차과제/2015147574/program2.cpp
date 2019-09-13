#include <iostream>
#include <math.h>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

#define Shared_Memory_key 1234 //shared memory key


// n : 리만합을 구하기 위한 사각형의 개수 m : 다항함수의 최고차항의 차수 c[] : 각 항의 계수
int main(int argc, char* argv[] )
{
	int arg[100]; //argv 변수값을 int형태의 배열로 저장할 장소
	int n, m; // n은 사각형 개수, m은 다항함수의 최고차항의 차수 
	long double dx; // delta x
	long double xk; // xk값
	long double *riemann_sum; 	// 프로그램을 통해 계산된 리만 합
	long double function_sum=0;	// function sum
	long total_time_ms; 		// 리만 합을 계산하는데 소요된 시간 (밀리초 단위) ss
	struct timeval start, end; // check time argument
	int total_ps; // 총 생성할 프로세스 수
	int run_ps=0; // 생성한 프로세스 수
	int status; // wait function
	int shmid; // shared memory id
	long double total_sum=0; //total_sum
	long double temp_sum=0; //temp
	

	// arg[0]=total_process, arg[1]=n, arg[2]=m, 다른것모두다 c
	for (int i = 1; i < argc; i++) {
		arg[i-1] = atoi(argv[i]); // argv[]값을 char->long double로 변환
	}
	total_ps=arg[0];
	n=arg[1];
	m=arg[2];

	shmid=shmget((key_t)Shared_Memory_key, sizeof(long double),IPC_CREAT|0666); //충분한 크기만큼 공유메모리 할당
	riemann_sum=(long double *)shmat(shmid, NULL, 0); // 리만합변수를 공유메로리에 접속
	*riemann_sum=0; // 공유메모리 초기화
	
	pid_t pids[total_ps]; //process 변수

	int divide[total_ps]; //각 프로세스당 몇번째 사각형까지 했는지 저장하는 배열
	int d_r=n%total_ps; //나머지
	int d_q=n/total_ps; //몫
	divide[0]=0; //첫번째값 0으로 초기화
	if(d_r==0) { //나머지가 0이면 균등하게 분배
		for(int i=1; i<total_ps+1; i++)
		{
			divide[i]=d_q;	
		}
	}
	else { //나머지가 있으면 그 나머지만큼은 사각형을 몫+1만큼 설정하고 그 뒤에는 몫만큼 설정 
		for(int i=1; i<d_r+1; i++)
		{
			divide[i]=d_q+1;
		}
		for(int i=d_r+1; i<total_ps+1; i++)
		{
			divide[i]=d_q;
		}
	} 
	for(int i=1; i<total_ps+1; i++) { // 지금 내가 몇번째 사각형이 있는지 계산
		divide[i]=divide[i]+divide[i-1]; 
	}

	gettimeofday(&start, NULL); //check start_time
	dx=(long double)1000/arg[1]; //delta x
	sem_t *syn; //세마포어 변수 선언
	if((syn=sem_open("syn",O_CREAT,0777,1))==NULL) { //선언실패시 에러메시지 출력
		perror("Sem_open error");
		exit(1);
	}

	while(run_ps<total_ps) { //생성할 process개수만큼 반복
		pids[run_ps] = fork(); //create process
		if(pids[run_ps] < 0) { //fork fail
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
		else if(pids[run_ps] == 0) { //child
			
			for(int k=divide[run_ps]; k<divide[run_ps+1]; k++) { 
    				for(int j = 0; j < m+1; j++) {
    					xk=(long double)k*dx; // xk값 
    					function_sum+=(long double)arg[j+3]*pow(xk, m-j); //arg[j+2]는 cm을 의미
    				}
			temp_sum+=(long double)function_sum*dx; 
			function_sum=0;
   		 	}
			sem_wait(syn); //세마포어 시작
			*riemann_sum+=temp_sum; // 공유메모리에다가 값저장
			sem_post(syn); //세마포어 종료
			sem_close(syn);	//세마포어 없애기
			shmdt(riemann_sum); // 프로세스가 끝나면 공유메모리와 접속끊기
			_exit(EXIT_SUCCESS); //자식프로세스 종료		
		}
		else { //parent
		}
		run_ps++; //현재 생성한 프로세스개수 체크
	}
	
	for(int i=0; i<total_ps; i++) {
		wait(&status); // 부모프로세스가 자식프로세스 갯수만큼 다끝날때까지 기다리기
	}
	total_sum=*riemann_sum; //변수옮기기
	
    	gettimeofday(&end, NULL); //check end_time
    	total_time_ms=1000*((long)end.tv_sec-(long)start.tv_sec)+((long)end.tv_usec-(long)start.tv_usec)/1000; //리만합 실행 시간을 ms단위로 측정
	printf("Riemann Sum: %.4Lf\nTotal Time: %ld\n", total_sum, total_time_ms); //시간(ms)와 리만합계산값 출력

	if( -1 == shmctl(shmid,IPC_RMID, 0)) { //공유메모리해체
		return -1;
	}
	return 0;

}
