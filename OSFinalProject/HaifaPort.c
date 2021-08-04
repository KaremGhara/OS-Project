#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include<time.h>
#define minVesselNUM 2
#define maxVesselNUM 50
#define BUFFER_SIZE 100
#define MAX_SLEEP_TIME 3000 //3 Seconds
#define MIN_SLEEP_TIME 500



//Functions
DWORD WINAPI HaifaPort(PVOID Param);
int CheckVessNum(int);
int InitPipes();
int InitProcces();;
int InitThreads(int);
int HaifaToEilat(int);
int RandomizeSleep();
//void Time(char*);
void SendVessels(int);
BOOL initGlobalData();

//Variables
DWORD ThreadID;
int vesselThreadID[maxVesselNUM];
HANDLE  vesselThread[maxVesselNUM];
TCHAR ProcessName[256];
HANDLE inReadHandle, inWriteHandle;
HANDLE outReadHandle, outWriteHandle;
STARTUPINFO si;
PROCESS_INFORMATION pi;
DWORD written,read;
char buffer[BUFFER_SIZE];
int vesselNum;
HANDLE rndMutex, mutex,sem[maxVesselNUM];
//Time Variable
time_t timeNow;
char timeInLocal[70];
int* dynamicNumber;


int main(int argc,char*argv[]) {
	/*vesselNum =atoi(argv[1]);*/
	printf("Enter vessel number!:");
	scanf_s("%d", &vesselNum);
	if (CheckVessNum(vesselNum) == 1) {
		
		//Time(timeInLocal);
		/*InitPipes();
		InitProcces();*/
		/*HaifaToEilat(vesselNum);*/
		
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL,TRUE };
		ZeroMemory(&pi, sizeof(pi));
		//Pipe to father to son(Haifa to Eilat)
		if (!CreatePipe(&inReadHandle, &inWriteHandle, &sa, 0)) {
			fprintf(stderr, "Create Pipe(Parent) Failed\n");
			return 1;
		}
		//Pipe to wrirte Child to Parent(From Eilat to Haifa) 
		if (!CreatePipe(&outReadHandle, &outWriteHandle, &sa, 0)) {
			fprintf(stderr, "Create Pipe(Child) Failed\n");
			return 1;
		}

		GetStartupInfo(&si);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdInput = inReadHandle;
		si.hStdOutput = outWriteHandle;
		si.dwFlags = STARTF_USESTDHANDLES;

		wcscpy(ProcessName, L"Eilat.exe");

		if (!CreateProcessW(NULL,
			ProcessName,
			NULL,
			NULL,
			TRUE, /* inherit handles */
			0,
			NULL,
			NULL,
			&si,
			&pi))
		{
			fprintf(stderr, "Process Creation Failed\n");
			return -1;
		}
		/*CloseHandle(outWriteHandle);
		CloseHandle(inReadHandle);*/
		if (initGlobalData() == FALSE)
		{
			printf("main::Unexpected Error in Global Semaphore Creation\n");
			return 1;
		}
		
		
		HaifaToEilat(vesselNum);
		WaitForSingleObject(pi.hProcess, INFINITE);
		WaitForSingleObject(pi.hThread, INFINITE);

	}
}

//int InitPipes()
//{
//	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL,TRUE };
//	ZeroMemory(&pi, sizeof(pi));
//	//Pipe to father to son(Haifa to Eilat)
//	if (!CreatePipe(&inReadHandle, &inWriteHandle, &sa, 0)) {
//		fprintf(stderr, "Create Pipe(Parent) Failed\n");
//		return 1;
//	}
//	//Pipe to wrirte Child to Parent(From Eilat to Haifa) 
//	if (!CreatePipe(&outReadHandle, &outWriteHandle, &sa, 0)) {
//		fprintf(stderr, "Create Pipe(Child) Failed\n");
//		return 1;
//	}
//
//	GetStartupInfo(&si);
//	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
//	si.hStdInput = inReadHandle;
//	si.hStdOutput = outWriteHandle;
//	si.dwFlags = STARTF_USESTDHANDLES;
//
//}
//int InitProcces() {
//	wcscpy(ProcessName, L"Eilat.exe");
//
//	if (!CreateProcessW(NULL,
//		ProcessName,
//		NULL,
//		NULL,
//		TRUE, /* inherit handles */
//		0,
//		NULL,
//		NULL,
//		&si,
//		&pi))
//	{
//		fprintf(stderr, "Process Creation Failed\n");
//		return -1;
//	}
//}


