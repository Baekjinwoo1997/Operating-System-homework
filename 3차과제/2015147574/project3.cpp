#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#define MAX 100000 //numver of instructions
#define pagetable_size 64
#define physical_size 32
using namespace std;

int main()
{
	int count=1; //text count
	int count2=0; //instruction count
	int alloc_count=0; //alloc_count;
	int access_count[physical_size]; //for LFU and MFU
	int choose; //paging placement 
	int P; //number of processes
	int N; //number of instructions
	int instruction[MAX][4]; //information of instructions
	int page_fault=0; //number of pagefault
	int pid, alloc_id, demand_page; //variables of instruction
	int stack_top; //physical stack top
	int bound; // access data by buddy system
	int check_empty=0; //check physical memory to empty
	bool collision=true; //variables for code to be executed only once
	int pop; //alloc_id of popped
	int min, min_index, min_alloc, max, max_index, max_alloc; //variables for LFU and MFU
	int optimal=0, optimal_change, optimal_index, optimal_alloc; //variables for Optimal
	int buddy=0; //check buddy system
	int buddy_bound=1; //check buddy bound
	int Xor; //check buddy
	int temp_and; //temp
	int And; //and
	int buddy_min_size, buddy_min_index; //for buddy system
	int ttemp;
	int access_demand;
	string function; //variable of instruction
	string input; //input string
	string temp; //temp string for split
	
	
	//------------------------------------------------------------------------------------------------------
	//file read

	ifstream data("input.txt"); //read input.txt
	while(getline(data, input)) { //allocate variable from input.txt
		switch(count) {
			case 1 : //paging placement
				choose=stoi(input);
				break;
			case 2 : //number of processes
				P=stoi(input);
				break;
			case 3 : //number of instructions
				N=stoi(input);
				break;
			default : //information of instructions
				stringstream split(input);
				int temp_count=0; //functions of one instruction
				while(split>>temp) {
					instruction[count2][temp_count]=stoi(temp);
					temp_count++;
				}
				count2++; //increase instruction count
				break;
		}
		count++; //increase text count
	}
	count2--;
	data.close();	
	
	//------------------------------------------------------------------------------------------------------
	// initialize page_table, physical memory

	int page_t[P][pagetable_size][2]; //declare page_table, page_t[pid][i][0]=>"i" index alloc_id, page_t[pid][i][1]=>"i" index Valid bit
	int page_p[P]; //pointer for page table
	int physical_stack[physical_size+1][5]; 
	//physical stack for placement, physical_stack[0][0] => stack_top, physical_stack[i][0](i>0) => i's alloc id, physical_stack[i][1] => i's count, 
	//physical_stack[i][2] => i's reference bit, physical_stack[i][3] => i's optimal count, physical_stack[i][4] => i's referencebyte
	physical_stack[0][0]=1; //initialize stack_top
	int physical_start[physical_size];
	int physical_index_size[physical_size];		
	
	for(int i=0; i<P; i++) {//initialize page_table & page table pointer, -1 means empty
		for(int j=0; j<pagetable_size; j++) {
			page_t[i][j][0]=-1;
			page_t[i][j][1]=-1;
		}
		page_p[i]=0;
	}
	int physical_m[physical_size]; //physical memory
	for(int j=0; j<physical_size; j++) { //initialize physical memory & physical memory pointer, -1 means empty
		physical_m[j]=-1;
		physical_start[j]=j;
		physical_index_size[j]=1;
		physical_stack[j+1][0]=-1; //alloc_id
		physical_stack[j+1][1]=0;  //count for LFU and MFU
		physical_stack[j+1][2]=0;  //reference bit for Sampled LRU
		physical_stack[j+1][3]=0;  //optimal count for Optimal
		physical_stack[j+1][4]=0; //reference byte
		access_count[j]=0;
	}
	
	//------------------------------------------------------------------------------------------------------
	//Instruction if demand_page>
	for(int i=0; i<count2+1; i++) {
		pid=instruction[i][0];
		alloc_id=instruction[i][2];
		demand_page=instruction[i][3];

		if(instruction[i][1]==1) { //allocate process's page table
			function="ALLOCATION";
			for(int j=page_p[pid]; j<page_p[pid]+demand_page; j++) {
				page_t[pid][j][0]=alloc_id; //AID
				page_t[pid][j][1]=0; //valid bit is 0
			}
			page_p[pid]+=demand_page;
		}
		//------------------------------------------------------------------------------------------------------
		else { //access physical memory
			function="ACCESS";
			bound=1; // access data by buddy system
			check_empty=0;
			bool access_check=true; //Check if same alloc_id exists
			//+1
			for(int x=1; x<physical_size+1; x++) { 
				if(physical_stack[x][0]==alloc_id) 
					access_check=false; //Nothing with the same alloc_id
			}
			if(access_check) {
				for(int j=0; j<page_p[pid]; j++) //check page_table[pid]
				{
					if(page_t[pid][j][0]==alloc_id) {
						alloc_count++; //count alloc_id
						page_t[pid][j][1]=1; //change the valid bit to 1 
					}
				}
				if(alloc_count==0) //Ignore access alloc_id in the absence of it
					break;
				while(bound<alloc_count) { //if 2^n-1<=number of alloc blocks <=2^n -> number of alloc blocks become 2^n-1 
					bound=bound*2;
				}
				//------------------------------------------------------------------------------------------------------
				//Buddy Combine
				//physical_start[], physical_index_size[], physical_m[]
				//버디시스템에서 버디끼리는 한비트 차이난다. 따라서 xor한값과 xor한값에서 1뺀값을 and연산하면 0이 나와야 한다.
				for(int j=0; j<physical_size; j++) {
					physical_start[j]=j;
					physical_index_size[j]=1;
				}
				for(int j=0; j<5; j++) { //vary in physical size
					buddy=0;
					buddy_bound=1;
					for(int k=0; k<j; k++) {
						buddy_bound=buddy_bound*2;
					}
					while(buddy<physical_size-1) {
						Xor=physical_start[buddy]^physical_start[buddy+buddy_bound];
						temp_and=Xor-1;
						if(physical_start[buddy]==physical_start[buddy+buddy_bound])
							And=-1;
						else
							And=temp_and&Xor;
						// 둘 다 비어있고 주소차이가 1이고 사이즈가 같으면 합친다.
						if(And==0 && physical_index_size[buddy]==physical_index_size[buddy+buddy_bound] && physical_m[buddy]==-1 && physical_m[buddy+buddy_bound]==-1 && physical_index_size[buddy]==buddy_bound && physical_index_size[buddy+buddy_bound]==buddy_bound) {
							ttemp=physical_index_size[buddy];
							for(int w=0; w<ttemp; w++)
							{
								physical_index_size[buddy+w]*=2;
								physical_index_size[buddy+buddy_bound+w]*=2;
								physical_start[buddy+buddy_bound+w]=physical_start[buddy];
							}
						}
						buddy+=buddy_bound;						
					}
				}
				buddy_min_size=physical_size*2;
				buddy_min_index=physical_size;
				for(int j=0; j<physical_size; j++) {
					 if(bound<=physical_index_size[j] && physical_m[j]==-1) {
						if(buddy_min_size>physical_index_size[j]) {
							buddy_min_size=physical_index_size[j];
							buddy_min_index=physical_start[j];
						}
					 }
				}
				check_empty=buddy_min_index;
				//------------------------------------------------------------------------------------------------------

				if(check_empty>=physical_size) { //require page replacement
					while(check_empty>=physical_size) {
						switch(choose) {
							case 0: { //FIFO
								for(int x=2; x<physical_stack[0][0]+1; x++) {
									pop=physical_stack[x-1][0]; //pop alloc_id
									physical_stack[x-1][0]=physical_stack[x][0]; //pull the stack down one space at a time
									if(collision) { //this code will only be executed once.
										for(int y=0; y<physical_size; y++) //check page_table[pid]
										{
											if(physical_m[y]==pop) {
												physical_m[y]=-1; //swap out physical memory
											}
										}
										for(int z=0; z<P; z++) { // change the valid bit to 0
											for(int y=0; y<pagetable_size; y++) {
												if(page_t[z][y][0]==pop)										
													page_t[z][y][1]=0; 
											}
										}
									}
									collision=false;
								}
								collision=true;
								physical_stack[0][0]--; //stack_top-1
								break;
							}
							//------------------------------------------------------------------------------------------------------
							case 1: { //LRU
								for(int x=2; x<physical_stack[0][0]+1; x++) {
									pop=physical_stack[x-1][0]; //pop alloc_id
									physical_stack[x-1][0]=physical_stack[x][0]; //pull the stack down one space at a time
									if(collision) { //this code will only be executed once.
										for(int y=0; y<physical_size; y++) //check page_table[pid]
										{
											if(physical_m[y]==pop) {
												physical_m[y]=-1; //swap out physical memory
											}
										}
										for(int z=0; z<P; z++) { // change the valid bit to 0
											for(int y=0; y<pagetable_size; y++) {
												if(page_t[z][y][0]==pop)										
													page_t[z][y][1]=0; 
											}
										}
									}
									collision=false;
								}
								collision=true;
								physical_stack[0][0]--; //stack_top-1
								break;
							}
							//------------------------------------------------------------------------------------------------------
							case 2: { //Sampled LRU
								min=physical_stack[1][4]; //time interval
								min_index=1;
								min_alloc=physical_stack[1][0];
								for(int x=2; x<physical_stack[0][0]+1; x++) {
									//find minimum referecne byte for Sampled LRU
									if(min>=physical_stack[x][4] && physical_stack[x][0]!=-1) {
										if(min==physical_stack[x][4]) { //If alloc_id is the same, replace the page with a small alloc_id.  
											if(min_alloc>physical_stack[x][0])
											{
												min=physical_stack[x][4];
												min_alloc=physical_stack[x][0];
												min_index=x;	
											}
										}
										else {
											min=physical_stack[x][4];
											min_alloc=physical_stack[x][0];
											min_index=x;
										}
									}
								}
								pop=physical_stack[min_index][0];
								for(int x=min_index+1; x<physical_stack[0][0]+1; x++) {
									//pull the stack down one space at a time
									//Reference bit initialization of out
									physical_stack[x-1][0]=physical_stack[x][0]; 
									physical_stack[x-1][2]=physical_stack[x][2];
									physical_stack[x-1][4]=physical_stack[x][4]; 
								}
								for(int y=0; y<physical_size; y++) {//check page_table[pid]
									if(physical_m[y]==pop) {
										physical_m[y]=-1; //swap out physical memory
									}
								}
								for(int z=0; z<P; z++) { // change the valid bit to 0
									for(int y=0; y<pagetable_size; y++) {
										if(page_t[z][y][0]==pop) {										
											page_t[z][y][1]=0; 
										}
									}
								}
								physical_stack[0][0]--; //stack_top-1
								break;
							}
							//------------------------------------------------------------------------------------------------------
							case 3: { //LFU
								min=physical_stack[1][1]; //access_count minimum
								min_index=1;
								min_alloc=physical_stack[1][0];
								for(int x=2; x<physical_stack[0][0]+1; x++) {
									//find minimum for LFU
									if(min>=physical_stack[x][1] && physical_stack[x][0]!=-1) {
										if(min==physical_stack[x][1]) { //If alloc_id is the same, replace the page with a small alloc_id.  
											if(min_alloc>physical_stack[x][0])
											{
												min=physical_stack[x][1];
												min_alloc=physical_stack[x][0];
												min_index=x;	
											}
										}
										else {
											min=physical_stack[x][1];
											min_alloc=physical_stack[x][0];
											min_index=x;
										}
									}
								}
								pop=physical_stack[min_index][0];
								for(int x=min_index+1; x<physical_stack[0][0]+1; x++) {
									//pull the stack down one space at a time
									physical_stack[x-1][0]=physical_stack[x][0]; 
									physical_stack[x-1][1]=physical_stack[x][1]; 
								}
								for(int y=0; y<physical_size; y++) {//check page_table[pid]
									if(physical_m[y]==pop) {
										physical_m[y]=-1; //swap out physical memory
									}
								}
								for(int z=0; z<P; z++) { // change the valid bit to 0
									for(int y=0; y<pagetable_size; y++) {
										if(page_t[z][y][0]==pop) {										
											page_t[z][y][1]=0; 
										}
									}
								}
								physical_stack[0][0]--; //stack_top-1
								break;
							}
							//------------------------------------------------------------------------------------------------------
							case 4: { //MFU
								max=physical_stack[1][1]; //access_count maximum
								max_index=1;
								max_alloc=physical_stack[1][0];
								for(int x=2; x<physical_stack[0][0]+1; x++) {
									//find maximum for MFU
									if(max<=physical_stack[x][1] && physical_stack[x][0]!=-1) {
										if(max==physical_stack[x][1]) { //If alloc_id is the same, replace the page with a small alloc_id.  
											if(max_alloc>physical_stack[x][0])
											{
												max=physical_stack[x][1];
												max_alloc=physical_stack[x][0];
												max_index=x;	
											}
										}
										else {
											max=physical_stack[x][1];
											max_alloc=physical_stack[x][0];
											max_index=x;
										}
									}
								}
								pop=physical_stack[max_index][0];
								for(int x=max_index+1; x<physical_stack[0][0]+1; x++) {
									//pull the stack down one space at a time
									physical_stack[x-1][0]=physical_stack[x][0]; 
									physical_stack[x-1][1]=physical_stack[x][1]; 
								}
								for(int y=0; y<physical_size; y++) {//check page_table[pid]
									if(physical_m[y]==pop) {
										physical_m[y]=-1; //swap out physical memory
									}
								}
								for(int z=0; z<P; z++) { // change the valid bit to 0
									for(int y=0; y<pagetable_size; y++) {
										if(page_t[z][y][0]==pop) {										
											page_t[z][y][1]=0; 
										}
									}
								}
								physical_stack[0][0]--; //stack_top--
								break;
							}
							//------------------------------------------------------------------------------------------------------
							case 5: { //Optimal
								//If future command has the same alloc_id, increase the "optimal" variable one by one.
								for(int x=i+1; x<count2; x++) { //count2 is number of instructions
									if(instruction[x][1]==0) { //function is access
										for(int y=1; y<physical_size; y++) {
											if(physical_stack[y][0]==instruction[x][2]) { //instruction[i][2] is alloc_id
												if(physical_stack[y][3]==0) { //To prevent the same thing from being accessed twice
													optimal+=1; 
													physical_stack[y][3]=optimal;	
												}								
											}
										}
									}
								}
								optimal_change=physical_stack[1][3]; //the last page to be used
								optimal_index=1;
								optimal_alloc=physical_stack[1][0];
								for(int x=2; x<physical_stack[0][0]+1; x++) { 
									//find the last page to be used for Optimal
									if(optimal_change!=0) { // If optimal is zero, replace it unconditionally. 
										if(physical_stack[x][3]==0 && physical_stack[x][0]!=-1) {
											optimal_change=physical_stack[x][3];
											optimal_index=x;		
										}
										else {
											if(optimal_change<=physical_stack[x][3] && physical_stack[x][0]!=-1) {
												if(optimal_change==physical_stack[x][3]) { 
												//If alloc_id is the same, replace the page with a small alloc_id.  
													if(optimal_alloc>physical_stack[x][0])
													{
														optimal_change=physical_stack[x][3];
														optimal_alloc=physical_stack[x][0];
														optimal_index=x;	
													}
												}
												else {
													optimal_change=physical_stack[x][3];
													optimal_alloc=physical_stack[x][0];
													optimal_index=x;
												}
											}
										}
									}
									else {  //If alloc_id is the same, replace the page with a small alloc_id. 
										if(physical_stack[x][3]==0 && physical_stack[x][0]!=-1) {
											if(physical_stack[x][0]<optimal_alloc) {
												optimal_change=physical_stack[x][3];
												optimal_alloc=physical_stack[x][0];
												optimal_index=x;
											}
										}
									}
								}
								pop=physical_stack[optimal_index][0];
								for(int x=optimal_index+1; x<physical_stack[0][0]+1; x++) {
									//pull the stack down one space at a time
									physical_stack[x-1][0]=physical_stack[x][0]; 
									physical_stack[x-1][3]=physical_stack[x][3]; 
								}
								for(int y=0; y<physical_size; y++) {//check page_table[pid]
									if(physical_m[y]==pop) {
										physical_m[y]=-1; //swap out physical memory
									}
								}
								for(int z=0; z<P; z++) { // change the valid bit to 0
									for(int y=0; y<pagetable_size; y++) {
										if(page_t[z][y][0]==pop) {										
											page_t[z][y][1]=0; 
										}
									}
								}
								physical_stack[0][0]--; //stack_top-1
								break;
							}
							
						}
						//------------------------------------------------------------------------------------------------------
						//Buddy Combine
						//physical_start[], physical_index_size[], physical_m[]
						//버디시스템에서 버디끼리는 한비트 차이난다. 따라서 xor한값과 xor한값에서 1뺀값을 and연산하면 0이 나와야 한다.
						for(int j=0; j<physical_size; j++) {
							physical_start[j]=j;
							physical_index_size[j]=1;
						}
						for(int j=0; j<5; j++) { //vary in physical size
							buddy=0;
							buddy_bound=1;
							for(int k=0; k<j; k++) {
								buddy_bound=buddy_bound*2;
							}
							while(buddy<physical_size-1) {
								Xor=physical_start[buddy]^physical_start[buddy+buddy_bound];
								temp_and=Xor-1;
								if(physical_start[buddy]==physical_start[buddy+buddy_bound])
									And=-1;
								else
									And=temp_and&Xor;
								// 둘 다 비어있고 주소차이가 1이고 사이즈가 같으면 합친다.
								if(And==0 && physical_index_size[buddy]==physical_index_size[buddy+buddy_bound] && physical_m[buddy]==-1 && physical_m[buddy+buddy_bound]==-1 && physical_index_size[buddy]==buddy_bound && physical_index_size[buddy+buddy_bound]==buddy_bound) {
									ttemp=physical_index_size[buddy];
									for(int w=0; w<ttemp; w++)
									{
										physical_index_size[buddy+w]*=2;
										physical_index_size[buddy+buddy_bound+w]*=2;
										physical_start[buddy+buddy_bound+w]=physical_start[buddy];
									}
								}
								buddy+=buddy_bound;						
							}
						}
						buddy_min_size=physical_size*2;
						buddy_min_index=physical_size;
						for(int j=0; j<physical_size; j++) {
							 if(bound<=physical_index_size[j] && physical_m[j]==-1) {
								if(buddy_min_size>physical_index_size[j]) {
									buddy_min_size=physical_index_size[j];
									buddy_min_index=physical_start[j];
								}
							 }
						}
						check_empty=buddy_min_index;
					}
					//------------------------------------------------------------------------------------------------------
					for(int j=check_empty; j<check_empty+bound; j++) //allocate AID to physical memory
					{
						physical_m[j]=alloc_id;
					}
					alloc_count=0;
					stack_top=physical_stack[0][0];
					physical_stack[stack_top][0]=alloc_id; //Place alloc_id on new page in top of stack
					physical_stack[stack_top][1]=0;
					physical_stack[stack_top][2]=1; //reference bit for Sampled LRU
					physical_stack[0][0]++; //stack_top++
					for(int j=1; j<physical_size+1; j++) {
						physical_stack[j][3]=0; //initialize optimal
					}
					optimal=0;
					page_fault++; //number of pagefault +1
				}
				//------------------------------------------------------------------------------------------------------
				else { //not require page replacement
					for(int j=check_empty; j<check_empty+bound; j++) //allocate AID to physical memory
					{
						physical_m[j]=alloc_id;
					}
					stack_top=physical_stack[0][0];
					physical_stack[stack_top][0]=alloc_id;
					physical_stack[stack_top][2]=1; //reference bit for Sampled LRU
					physical_stack[0][0]++; //stack_top+1(the initial value of stack_top is 1)
					alloc_count=0;
					page_fault++; //number of pagefault +1
				}
			}
			//------------------------------------------------------------------------------------------------------
			else { //When there is same alloc_id
				switch(choose) {
					case 0: //FIFO
						break;
					case 1: //LRU
						//The accessed id is raised to the top of the stack.
						for(int z=1; z<physical_size+1; z++)
						{
							if(physical_stack[z][0]==alloc_id) {
								//+1
								for(int t=z+1; t<physical_stack[0][0]+1; t++) {
									physical_stack[t-1][0]=physical_stack[t][0];
								}
								physical_stack[physical_stack[0][0]-1][0]=alloc_id;
							}
						}
						break;
					case 2: { //Sampled LRU
						for(int x=1; x<physical_size+1; x++) { 
							if(physical_stack[x][0]==alloc_id) 
								physical_stack[x][2]=1;
						}
						break;
					}
					case 3: //LFU 
						for(int z=1; z<physical_size+1; z++)
						{
							if(physical_stack[z][0]==alloc_id) {
								physical_stack[z][1]++; //access_count+1
							}
						}
						break;
					case 4: //MFU
						for(int z=1; z<physical_size+1; z++)
						{
							if(physical_stack[z][0]==alloc_id) {
								physical_stack[z][1]++; //access_count+1
							}
						}
						break;
					case 5: //Optimal
						break;
				}
				
			}
		}
		//tiem interveral for Sample LRU(time interval is 8), physical_stack[x][2] -> refernce bit, physical_stack[x][4] -> timeinterval
		//Reference bit initialization per time interrupt
		if(i%8==7) {
			for(int x=1; x<physical_size+1; x++) {
				if(physical_stack[x][2]==1) 
					physical_stack[x][4]=physical_stack[x][4]/10+10000000;
				else
					physical_stack[x][4]=physical_stack[x][4]/10;
				physical_stack[x][2]=0; //initialize reference bit
			}
		}
		/*
		for(int k=1; k<physical_stack[0][0]; k++) {
			cout<<"stack : "<<physical_stack[k][0]<<" ";
			cout<<"reference : "<<physical_stack[k][2]<<" ";
			cout<<"time : "<<physical_stack[k][4]<<" ";
			cout<<" |  ";
		}
		cout<<endl;		
		*/
	//---------------------------------------------------------------------------------------------------------------------------------------
	//print result
		if(instruction[i][1]==1)
			printf("* Input : Pid [%d] Function [%s] Alloc ID [%d] Page Num[%d]\n", pid, function.c_str(), alloc_id, demand_page); //Information about each line in the input file
		else {
			access_demand=0;
			for(int j=0; j<page_p[pid]; j++) //check page_table[pid]
			{
				if(page_t[pid][j][0]==alloc_id) {
					access_demand++; //count alloc_id 
				}
			}
			printf("* Input : Pid [%d] Function [%s] Alloc ID [%d] Page Num[%d]\n", pid, function.c_str(), alloc_id, access_demand);
		}
		printf("%-30s", ">> Physical Memory : ");
		int temp=0; //split 4
		for(int x=0; x<physical_size; x++) {
			if(temp%4==0)
				cout<<"|";
			if(physical_m[x]==-1)
				cout<<"-";
			else
				cout<<physical_m[x];
			temp++;
		}
		cout<<"|"<<endl;
		
		for(int w=0; w<P; w++) { //"w" means pid
			printf(">> pid(%d) %-20s",w, "Page Table(AID) : ");  // Status of the current page table (by Allocation ID)
			temp=0; //split 4
			for(int x=0; x<pagetable_size; x++) {
				if(temp%4==0)
					cout<<"|";
				if(page_t[w][x][0]==-1)
					cout<<"-";
				else
					cout<<page_t[w][x][0];
				temp++;
			}
			cout<<"|"<<endl;
			printf(">> pid(%d) %-20s",w, "Page Table(Valid) : "); //current page table status (by physical memory allocation)
			temp=0; //split 4
			for(int x=0; x<pagetable_size; x++) {
				if(temp%4==0)
					cout<<"|";
				if(page_t[w][x][1]==-1)
					cout<<"-";
				else
					cout<<page_t[w][x][1];
				temp++;
			}
			cout<<"|"<<endl;
		}
		cout<<endl;
	}
	printf("page fault = %d\n", page_fault); // Total number of page faults generated during simulation
	//---------------------------------------------------------------------------------------------------------------------------------------
	return 0;
}
