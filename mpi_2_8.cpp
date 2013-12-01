// 8. Посчитать значения ряда Тейлора для функции sqrt(1+x).
#include "stdafx.h"
#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>

#define MPI_ROOT_PROCESS 0

const double R = 1.0;

double GetFunctionValue(double x)
{
	return sqrt(1+x);
}

double GetApproxValue(double x, double eps)
{
	double result = 1.0;
	double term = 1.0 / 2.0 * x;
	for (int i = 1; abs(term) > eps; i++)
	{
		result += term;
		term *= (-1.0) * x * (2.0*i - 1.0) / (2.0*(i+1.0));
	}
	return result;
}

int _tmain(int argc, char* argv[])
{
	MPI_Init(&argc,&argv);
	//Получаем число процессов
	int procCount;
	MPI_Comm_size(MPI_COMM_WORLD,&procCount);
	//Получаем ранг процесса
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	//Если процесс главный
	if (rank == MPI_ROOT_PROCESS)
	{
		//Запрашиваем А и B
		double A,B;
		do
		{
			std::cout<<"ENTER A B\n";
			std::cin>>A>>B;
		}
		while (A>=B && ((abs(A)>=R) || (abs(B)>=R)));
		//Запрашиваем эпсилон
		double eps;
		do
		{
			std::cout<<"ENTER epsylon\n";
			std::cin>>eps;
		}
		while (eps<=0 || eps>=1);
		//Запрашиваем количество точек
		int N;
		do
		{
			std::cout<<"ENTER count of points\n";
			std::cin>>N;
		}
		while (N<2);
		//Создаём массив точек, для которых будем считать функцию
		double *pointVector = new double[N];
		double step = (B-A)/(N-1);
		double cv = A;
		for (int i=1;i<N;i++)
		{
			pointVector[i-1] = cv;
			cv += step;
		}
		//Последняя точка может не совпадать с B, поэтому добавляем её отдельно
		pointVector[N-1] = B;

		//Теперь начинаем рассылку сообщений
		//Отправляем эпсилон
		MPI_Bcast(&eps,1,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);

		double *accValues = new double[N]; //точные значения функции
		double *approxValues = new double[N]; // приближённые значения

		int sendCount = N / procCount;
		//Если количество процессов кратно числу точек, то всё хорошо
		if (N % procCount == 0)
		{ 
			//Отправляем процессам сообщение с количеством точек, для которых необходимо рассчитать значение функции
			MPI_Bcast(&sendCount,1,MPI_INT,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Отправляем сами точки
			MPI_Scatter(pointVector,sendCount,MPI_DOUBLE,pointVector,sendCount,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Считаем функцию в своих точках
			for (int i = 0; i<sendCount; i++)
			{
				accValues[i]=GetFunctionValue(pointVector[i]);
				approxValues[i] = GetApproxValue(pointVector[i],eps);
			}

			//Теперь принимаем данные
			//Точные значения
			MPI_Gather(accValues,sendCount,MPI_DOUBLE,accValues,sendCount,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Затем приближённые
			MPI_Gather(approxValues,sendCount,MPI_DOUBLE,approxValues,sendCount,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		}
		//Иначе надо определить сколько кому отправлять
		else
		{
			//Отправляем сообщение другим процессам готовиться к векторному приёму/передаче
			{
				int tmp = -1;
				MPI_Bcast(&tmp,1,MPI_INT,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			}
			//Формируем массив смещений и массив с числом элементов (смещения считаются в элементах от начала массива)
			int *sendArray = new int[procCount];
			int *offsetArray = new int[procCount];
			for (int i = 0; i<procCount; i++)
			{
				sendArray[i] = sendCount;
			}
			for (int i = 0; i<N%procCount; i++)
				sendArray[i]++;
			offsetArray[0]=0;
			for (int i=1; i<procCount; i++)
			{
				offsetArray[i]= sendArray[i-1]+offsetArray[i-1];
			}
			//Отправляем количество элементов для получения
			MPI_Scatter(sendArray,1,MPI_INT,&sendArray[0],1,MPI_INT,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Отправляем данные
			MPI_Scatterv(pointVector,sendArray,offsetArray,MPI_DOUBLE,pointVector,sendArray[0],MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			for (int i = 0;i<sendArray[0];i++)
			{
				accValues[i]=GetFunctionValue(pointVector[i]);
				approxValues[i] = GetApproxValue(pointVector[i],eps);
			}
			//Теперь принимаем данные
			//Точные
			MPI_Gatherv(accValues,sendArray[0],MPI_DOUBLE,accValues,sendArray,offsetArray,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//затем приближённые
			MPI_Gatherv(approxValues,sendArray[0],MPI_DOUBLE,approxValues,sendArray,offsetArray,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		}
		//Выводим значения (точность - 15 знаков, выводить числа без отбрасывания нулей, показывать знак для положительных)
		std::cout.precision(15);
		std::cout.setf(std::ios::fixed | std::ios::showpos);
		std::cout<<"point              Accurate value     Approximate value\n";
		for (int i = 0; i<N;i++)
			std::cout<<pointVector[i]<<' '<<accValues[i]<<' '<<approxValues[i]<<'\n';
	}
	else
	{
		//Получаем эпсилон
		double eps;
		MPI_Bcast(&eps,1,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		//Получаем количество точек
		int pointCount;
		MPI_Bcast(&pointCount,1,MPI_INT,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		//Пришло положительное, значит число точек для всех процессов одинаковое и мы его получили
		if (pointCount>0)
		{
			double *points = new double[pointCount];
			//Получили точки
			MPI_Scatter(0,0,MPI_DOUBLE,points,pointCount,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Считаем функцию
			double *accValues = new double[pointCount];
			double *approxValues = new double[pointCount];
			for (int i = 0; i<pointCount; i++)
			{
				accValues[i] = GetFunctionValue(points[i]);
				approxValues[i] = GetApproxValue(points[i],eps);
			}
			//Отправляем
			//Точные значения
			MPI_Gather(accValues,pointCount,MPI_DOUBLE,0,0,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Затем приближённые
			MPI_Gather(approxValues,pointCount,MPI_DOUBLE,0,0,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		}
		else
		//Если пришло отрицательное значение, значит число точек будет получено через scatter
		{
			//Получаем число точек
			MPI_Scatter(0,0,MPI_DOUBLE,&pointCount,1,MPI_INT,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Получаем сами точки
			double *points = new double[pointCount];
			MPI_Scatterv(0,0,0,MPI_DOUBLE,points,pointCount,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Считаем функцию
			double *accValues = new double[pointCount];
			double *approxValues = new double[pointCount];
			for (int i = 0; i<pointCount; i++)
			{
				accValues[i]=GetFunctionValue(points[i]);
				approxValues[i] = GetApproxValue(points[i],eps);
			}
			//Отправляем
			//Точные значения
			MPI_Gatherv(accValues,pointCount,MPI_DOUBLE,0,0,0,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
			//Затем приближённые
			MPI_Gatherv(approxValues,pointCount,MPI_DOUBLE,0,0,0,MPI_DOUBLE,MPI_ROOT_PROCESS,MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();
	return 0;
}
