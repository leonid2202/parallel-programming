#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "MyGather.h"

inline int findFirstSet(int n)
{
	if (!n)
		return 0;

	int i;
	for (i = 0; !(n & 1); ++i)
		n >>= 1;
	return i;
}
int getSubTreeSize(int procRank, int procNum)
{
	int ffs = findFirstSet(procRank);
	if (!procRank)
		return procNum;
	else
	{
		int res = (1 << ffs);
		if (procNum - procRank < res)
			return procNum - procRank;
		else
			return res;
	}
}
inline int changeRank(int procRank, int root, int procNum)
{
	return (procRank - root < 0) ? (procRank - root + procNum) : (procRank - root);
}


int MyGather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
	void *recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int procNum, procRank, pReceiver;
	int err = MPI_SUCCESS;
	char* buff = nullptr;
	MPI_Aint extent, lb;
	MPI_Status status;

	MPI_Comm_rank(comm, &procRank);
	MPI_Comm_size(comm, &procNum);
	MPI_Type_get_extent(recvtype, &lb, &extent);

	procRank = changeRank(procRank, root, procNum);

	int subTreeSize = getSubTreeSize(procRank, procNum);
	if (subTreeSize != 1 || procNum == 1)
	{
		buff = new char[subTreeSize*sendcount*extent];
		memcpy(buff, sendbuf, sendcount*extent);
	}

	int pSender, recCount;
	char* recBuff;
	for (int i = 1; ((procRank | i) > procRank) && ((procRank | i) < procNum); i <<= 1)
	{
		pSender = procRank | i;
		recBuff = buff + i*sendcount*extent;
		recCount = getSubTreeSize(pSender, procNum) * sendcount;
		MPI_Recv(recBuff, recCount, recvtype, (pSender + root) % procNum, 0, comm, &status);
	}

	if (procRank)
	{
		pReceiver = procRank - (1 << findFirstSet(procRank));
		if (subTreeSize != 1)
			MPI_Send(buff, subTreeSize*sendcount, sendtype, (pReceiver + root) % procNum, 0, comm);
		else
			MPI_Send(sendbuf, sendcount, sendtype, (pReceiver + root) % procNum, 0, comm);
	}
	else
	{
		memcpy((char*)recvbuf + root* recvcount * extent, buff, (procNum - root)* recvcount * extent);
		memcpy(recvbuf, buff + (procNum - root)* recvcount * extent, root * recvcount * extent);
	}

	MPI_Barrier(comm);
	return err;
}
