#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "mpi.h"

int* generateRandomArray(int size)
{
	int *arr = NULL, i;
	if (size > 0)
	{
		arr = (int*)malloc(size * sizeof(int));
		for (i = 0; i < size; i++)
			arr[i] = rand();
	}
	return arr;
}
int findMaxInArray(int* arr, int size)
{
	int max = INT_MIN, i;
	for (i = 0; i < size; i++)
		if (arr[i] > max)
			max = arr[i];
	return max;
}

int main(int argc, char* argv[]) {
	int ProcNum, ProcRank;
	int max = INT_MIN, i, arrayLength;
	int localMax;
	double startTime, endTime;
	MPI_Status Status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if (ProcRank == 0)
		arrayLength = atoi(argv[1]);
	MPI_Bcast(&arrayLength, 1, MPI_INT, 0, MPI_COMM_WORLD);

	int step = arrayLength / ProcNum;

	if (ProcRank == 0) {
		srand(time(NULL));
		int* arr = generateRandomArray(arrayLength);
		//for (i = 0; i < arrayLength; i++)
		//	printf("%6d", arr[i]);
		startTime = MPI_Wtime();
		for (i = 1; i < ProcNum - 1; i++)
			MPI_Send(arr + step * i, step, MPI_INT, i, 0, MPI_COMM_WORLD);
		if (ProcNum > 1)
			MPI_Send(arr + step * i, arrayLength - step * (ProcNum - 1), MPI_INT, i, 0, MPI_COMM_WORLD);
		max = findMaxInArray(arr, step);
		free(arr);
	}
	else
	{
		int localSize = (ProcRank != (ProcNum - 1)) ? step : (arrayLength - step * (ProcNum - 1));
		int* recvBuf = malloc(localSize * sizeof(int));
		MPI_Recv(recvBuf, localSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
		localMax = findMaxInArray(recvBuf, localSize);
		free(recvBuf);
	}
	if (ProcNum > 1)
		MPI_Reduce(&localMax, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

	if (ProcRank == 0)
	{
		endTime = MPI_Wtime();
		printf("\nMax element - %d\nTime spent - %.6f", max, endTime - startTime);
	}
	MPI_Finalize();
	return 0;
}