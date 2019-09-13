#include <iostream>
#include <math.h>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <cstring>

using namespace std;

const string GetTime() { //현재시간 출력
	time_t current=time(0); // store current time
	struct tm t; //tm과 t 선언
	char tp[80]; //buf
	t=*localtime(&current); //현재 시간을 초단위로 저장
	strftime(tp, sizeof(tp), "%X", &t); //시:분:초 로표시됨
	return tp;
}

int main() {
	string str1; // 입력문자열 저장공간
	pid_t pid; // process
	char **argv; //total_thread or process, n, m
	//int argument[1000]; //total_thread or process, n, m
	int count; //word count
	int count2; //word count2
	string end="exit"; // exit
	int status; //자식프로세스 종료상태

	int str_length=str1.length(); //string length
	//stringstream split(str1); //공백을 기준으로 string 나누기
	string d[1000]; // 나눠진 단어 저장
	string d2[1000]; // 나눠진 단어 저장
	string temp; //문자열 임시 복사장소

	while(1) { 
		count=0; //count 초기화
		count2=0; //count2 초기화
		cout<<GetTime()<<"$ "; //현재 시:분:초 출력
		getline(cin, str1); //exit 문자열이 나올때까지 문자열 입력
        	if(str1.compare(end)==0) { // exit 입력시 종료
			return 0;
		}
        
		stringstream split(str1); //str1을 단어단위로 split
		while(split>>d2[count2]) { // 단어가 몇개 있는지 세기
			count2++;
		}
		argv=(char **)malloc(sizeof(char *)*(count2+1)); //argv 동적할당
	
        
		stringstream split2(str1); //str1을 단어단위로 split
		while(split2>>d[count]) { //문자열 나누기		
			temp=d[count]; //temp에 나눠진 단어 저장
			argv[count] = (char *)malloc(sizeof(char)*(temp.size()+1)); //argv[count] 동적할당
			strcpy(argv[count], temp.c_str()); //argv[count]에 temp 문자열 복사
			count++;
		}
        	argv[count2] = NULL; //argv[] 끝에 값에 NULL값 저장

		pid=fork(); //create process
		if(pid<0) { //fork fail
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
		else if(pid==0) { //child process
			execvp(argv[0], argv); //program1-3 실행
		}
		else { //parent process
			wait(&status); //wait child process
		}	
	}
	
	return 0;
}
