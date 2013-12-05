// 11. Найти сумму отрицательных (0, если нет отрицательных).
#include "stdafx.h"
#include <mpi.h>
#include <random>
#include <iostream>
#include <sstream>
#include <string>

// Размерность массива
const int n = 10;

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

void GetNegSum(int *in, int *inout, int *len, MPI_Datatype *datatypes)
{
	for (int i = 0; i < *len; i++)
	{
		if (inout[i] > 0)
			inout[i] = 0;
		if (in[i] < 0)
			inout[i] += in[i];
	}
}

int _tmain(int argc, char* argv[])
{
	int rank = 0, processCount = 0;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD,&processCount);

	if (processCount > 1)
	{
		int a[n];
		int sum[n];
		srand((unsigned int)a);
		
		std::string str = "Process " + intToString(rank) + " generated elements:\n";
		// Заполнение массива и его вывод на экран.
		for (int i = 0; i<n; i++)
		{
			a[i] = rand()%21-10;
			sum[i] = 0;
			str += intToString(a[i]) + "\t";
		}
		printf("%s",str.c_str());

		MPI_Op operation;
		MPI_Op_create((MPI_User_function*)GetNegSum,1,&operation);
		MPI_Reduce(&a,&sum,n,MPI_INT,operation,0,MPI_COMM_WORLD);
		MPI_Op_free(&operation);

		if (rank==0)
		{
			std::string resultString = "Result:\n";
			for ( int i = 0; i < n; i++)
				resultString += intToString(sum[i]) + "\t";
			printf("%s\n",resultString.c_str());
		}
	}

	MPI_Finalize();
	return 0;
}
