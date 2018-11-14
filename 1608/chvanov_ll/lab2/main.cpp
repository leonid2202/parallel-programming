#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "MyGather.h"

#define TYPE double
#define MPITYPE MPI_DOUBLE
#define ARR_SIZE 100000
#define ROOT_PROC 5

TYPE* generateRandomArray(int size)
{
	TYPE* arr = nullptr;
	if (size > 0)
	{
		arr = new TYPE[size];
		for (int i = 0; i < size; i++)
			arr[i] = (TYPE)rand();
	}
	return arr;
}

using namespace std;

int main(int argc, char *argv[])
{
	TYPE* myBuff = nullptr, *MPIBuff = nullptr;
	double startTime, myTime, MPITime;
	int procRank, procNum;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);

	srand(time(NULL) + procRank);

	if (procRank == ROOT_PROC)
	{
		myBuff  = new TYPE[procNum*ARR_SIZE];
		MPIBuff = new TYPE[procNum*ARR_SIZE];
	}

	TYPE* localArr = generateRandomArray(ARR_SIZE);
	startTime = MPI_Wtime();
	MPI_Gather(localArr, ARR_SIZE, MPITYPE, MPIBuff, ARR_SIZE, MPITYPE, ROOT_PROC, MPI_COMM_WORLD);
	MPITime = MPI_Wtime() - startTime;

	startTime = MPI_Wtime();
	MyGather(localArr, ARR_SIZE, MPITYPE, myBuff, ARR_SIZE, MPITYPE, ROOT_PROC, MPI_COMM_WORLD);
	myTime = MPI_Wtime() - startTime;


	if (procRank == ROOT_PROC)
	{
		cout << "MPI_Gather time:" << MPITime << endl;
		cout << "MyGather   time:" << myTime << endl;

		//cout << endl << "result:";
		//for (int i = 0; i < procNum*ARR_SIZE; i++)
		//{
		//	cout << (TYPE)myBuff[i] << ' ';
		//}
		//cout << endl;

		//for (int i = 0; i < procNum * ARR_SIZE; ++i)
		//	if (myBuff[i] != MPIBuff[i])
		//		cout << "error in " << i << endl;
	}

	MPI_Finalize();
	return 0;
}
