#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <mpi.h>
#include "ShellSort.h"

#define ARR_SIZE 1000000

inline int findFirstSet(int n)
{
	if (!n)
		return 0;

	int i;
	for (i = 0; !(n & 1); ++i)
		n >>= 1;
	return i;
}

void generateRandomArray(TYPE* arr, int n)
{
	if (n > 0)
		for (int i = 0; i < n; i++)
			arr[i] = (TYPE)rand();
}

void printArray(int* arr, int size)
{
	for (int i = 0; i < size; ++i)
		std::cout << arr[i] << ' ';
	std::cout << std::endl;
}

void calcSendCountsDispls(int* sendcounts, int* displs, int n, int procNum)
{
	int sum = 0, rem = n % procNum;
	for (int i = 0; i < procNum; i++) {
		sendcounts[i] = n / procNum;
		if (rem > 0) {
			sendcounts[i]++;
			rem--;
		}

		displs[i] = sum;
		sum += sendcounts[i];
	}
}

void calcMergeBuffSize(int* mergeBuffSize, int* sendcounts, int procNum, int procRank)
{
	for (int i = 1; ((procRank | i) > procRank) && ((procRank | i) < procNum); i <<= 1)
	{
		if (mergeBuffSize[procRank | i] == 0)
			calcMergeBuffSize(mergeBuffSize, sendcounts, procNum, procRank | i);
		mergeBuffSize[procRank] += mergeBuffSize[procRank | i];
	}
	mergeBuffSize[procRank] += sendcounts[procRank];
}

using namespace std;

int main(int argc, char *argv[])
{
	TYPE* mergeBuff = nullptr, *recBuff = nullptr, *arr = nullptr;
	double startTime, parTime, linTime;
	int procRank, procNum;
	int* sendcounts, * displs, * mergeBuffSize;
	MPI_Status status;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);

	if (procRank == 0)
	{
		srand(time(NULL));
		arr = new TYPE[ARR_SIZE];
		generateRandomArray(arr, ARR_SIZE);
		startTime = MPI_Wtime();
	}

	sendcounts = new int[procNum];
	displs = new int[procNum];
	mergeBuffSize = new int[procNum];
	for (int i = 0; i < procNum; ++i)
		mergeBuffSize[i] = 0;

	calcSendCountsDispls(sendcounts, displs, ARR_SIZE, procNum);
	calcMergeBuffSize(mergeBuffSize, sendcounts, procNum, 0);
	mergeBuff = new TYPE[mergeBuffSize[procRank]];

	TYPE* curBuff = mergeBuff + mergeBuffSize[procRank] - sendcounts[procRank];
	int curSize = sendcounts[procRank];
	MPI_Scatterv(arr, sendcounts, displs, MPI_TYPE, curBuff, sendcounts[procRank], MPI_TYPE, 0, MPI_COMM_WORLD);
	shellSort(curBuff, sendcounts[procRank]);

	for (int i = 1; ((procRank | i) > procRank) && ((procRank | i) < procNum); i <<= 1)
	{
		int pSender = procRank | i;
		int recCount = mergeBuffSize[pSender];
		recBuff = new TYPE[recCount];
		MPI_Recv(recBuff, recCount, MPI_TYPE, pSender, 0, MPI_COMM_WORLD, &status);
		merge(curBuff, curSize, recBuff, recCount, curBuff - recCount);
		curSize += recCount;
		curBuff -= recCount;
		delete[] recBuff;
	}
	if (procRank)
	{
		int pReceiver = procRank - (1 << findFirstSet(procRank));
		MPI_Send(mergeBuff, mergeBuffSize[procRank], MPI_TYPE, pReceiver, 0, MPI_COMM_WORLD);
	}

	if (procRank == 0)
	{
		parTime = MPI_Wtime() - startTime;
		startTime = MPI_Wtime();
		shellSort(arr, ARR_SIZE);
		linTime = MPI_Wtime() - startTime;

		for (int i = 0; i < ARR_SIZE; ++i)
			if (arr[i] != mergeBuff[i])
				cout << i << ' ';

		cout << "Parallel sort time:  " << parTime << endl;
		cout << "Linear sort time:    " << linTime << endl;

		delete[] arr;
	}
	delete[] mergeBuff;

	MPI_Finalize();
}
