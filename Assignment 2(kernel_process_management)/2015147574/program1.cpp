#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <iomanip>
using namespace std;

// n : 리만합을 구하기 위한 사각형의 개수 m : 다항함수의 최고차항의 차수 c[] : 각 항의 계수
int main(int argc, char* argv[] )
{
	int arg[100]; //argv로 받은 변수를 int형태로 저장할 배열
	int n, m; 
	long double dx; // delta x
	long double xk; // xk값
	long double riemann_sum=0; 	// 프로그램을 통해 계산된 리만 합
	long double function_sum=0;	// function sum
	long total_time_ms; 		// 리만 합을 계산하는데 소요된 시간 (밀리초 단위)
	struct timeval start, end; //gettimeofday로 리만프로그램 수행시간 측정
	
	for (int i = 1; i < argc; i++)
	{
		arg[i-1] = atoi(argv[i]); // argv[]값을 char->int로 변환
	}
	//arg[0]=n, arg[1]=m, 다른것모두다 c
	n=arg[0];
	m=arg[1];

	gettimeofday(&start, NULL); //check start_time
    	dx=(long double)1000/n;	//delta x=1000/n
    	for(int k=0; k<n; k++) 
    	{
    		for(int j = 0; j < m+1; j++) // m반복
    		{
    			xk=(long double)k*dx; // xk값 
    			function_sum+=(long double)arg[j+2]*pow(xk, m-j); //arg[j+2]는 cm을 의미
    		}
		riemann_sum+=(long double)function_sum*dx; // 리만합에 사각형 하나값 저장
		function_sum=0; //function_sum 초기화
    	}
    	gettimeofday(&end, NULL); //check end_time
    	total_time_ms=1000*((long)end.tv_sec-(long)start.tv_sec)+((long)end.tv_usec-(long)start.tv_usec)/1000; // 시간을 ms단위로 total_time_ms에 저장
	printf("Riemann Sum: %.4Lf\nTotal Time: %ld\n", riemann_sum, total_time_ms); //시간(ms)와 리만합계산값 출력
	return 0;
}
