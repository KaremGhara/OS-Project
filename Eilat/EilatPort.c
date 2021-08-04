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
void readVesselNum();
void readVessels();
void InitCrane(int);
DWORD WINAPI vessThreads(PVOID Param);
DWORD WINAPI craneeThreads(PVOID Param);
int RandomNumber(int);
void toBarrier(int);

//Variables
int vessels[maxVesselNum];
int vesselNum;
char buffer[BUFFER_SIZE];
BOOL isRead;
HANDLE ReadHandle, WriteHandle, mutex, rndMutex, sem[maxVesselNum];
DWORD read, written, ThreadID;
HANDLE vesThreads[maxVesselNum],craneThreads[maxVesselNum];
int vesThreadsID[maxVesselNum];
int cranesID[maxVesselNum];

void main(VOID)
{

	readVesselNum();
	readVessels();
	if (initGlobalData() == FALSE) {
		printf("main::Unexpected Error in Global Semaphore Creation\n");
		
	}
	InitCrane(vesselNum);

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

	for (int i = 1; i <= vesselNum; i++)
	{
		isRead = ReadFile(ReadHandle, &vessels[i - 1], sizeof(int), &read, NULL);
		if (isRead) {
			vesThreadsID[i - 1] = i;
			vesThreads[i - 1] = CreateThread(NULL, 0, vessThreads, &vesThreadsID[i - 1], 0, &ThreadID);
			Sleep(RandomizeSleep());
			if (vesThreads[i - 1] == NULL) {
				printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i);
				exit(0);
			}
		}
	}
}

void InitCrane(vesselNum) {
	int craneNum = RandomNumber(vesselNum);
	for (int i = 1; i <= craneNum; i++)
	{
		cranesID[i - 1] = i;
		craneThreads[i - 1] = CreateThread(NULL, 0, craneeThreads, &cranesID[i - 1], 0, &ThreadID);
		Sleep(RandomizeSleep());
		if (vesThreads[i - 1] == NULL) {
			printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i);
			exit(0);
		}
	}
}



int checkPrime(int vesselNum)
{
	for (int i = 2; i < vesselNum; i++) {
		if (vesselNum % i == 0 && i != vesselNum) return 1;
	}
	return 0;
}

DWORD WINAPI vessThreads(PVOID Param) {
	int vesID = *(int*)Param;
	Sleep(RandomizeSleep());
	fprintf(stderr, "\nVessel %d arrived in Eilat!\n", vesID);
	toBarrier(vesID);
	return 0;
	  
}
void toBarrier(int vesID) {

}


DWORD WINAPI craneeThreads(PVOID Param) {

	int craneID = *(int*)Param;
	Sleep(RandomizeSleep());
	fprintf(stderr,"\nCrane %d has been created!\n", craneID);
	Sleep(RandomizeSleep());
	return 0;
}
int RandomNumber(int vesselNum) {
	WaitForSingleObject(rndMutex, INFINITE);
	srand(time(NULL));
	int res =rand()% vesselNum +1;
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
int RandomizeSleep()
{
	int res;

	WaitForSingleObject(rndMutex, INFINITE);
	res = rand() % MAX_SLEEP_TIME + MIN_SLEEP_TIME + 1;
	if (!ReleaseMutex(rndMutex))
		printf("RandomizeSleep::Unexpected error rndMutex.V()\n");

	return res;
}