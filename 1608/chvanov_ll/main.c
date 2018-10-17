#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "mpi.h"

#define ARRAY_LENGTH 20

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
	int max = INT_MIN, i;
	MPI_Status Status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	int step = ARRAY_LENGTH / ProcNum;			

	if (ProcRank == 0) {
		srand(time(NULL));
		int* arr = generateRandomArray(ARRAY_LENGTH);
		for (i = 0; i < ARRAY_LENGTH; i++)
			printf("%6d", arr[i]);

		for (i = 1; i < ProcNum - 1; i++)
			MPI_Send(arr + step * i, step, MPI_INT, i, 0, MPI_COMM_WORLD); 
		if (ProcNum > 1)
			MPI_Send(arr + step * i, ARRAY_LENGTH - step * (ProcNum - 1), MPI_INT, i, 0, MPI_COMM_WORLD);

		max = findMaxInArray(arr, step);

		int recvMax;
		for (i = 1; i < ProcNum; i++) {
			MPI_Recv(&recvMax, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			if (recvMax > max)
				max = recvMax;
		}

		free(arr);

		printf("\nMax element - %d", max);
	}
	else
	{
		int localSize = (ProcRank != (ProcNum - 1)) ? step : (ARRAY_LENGTH - step * (ProcNum - 1));
		int* recvBuf = malloc(localSize * sizeof(int));
		MPI_Recv(recvBuf, localSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
		int localMax = findMaxInArray(recvBuf, localSize);
		free(recvBuf);

		MPI_Send(&localMax, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}