int HaifaToEilat(int vesselNum) {
	sprintf(buffer, "%d", vesselNum);
	if (!WriteFile(inWriteHandle, buffer, BUFFER_SIZE, &written, NULL)) {
		printf(stderr, "Error writing to pipe-father\n");
	}
	BOOL Approve = ReadFile(outReadHandle, buffer, BUFFER_SIZE, &read, NULL);
	if (Approve)
	{
		printf("\nYOOOOOO\n");
		int approveMent = atoi(buffer);
		if (approveMent == vesselNum);
		{

			for (int i = 1; i <= vesselNum; i++)
			{
				vesselThreadID[i - 1] = i;
				vesselThread[i - 1] = CreateThread(NULL, 0, HaifaPort, &vesselThreadID[i - 1], 0, &ThreadID);
				if (vesselThread[i - 1] == NULL)
				{
					printf("Vessel Thread::Unexpected Error in Vessel %d  Creation\n", i);
					return 0;
				}


			}

		}

	}
}


//int InitThreads(int vesselNum) {
//	for (int i = 1; i <=vesselNum; i++)
//	{
//		vesselThreadID[i - 1] = i;
//		vesselThread[i - 1] = CreateThread(NULL, 0, HaifaPort, &vesselThreadID[i - 1], 0, &ThreadID);
//		if (vesselThread[i-1] == NULL)
//		{
//			printf("main::Unexpected Error in Vessel %d  Creation\n", i);
//			return 0;
//		}
//
//	}
//
//}



int CheckVessNum(int vesselNum) {
	if (vesselNum > maxVesselNUM || vesselNum < minVesselNUM) {
		printf("Vessel number should be between 2 and 50!- Ending the whole Process!");
		exit(0);
	}
	else return 1;
}

//void Time(char output) {
//	time_t rawtime=time(NULL);
//	struct tm* timeInfo;
//	time(&rawtime);
//	timeInfo = localtime(&rawtime);
//	sprintf(output, "[%d:%d:%d]", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
//}

DWORD WINAPI HaifaPort(PVOID Param) {
	int vesselID = *(int*)Param;
	printf("\nVessel %d starts sailing @ Haifa port\n", vesselID);
	Sleep(RandomizeSleep());
	SendVessels(vesselID);
	Sleep(RandomizeSleep());
	return 0;
	
}
//DWORD WINAPI HaifaPort(PVOID Param){
//
//	int vesID = *(int*)(Param);
//	Sleep(RandomizeSleep());
//	printf("\nVessel %d starts sailing @ Haifa port\n", vesID);
//	Sleep(RandomizeSleep());
//	return 0;
//	
//	
//}

//int WriteVessel(int vesID) {
//	for (int i = 0; i <vesselNum; i++)
//	{
//		if (!WriteFile(inWriteHandle, &vesID, sizeof(int), &written, NULL))
//			fprintf(stderr, "Error writing to pipe-father\n");
//	}
//}

void SendVessels(int vesID) {
	
	
	for (int i = 0; i < vesselNum; i++)
	{
		WaitForSingleObject(mutex, INFINITE);
		printf("\nVessel %d - entering Canal: Med. Sea ==> Red. Sea\n", vesID);
		if (!WriteFile(inWriteHandle, &vesID, sizeof(int), &written, NULL)) {
			printf(stderr, "Error writing to pipe-father\n");
		}
		else {
			if (!ReleaseMutex(mutex)) { printf("Send Vessels::Unexpected error mutex.V()\n"); }
			//	WaitForSingleObject(sem[vesID], INFINITE);
			WaitForSingleObject(vesselThread[i], INFINITE);			
			

		}
	}
	return 0;
}
	
//	
//
//	
//			
//}



//Initialise global Semaphores
//If all Successful - return True, otherwise (if problem arises) return False
//This is invoked before all Vessels Threads start running
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

	for (i = 0; i <vesselNum; i++)
	{
		sem[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (sem[i] == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}


//This funcion was taken from example code in class
int RandomizeSleep()
{
	int res;

	WaitForSingleObject(rndMutex, INFINITE);
	res = rand() % MAX_SLEEP_TIME+MIN_SLEEP_TIME + 1;
	if (!ReleaseMutex(rndMutex))
		printf("calcSleepTime::Unexpected error rndMutex.V()\n");

	return res;
}

//Close all global semaphore handlers - after all Vessels Threads finish.
void cleanupGlobalData()
{
	int i;

	CloseHandle(mutex);
	CloseHandle(rndMutex);

	for (i = 0; i < vesselNum; i++)
		CloseHandle(sem[i]);
	
}
