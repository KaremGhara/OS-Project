#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>



#define MIN_SLEEP_TIME 500
#define MAX_SLEEP_TIME 3000
#define maxVesselNum 50
#define BUFFER_SIZE 100
#define CARGO_MIN 5
#define CARGO_MAX 50

//Functions
int RandomWeight();
void FinishAll();
void sendVessel(int[]);
void toHaifa();
char* func_time();
void InitADT(cargoWeight, vesArr, craneNum);
void readVesselNum();
void readVessels();
void InitCrane(int);
DWORD WINAPI backToHaifa(PVOID Param);
DWORD WINAPI vessThreads(PVOID Param);
DWORD WINAPI craneeThreads(PVOID Param);
DWORD WINAPI barrierFun(PVOID Param[]);
int RandomNumber();
void InitBarrier();
//Variables
HANDLE barrierThread;
int cnt = 0, counter = 0;
int *vessels;
 static int vesselNum;
char buffer[BUFFER_SIZE];
BOOL isRead;
HANDLE ReadHandle, WriteHandle, mutex, rndMutex,barrierMutex;
DWORD read, written, ThreadID;
HANDLE vesThreads[maxVesselNum],craneThreads[maxVesselNum];
int vesThreadsID[maxVesselNum], barrierArr[maxVesselNum];
int cranesID[maxVesselNum];
int craneNum,vesID;
int vesArr[maxVesselNum],cargoWeight[maxVesselNum];
int* vess;
SYNCHRONIZATION_BARRIER sb;

//Main thats Init the required function that we need
void main(VOID)

{
	if (initGlobalData() == FALSE) {
		printf("main::Unexpected Error in Global Semaphore Creation\n");

	}
	readVesselNum();
	RandomWeight();
	readVessels();
	InitCrane(vesselNum);
	InitBarrier(vesID);
	InitADT(cargoWeight, vesArr,craneNum);
	toHaifa();
	sendVessel(vesArr);
	FinishAll();
}

//Function that welcomes you and  checks the vessel number that has been sent from Haifa 
void readVesselNum() {
	fprintf(stderr, "\nEilat is Welcoming you!\n");
	ReadHandle = GetStdHandle(STD_INPUT_HANDLE);
	WriteHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	isRead = ReadFile(ReadHandle, buffer, BUFFER_SIZE, &read, NULL);
	if (isRead) {
		vesselNum = atoi(buffer);
		fprintf(stderr, "\nHaifa sent %d vessels to Eilat port\n", vesselNum);
		if (checkPrime(vesselNum) == 1) {
			sprintf(buffer, "%d", vesselNum);
			printf("\nHaifa sent %d vessels to Eilat port\n", vesselNum);
			if (!WriteFile(WriteHandle, buffer, BUFFER_SIZE, &written, NULL))
				fprintf(stderr, "\nEilat:Error writing to the pipe\n");
		}
		else {
			printf("Sorry!"); exit(0);
		}
	}
}
//Function that Welcomes the vessel Threads that being sent from Haifa with an array pointer and creating thread for each Vessel
void readVessels() {
	vessels = (int*)malloc(vesselNum * sizeof(int));
	for (int i = 0; i <vesselNum; i++)
	{
		isRead = ReadFile(ReadHandle, &vessels, sizeof(int), &read, NULL);
		if (isRead) {
			WaitForSingleObject(mutex, INFINITE);
			vesThreads[i]  = CreateThread(NULL, 0, vessThreads, &vessels, 0, &ThreadID);
			Sleep(RandomizeSleep());

			if (vesThreads[i] == NULL) {
				printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i);
				exit(0);
			}
			if(!ReleaseMutex(mutex)) printf("Mutex::Unexpected Error\n");


		}
	}
	
}
//Function that Init the crane number and randomize the crane number with randomize function and a divider function
//and creating thread for each Crane with a Unique ID for each Crane
void InitCrane(int vesselNum) {
	craneNum = RandomNumber();
	while (funDevider(craneNum,vesselNum) == 0) {

		craneNum = RandomNumber();
	}

		for (int i = 0; i <	 craneNum; i++)
		{
			cranesID[i] = i+1;
			craneThreads[i] = CreateThread(NULL, 0, craneeThreads, &cranesID[i], 0, &ThreadID);
			Sleep(RandomizeSleep());
			if (vesThreads[i] == NULL) {
				printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i+1);
				exit(0);
			}
		}
	}


