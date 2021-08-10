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

//Functions
char* func_time();
void readVesselNum();
void readVessels();
void InitCrane(int);
BOOL checkDivisor(int craneNum);
DWORD WINAPI vessThreads(PVOID Param);
DWORD WINAPI craneeThreads(PVOID Param);
DWORD WINAPI barrierFun(PVOID Param[]);
int RandomNumber();
void InitBarrier();
//Variables
HANDLE barrierThread;
int cnt = 0;
int *vessels;
int vesselNum;
char buffer[BUFFER_SIZE];
BOOL isRead;
HANDLE ReadHandle, WriteHandle, mutex, rndMutex, sem[maxVesselNum],barrierMutex;
DWORD read, written, ThreadID;
HANDLE vesThreads[maxVesselNum],craneThreads[maxVesselNum];
int vesThreadsID[maxVesselNum], barrierArr[maxVesselNum];
int cranesID[maxVesselNum];
int craneNum,vesID;
int vesArr[maxVesselNum];
int* vess;
SYNCHRONIZATION_BARRIER sb;

void main(VOID)
{
	if (initGlobalData() == FALSE) {
		printf("main::Unexpected Error in Global Semaphore Creation\n");

	}
	readVesselNum();
	readVessels();
	InitCrane(vesselNum);
	//InitVesID();
	InitBarrier(vesID);



}

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
//
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

			//vesThreadsID[i] = vessels[i];

		}
	}
	
}

void InitCrane(int vesselNum) {
	int craneNum = RandomNumber();
	fprintf(stderr, "\n Rand : %d\n", craneNum);
	while (Divisor(craneNum)) {
		fprintf(stderr, "\n Rand : %d\n", craneNum);
		craneNum = RandomNumber(); 
		break;
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


BOOL Divisor(int randNum) {
	if ((vesselNum % randNum != 0) || (randNum == vesselNum) || (randNum == 1))
		return FALSE;
	else
		return TRUE;
}


//int checkDivisor(int craneNum) {
//	
//	if (craneNum == 2) {
//		return 2;
//	}
//	for (int i = 3; i < craneNum; i++)
//	{
//		if (((craneNum % i) == 0) && (craneNum > 1) && (craneNum < vesselNum)) {
//			return i;
//		}
//		else return 0;
//	}
//
//	
//	
//}

int checkPrime(int vesselNum)
{
	for (int i = 2; i < vesselNum; i++) {
		if (vesselNum % i == 0 && i != vesselNum) return 1;
	}
	return 0;
}

DWORD WINAPI vessThreads(PVOID Param) {
	
		vesID = *(int*)Param;
		vesArr[cnt] = vesID;
		cnt++;
		Sleep(RandomizeSleep());
		fprintf(stderr, "\n%s - Vessel %d arrived @ Eilat Port!\n", func_time(), vesID);
		Sleep(RandomizeSleep());
		
	
	return 0;
}


//void InitVesID() {
//	for (int i = 0; i < vesselNum; i++)
//	{
//		vesArr[i] = vesThreadsID[i];
//	}
//
//	
//
//}
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
}

DWORD WINAPI craneeThreads(PVOID Param) {
	
	int craneID = *(int*)Param;
	//Sleep(RandomizeSleep());
	fprintf(stderr,"\n%s - Crane %d has been created!\n",func_time() ,craneID);
	Sleep(RandomizeSleep());
	return 0;
}
int RandomNumber() {
	WaitForSingleObject(rndMutex, INFINITE);
	srand(time(NULL));
	int res =((rand())% (vesselNum+1));
	
	if (!ReleaseMutex(rndMutex))
		printf("RandomNumber::Unexpected error rndMutex.V()\n");
	return res;
}

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

	for (i = 0; i < vesselNum; i++)
	{
		sem[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (sem[i] == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

char* func_time() {
	static char currTimeNow[20];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(currTimeNow, "[%02d:%02d:%02d]", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return currTimeNow;

}

int RandomizeSleep()
{
	int res;

	WaitForSingleObject(rndMutex, INFINITE);
	res = rand() % MAX_SLEEP_TIME + MIN_SLEEP_TIME + 1;
	if (!ReleaseMutex(rndMutex))
		printf("RandomizeSleep::Unexpected error rndMutex.V()\n");

	return res;
}