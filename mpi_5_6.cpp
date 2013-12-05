// 6. res_i = max_j max_k Aik * Bki
#include "stdafx.h"
#include <mpi.h>
#include <random>
#include <sstream>
#include <string>

// Количество измерений для топологии кольцо
#define Dimensions_Count 1

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();
    return s;
}

int Calculate(int *a, int *b, int n)
{
	int max = -32767;
	for (int k = 0; k < n; k++)
		if ((a[k] * b[k]) > max) 
			max = a[k] * b[k];
	return max;
}

int _tmain(int argc, char* argv[])
{
	int rank;
	int process_count;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	// Массив значений, указывающих количество процессов в каждом измерении
	int *dims = new int[Dimensions_Count];
	// Массив значений, указывающих периодичность в каждом измерении
	int *periods = new int[Dimensions_Count];

	for (int i = 0; i < Dimensions_Count; i++)
	{
		dims[i] = 0;
		periods[i] = true;
	}
	// Позволяет выбрать сбалансированное распределение процессов по направлению координат, в зависимости от числа процессов в группе
	MPI_Dims_create(process_count, Dimensions_Count, dims);
	MPI_Comm RingComm;
	// Декартовая структура произвольного измерения
	MPI_Cart_create(MPI_COMM_WORLD, Dimensions_Count, dims, periods, false, &RingComm);
	int predRank, nextRank;
	// Возвращает информацию для входных спецификаций, требуемых в вызове MPI_Sendrecv
	MPI_Cart_shift(RingComm, 0, 1, &predRank, &nextRank);
#pragma region Генерация_матриц	
	int *A = new int[process_count];
	int *B = new int[process_count];
	std::string strA;

	srand((int)A + (int)B);
	for (int i = 0; i<process_count; i++)
	{
		A[i] = rand() % (process_count * 1) - process_count;
		B[i] = rand() % (process_count * 1) - process_count;
		strA += ((A[i] >= 0)? "  " : " ") + intToString(A[i]) + ' ';
	}
#pragma endregion
#pragma region Печать_матриц
	if (rank == 0)
	{
		printf("A:\n%s\n", strA.c_str());
		MPI_Send(0, 0, MPI_INT, nextRank, 0, RingComm);
		MPI_Recv(0, 0, MPI_INT, predRank, 0, RingComm, 0);
	}
	else
	{
		MPI_Recv(0, 0, MPI_INT, predRank, 0, RingComm, 0);
		printf("%s\n", strA.c_str());
		MPI_Send(0, 0, MPI_INT, nextRank, 0, RingComm);
	}
	if (rank == 0)
		printf("\nB:");
	if (rank == process_count - 1)
		MPI_Send(0, 0, MPI_INT, nextRank, 0, RingComm);

	for (int i = 0; i<process_count; i++)
	{
		MPI_Recv(0, 0, MPI_INT, predRank, 0, RingComm, 0);
		if (rank == 0)
			printf("\n");
		printf("%3d ", B[i]);
		MPI_Send(0, 0, MPI_INT, nextRank, 0, RingComm);
	}
	if (rank == 0)
	{
		MPI_Recv(0, 0, MPI_INT, predRank, 0, RingComm, 0);
		printf("\n");
	}
#pragma endregion
	int max = Calculate(A, B, process_count);
	MPI_Status stat;
	for (int i = 1; i<process_count; i++)
	{
		MPI_Sendrecv_replace(B, process_count, MPI_INT, nextRank, 2, predRank, 2, RingComm, &stat);
		int tmp = Calculate(A, B, process_count);
		if (tmp > max)
			max = tmp;
	}
	int *results = new int[process_count];
	MPI_Gather(&max, 1, MPI_INT, results, 1, MPI_INT, 0, RingComm);

	if (rank == 0)
	for (int i = 0; i < process_count; i++)
	{
		printf("Res_%d =%4d\n", i, results[i]);
	}
	MPI_Finalize();
	return 0;

}
