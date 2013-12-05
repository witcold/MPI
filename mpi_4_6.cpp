// 6. _ 
//    \|
#include "stdafx.h"
#include <mpi.h>
#include <iostream>
#include <math.h>;

const int Matrix_Width  = 9;
const int Matrix_Height = 6;

void PrintArray (int arr[Matrix_Height][Matrix_Width])
{
	std::cout<<'\n';
	for (int i = 0; i<Matrix_Height; i++)
	{
		for (int j = 0; j<Matrix_Width; j++)			
				std::cout<<arr[i][j]<<' ';
		std::cout<<'\n';
	}
}

int _tmain(int argc, char* argv[])
{
	MPI_Init(&argc,&argv);
	const int count = 2*Matrix_Height-2;

	double step = Matrix_Width*(1.0/Matrix_Height);
	// Массив, содержащий число элементов базового типа в каждом блоке.
	int blocklengths[count];
	// Массив смещений каждого блока от начала матрицы.
	int displasments[count];

	// Заполяем для первой строки
	blocklengths[0] = Matrix_Width;
	displasments[0] = 0;
	// Заполняем для всех, кроме последней
	for (int i = 1; i < Matrix_Height-1; i++)
	{
		//диагональ
		blocklengths[2*i-1] = floor(step+0.5);
		displasments[2*i-1] = i*Matrix_Width + i*step;
		
		//последний столбец
		blocklengths[2*i] = 1;
		displasments[2*i] = (i+1)*Matrix_Width - 1;
	}
	// Заполняем последнюю строку
	blocklengths[count-1] = floor(step+0.5);
	displasments[count-1] = Matrix_Width*(Matrix_Height)-step;

	MPI_Datatype LTriMatrix;
	MPI_Type_indexed(count,blocklengths,displasments,MPI_INT,&LTriMatrix);
	MPI_Type_commit(&LTriMatrix);
	
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	int arr[Matrix_Height][Matrix_Width];
	for (int i = 0; i<Matrix_Height; i++)
		for (int j=0; j<Matrix_Width; j++)
			arr[i][j] = rank+1;

	if (rank==0)
	{
		PrintArray(arr);
		MPI_Send(&arr,1,LTriMatrix,1,0,MPI_COMM_WORLD);
	}
	else
		if (rank==1)
	{
		MPI_Recv(&arr,1,LTriMatrix,0,0,MPI_COMM_WORLD,0);
		PrintArray(arr);
	}
	MPI_Type_free(&LTriMatrix);
	MPI_Finalize();
	return 0;
}
