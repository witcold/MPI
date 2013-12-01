// 9. Найти объединения множеств Xi и Xi+1.
#include "stdafx.h"
#include "mpi.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string>
#include <sstream>

#define ARRAY_TAG 1
#define RANK_TAG 2

const int m = 10;

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

//Проверяет, содержит ли <Array> элемент <element> в первых <asz> элементах
bool Contains(int *Array, int asz, int element)
{
	int i = 0;
	while (i < asz && Array[i] != element)
		i++;
	return i<asz;
}

//Заполняет массив <x> <size> элементами
void GetArray (int *x, int asz, int seed)
{
	srand(seed);
	const int maxValue = 20;
	for (int i = 0; i<asz; i++)
	{
		int t = rand() % maxValue;
		while (Contains(x,i,t))
		{
			t = (t+1)%maxValue;
		}
		x[i] = t;
	}
}

int _tmain(int argc, char* argv[])
{
	//Инициализация MPI
	MPI_Init(&argc, &argv);
	//Получаем количество процессов
	int processCount;
	MPI_Comm_size(MPI_COMM_WORLD, &processCount);
	if (processCount % 2 == 0)
	{
		//Получаем ранг текущего процесса
		int rank = 0;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		//Заполняем массив
		int *procVector = new int[m];
		GetArray(procVector, m, time(0)+rank);

		//Если процесс - четный
		if (rank % 2 == 0)
		{
			std::string strGenerated = "Process " + intToString(rank) + " generated elements: ";
			for (int i = 0; i < m; i++)
				strGenerated += intToString(procVector[i]) + " ";
			strGenerated += "\n";
			//Максимальное количество элементов объединения - 2m
			int *result = new int[2*m];
			//Текущее количество элементов объединения - m
			int count = m;
			//Инициализируем буфер данных
			int buffer[m] = {0};
			//Получаем m элементов в буфер от нечетного процесса
			MPI_Status mpistat;
			MPI_Recv(result,m,MPI_INT,rank+1,ARRAY_TAG,MPI_COMM_WORLD,&mpistat);
			std::string strRecv = "Received from " + intToString(rank+1) + " process: ";
			for (int i = 0; i < m; i++)
				strRecv += intToString(result[i]) + " ";
			strRecv += "\n";
			for (int j = 0; j < m; j++)
			{
				if (!Contains(result,count,procVector[j]))
				{
					result[count] = procVector[j];
					count++;
				}
			}		
			std::string strOut = "Union of sets: ";
			for (int i = 0; i < count; i++)
				strOut += intToString(result[i]) + " ";
			strOut += "\n\n";
			printf("%s%s%s",strGenerated.c_str(),strRecv.c_str(),strOut.c_str());	
		}
		else
		{
			MPI_Send(procVector,m,MPI_INT,rank-1,ARRAY_TAG, MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();
	return 0;
}
