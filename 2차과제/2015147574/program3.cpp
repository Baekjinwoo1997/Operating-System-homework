#include <iostream>
#include <math.h>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <thread>
#include <mutex>

using namespace std;

// arg[0]=total_process, arg[1]=n, arg[2]=m, 다른것모두다 c
long double riemann_sum=0; // Global 리만합
int arg[100]; //리만합계산에 필요한 변수 배열(n, m, c값들)
mutex syn; //동기화를 위한 mutex선언


void *riemann(void *data)
{
	//변수 재설정
   	int run_t=(int)(size_t)data;
	int total_thread=arg[0];
      	int n=arg[1];
      	int m=arg[2];
   	int divide[total_thread]; //각 프로세스당 몇번째 사각형까지 했는지 저장하는 배열  
   	int d_r=n%total_thread; //나머지
   	int d_q=n/total_thread; //몫
   	long double function_sum=0;
   	long double dx=(long double)1000/n; //delta x
   	long double xk;
	long double temp; //temp value
   	divide[0]=0; //첫번째값 0으로 초기화
  
   	// divide[] 값이 thread당 배정된 사격형의 개수
   	if(d_r==0) { //나머지가 0이면 균등하게 분배
      		for(int i=1; i<total_thread+1; i++)
            	{
              		divide[i]=d_q;   
            	}
      	}
      	else { //나머지가 있으면 그 나머지만큼은 사각형을 몫+1만큼 설정하고 그 뒤에는 몫만큼 설정 
        	for(int i=1; i<d_r+1; i++)
            	{
               		divide[i]=d_q+1;
            	}
            	for(int i=d_r+1; i<total_thread+1; i++)
            	{
               		divide[i]=d_q;
            	}
      	} 

      	for(int i=1; i<total_thread+1; i++) { //지금 내가 몇번째 사각형이 있는지 계산
           	divide[i]=divide[i]+divide[i-1];
      	}
  

     	for(int k=divide[run_t]; k<divide[run_t+1]; k++) { 
        	for(int j = 0; j < m+1; j++) {
           		xk=(long double)k*dx; // xk값 
           		function_sum+=(long double)arg[j+3]*pow(xk, m-j); //arg[j+2]는 cm을 의미
        	}
      	}
	temp=function_sum*dx; //리만부분합을 temp값에 저장
	syn.lock(); //mutex lock
      	riemann_sum+=temp; // 글로벌 데이터에 저장
	syn.unlock(); //mutex unlock
   	pthread_exit(NULL); //thread exit
}


int main(int argc, char* argv[] )
{
      	long total_time_ms;       // 리만 합을 계산하는데 소요된 시간 (밀리초 단위) ss
      	struct timeval start, end; // check time argument
      	unsigned long run_thread=0; // 생성한 thread 수 
      	int thr_id; //crate thread시 error check
      	int thread_result; // 스레드 종료시 값저장 변수
      	// arg[0]=total_process, arg[1]=n, arg[2]=m, 다른것모두다 c
      	for (int i = 1; i < argc; i++) {
            	arg[i-1] = atoi(argv[i]); // argv[]값을 char->int로 변환
      	}
      	int total_thread=arg[0];
      	int n=arg[1];
      	int m=arg[2];
      	pthread_t *thread_handle=new pthread_t[total_thread]; // 충분한 크기만큼 선언
      	gettimeofday(&start, NULL); //check start_time
   
      	while(run_thread<total_thread) {
            	thr_id=pthread_create(&thread_handle[run_thread], NULL, riemann, (void *)run_thread); //create thread
            	if(thr_id != 0) { //thread_handle[run_thread]값이 0보다 작으면 생성실패
               		perror("thread create failed");
               		exit(EXIT_FAILURE);
            	}
            	run_thread++; // run_thread값을 스레드 생성시마다 증가
      	}
	
      	for(int i=0; i<total_thread; i++) { // 생성된 pthread가 종료되기를 기다림
            	thr_id=pthread_join(thread_handle[i], (void **)&thread_result);
      	}
	gettimeofday(&end, NULL); //check end_time

  	total_time_ms=1000*((long)end.tv_sec-(long)start.tv_sec)+((long)end.tv_usec-(long)start.tv_usec)/1000; //리만합 생성하는데 걸린시간
      	printf("Riemann Sum: %.4Lf\nTotal Time: %ld\n", riemann_sum, total_time_ms); //print total_riemann, and execution time
      	return 0;
}