//Function that check the dividors of a number and check the right conditions of the crane number
int funDevider(int craneNum,int vesselNum) {

	if (vesselNum % craneNum != 0) {
		return 0;
	}

	if (craneNum == 2) {
		
		return craneNum;
	}
	for (int i = 2; i <= craneNum; i++)
	{
		if (((craneNum % i) == 0) && (craneNum > 1) && (craneNum < vesselNum)) {
			
			return i;
		}
	}
	return 0;
}



//Function that checks if the number is prime or not
int checkPrime(int vesselNum)
{
	for (int i = 2; i < vesselNum; i++) {
		if (vesselNum % i == 0 && i != vesselNum) return 1;
	}
	return 0;
}
//Vessel thread
DWORD WINAPI vessThreads(PVOID Param) {
	
		vesID = *(int*)Param;
		vesArr[cnt] = vesID;
		cnt++;
		Sleep(RandomizeSleep());
		fprintf(stderr, "\n%s - Vessel %d arrived @ Eilat Port!\n", func_time(), vesID);
		Sleep(RandomizeSleep());
		
	
	return 0;
}


//Function that Init the Barrier with a Thread
void InitBarrier() {
	 barrierThread = CreateThread(NULL, 0, barrierFun,NULL , 0, NULL);
	 Sleep(RandomizeSleep());
	 if (barrierThread == NULL) {
		 printf("Barrier Thread::Unexpected Error in Vessel Creation\n");
		 exit(0);
	 }
	

}
//Here we let every vessel enter the barrier with an array and we check the num of vessel(threads) that in the array if the vessel
	//num is equal to vesselNum we open the barrier! and print a good message to inform the user(me:D)!
DWORD WINAPI barrierFun(PVOID Param[]) {
	fprintf(stderr, "\n%s - Barrier is ready to welcome the Vessels\n",func_time());
	WaitForSingleObject(barrierMutex, INFINITE);
	
	for (int i = 0; i < vesselNum; i++)
	{
		fprintf(stderr,"\n%s - Vessel %d is ready to  enter the  Barrier\n", func_time(), vesArr[i]);
		
	}
	Sleep(RandomizeSleep());
	if (!ReleaseMutex(barrierMutex))
		printf("barrier::Unexpected error barrierMutex.V()\n");
	for (int i = 0; i < vesselNum; i++)
	{

		
		
		fprintf(stderr, "\n%s - Vessel %d has entered the  Barrier with %d ton\n", func_time(), vesArr[i], cargoWeight[i]);
		
	}
	fprintf(stderr, "\n%s - Vessels is entering the Unloading Quay\n", func_time());
	return 0;
}

//Function that Init the Unloading Quay and let the crane work with the vesses threads with random Weight that we genereated from RandomWeight function
void InitADT(int cargoWeight[], int vesArr[], int  craneNum) {
	int tmp = 0;
	Sleep(RandomizeSleep());
	for (int i = 0; i < vesselNum; i++)
	{
		tmp = i;
		
		for (int j = 0; j < craneNum; j++)
		{
			fprintf(stderr, "\n%s - Crane %d is working  with Vessel %d with %d ton\n", func_time(), j+1,vesArr[i] ,cargoWeight[i]);
			i++;
		}
		Sleep(RandomizeSleep());

		for (int k = 0; k < craneNum; k++)
		{

			WaitForSingleObject(mutex, INFINITE);
			fprintf(stderr, "\n%s - Crane %d finished working with Vessel %d thats has %d ton\n", func_time(), k + 1, vesArr[tmp], cargoWeight[tmp]);
			tmp++;
		}


		Sleep(RandomizeSleep());

		if(!ReleaseMutex(mutex)) printf("ADT::Unexpected errorMutex.V()\n");

		i--;
	}
	
 }
