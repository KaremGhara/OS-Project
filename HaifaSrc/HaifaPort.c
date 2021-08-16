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
char* func_time();

DWORD WINAPI HaifaPort(PVOID Param);
void FinishAll();
void fromEilat();
int CheckVessNum(int);
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
 static int vesselNum;
 HANDLE rndMutex, mutex;
//Time Variable
time_t timeNow;
char timeInLocal[70];
int  *ptr,eleNum;


//Main that holds the Init of Pipe and the Init of the process and using the functions to complete the required tasks
int main() {
	printf("Enter vessel number!:");
	scanf_s("%d", &vesselNum);
	if (CheckVessNum(vesselNum) == 1) {
		
		
		
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
		
		if (initGlobalData() == FALSE)
		{
			printf("main::Unexpected Error in Global Semaphore Creation\n");
			return 1;
		}
		CloseHandle(inReadHandle);
		CloseHandle(outWriteHandle);
		HaifaToEilat(vesselNum);
		WaitForSingleObject(pi.hProcess, INFINITE);
		WaitForSingleObject(pi.hThread, INFINITE);
		fromEilat();
		FinishAll();
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}



//In this function we send the number of vessels to eilat through the pipe with WriteFile and Eilat confirms the number
//We Init the Vessel Threads after Eilat has confirmed the number that Haifa sent with CreateThread
//Each Thread has Unique ID
int HaifaToEilat(int vesselNum) {
	sprintf(buffer, "%d", vesselNum);
	if (!WriteFile(inWriteHandle, buffer, BUFFER_SIZE, &written, NULL)) {
		printf(stderr, "Error writing to pipe-father\n");
	}
	BOOL Approve = ReadFile(outReadHandle, buffer, BUFFER_SIZE, &read, NULL);
	if (Approve)
	{
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

//Here we check if the vessel Number is between 2 and 50 otherwise end te process
int CheckVessNum(int vesselNum) {
	if (vesselNum > maxVesselNUM || vesselNum < minVesselNUM) {
		printf("Vessel number should be between 2 and 50!- Exiting see you later!!");
		exit(0);
	}
	else return 1;
}


//Thread for Haifa Vessel
//We send the vessel to Eilat through the SendVessels function
DWORD WINAPI HaifaPort(PVOID Param) {
	int vesselID = *(int*)Param;
	fprintf(stderr,"\n%s - Vessel %d starts sailing @ Haifa port\n",func_time(),vesselID);
	Sleep(RandomizeSleep());
	SendVessels(vesselID);
	Sleep(RandomizeSleep());
	return 0;
	
}


//Function thats sends the vessel threads to Eilat with an array pointer throught the Pipe with WriteFile
void SendVessels(int vesID) {
	ptr = (int*)malloc(maxVesselNUM * sizeof(int));
	*ptr = vesID;
	Sleep(RandomizeSleep());
	for (int i = 0; i < vesselNum; i++)
	{
		WaitForSingleObject(mutex, INFINITE);
		ptr[i] = vesID;
		fprintf(stderr,"\n%s - Vessel %d - entering Canal: Med. Sea ==> Red. Sea\n",func_time() ,vesID);
		
		if (!WriteFile(inWriteHandle, &ptr[i], sizeof(int), &written, NULL)) {
			printf(stderr, "Error writing to pipe-father\n");
		}

		else {
			if (!ReleaseMutex(mutex)) { printf("Send Vessels::Unexpected error mutex.V()\n"); }
			WaitForSingleObject(vesselThread[i], INFINITE);			
			

		}
		free(ptr);
	}
	return 0;
}
//Function the Welcomes the Vessel from 
void fromEilat() {
	for (int i = 0; i < vesselNum; i++)
	{
		ptr[i] = 0;
		WaitForSingleObject(mutex, INFINITE);
		if (ReadFile(outReadHandle, &ptr[i], sizeof(int), &read, NULL)) {
			fprintf(stderr, "\n%s - Vessel %d done sailing @ Haifa\n", func_time(), ptr[i]);
			Sleep(RandomizeSleep());
		}
		if(!ReleaseMutex(mutex)) printf("Vessel Thread::Unexpected Error in arriving Vessel %d \n", i);

	}
}

//Time function	
char* func_time() {
	 static char currTimeNow[20];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(currTimeNow,"[%02d:%02d:%02d]", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return currTimeNow;
	
}



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

	return TRUE;
}


//This funcion was taken from example code in class
//This function generates random number to use in Sleep function
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
void FinishAll()
{
	for (int i = 0; i < vesselNum; i++)
	{
		CloseHandle(vesselThread[i]);
	}
	fprintf(stderr, "\n%s - Haifa Port: All Vessel Threads are done\n",func_time());
	CloseHandle(mutex);
	CloseHandle(rndMutex);

	
}
