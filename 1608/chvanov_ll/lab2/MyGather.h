#ifndef __MYGATHER_H__
#define __MYGATHER_H__

#include <mpi.h>

int findFirstSet(int n);
int getSubTreeSize(int procRank, int procNum);

int MyGather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
	void *recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

#endif //__MYGATHER_H__