//Function that sends back the vessels threads to Haifa by creating a new thread with the same Unique ID to them
void toHaifa() {
	for (int i = 0; i < vesselNum; i++)
	{		
			WaitForSingleObject(mutex, INFINITE);
			vesThreads[i] = CreateThread(NULL, 0, backToHaifa, &vesArr[i], 0, &ThreadID);
			Sleep(RandomizeSleep());

			if (vesThreads[i] == NULL) {
				printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i);
				exit(0);
			}
			if (!ReleaseMutex(mutex)) printf("Mutex::Unexpected Error\n");

		
	}
}

//thread function to send the vessels back to Haifa
DWORD WINAPI backToHaifa(PVOID Param) {
	int vesselID = *(int*)Param;
	fprintf(stderr, "\n%s - Vessel %d starts sailing @ Eilat Port\n", func_time(), vesselID);
	return 0;
}
//Function the Writes the vessels threads to send them back to Haifa with WriteFile
void sendVessel(int vesArr[]) {
	Sleep(RandomizeSleep());
	for (int i = 0; i < vesselNum; i++)
	{
		WaitForSingleObject(mutex, INFINITE);
		fprintf(stderr, "\n%s - Vessel %d - entering Canal: Red. Sea ==> Med. Sea\n", func_time(), vesArr[i]);
		Sleep(RandomizeSleep());
		fprintf(stderr, "\n%s - Vessel %d exiting canal: Red sea  ==> Med. Sea \n", func_time(), vesArr[i]);
		if (!WriteFile(WriteHandle, &vesArr[i], sizeof(int), &written, NULL)) {
			printf(stderr, "Error writing to pipe-father\n");
		}


		else {
			if (!ReleaseMutex(mutex)) { printf("Send Vessels::Unexpected error mutex.V()\n"); }
			//	WaitForSingleObject(sem[vesID], INFINITE);

		}

	}
	return 0;
}
//Function that generate random Weight for each vessel and container them in an array 
int RandomWeight() {
	int cargo;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < vesselNum; i++)
	{
		 cargo= (rand() % (CARGO_MAX - CARGO_MIN + 1)) + CARGO_MIN;
		 cargoWeight[i] = cargo;
		 

	}

}






//Crane thread for each Crane
DWORD WINAPI craneeThreads(PVOID Param) {
	
	int craneID = *(int*)Param;
	//Sleep(RandomizeSleep());
	fprintf(stderr,"\n%s - Crane %d has been created!\n",func_time() ,craneID);
	Sleep(RandomizeSleep());
	return 0;
}


//Function that generate random number for cranes
int RandomNumber() {
	srand(time(0));
	WaitForSingleObject(rndMutex, INFINITE);
	int res =((rand())% (vesselNum)+1);
	if (!ReleaseMutex(rndMutex)) printf("rndMutex::Unexpected error barrierMutex.V()\n");
	return res;
}
//Init mutex,barrierMutex,semaphors
BOOL initGlobalData()
{
	int i;

	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		return FALSE;
	}
	barrierMutex = CreateMutex(NULL, FALSE, NULL);
	if (barrierMutex==NULL) {
		return FALSE;
	}
	rndMutex = CreateMutex(NULL, FALSE, NULL);
	if (rndMutex == NULL)
	{
		return FALSE;
	}

	return TRUE;
}
//Time function	
char* func_time() {
	static char currTimeNow[20];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(currTimeNow, "[%02d:%02d:%02d]", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return currTimeNow;

}

//function that generates random number for Sleep function
int RandomizeSleep()
{
	int res;

	WaitForSingleObject(rndMutex, INFINITE);
	res = rand() % MAX_SLEEP_TIME + MIN_SLEEP_TIME + 1;
	if (!ReleaseMutex(rndMutex))
		printf("RandomizeSleep::Unexpected error rndMutex.V()\n");

	return res;
}
//function that Closes all the Handels
void FinishAll()
{
	fprintf(stderr, "\n%s - Eilat Port: All Vessel Threads are done\n", func_time());
	CloseHandle(mutex);
	CloseHandle(rndMutex);
	CloseHandle(barrierMutex);
	free(vessels);
}