#include "stdafx.h"
#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

void printMatrix(vector<vector<int>> a)
{
	int size = a[0].size();
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
			cout << a[i][j] << "  ";
		cout << endl;
	}
}

void sumMatrix(vector<vector<int>> a)
{
	int size = a[0].size();
	int sum = 0;
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j)
			sum += a[i][j];
	cout << "Matrix sum = " << sum << endl;
}

bool checkMatrix(vector<vector<int>> a, vector<vector<int>> b)
{
	int size = a[0].size();
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j)
			if (a[i][j] != b[i][j]) return false;
	return true;
}

void serialMatrixMul(vector<vector<int>> a, vector<vector<int>> b)
{
	int size = a[0].size();
	vector<vector<int>> c(size, vector<int>(size, 0));
	double t = omp_get_wtime();
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j)
			for (int k = 0; k < size; ++k)
			{
				c[i][j] += a[i][k] * b[k][j];
			}
	t = omp_get_wtime() - t;
	cout << "Serial algorithm time =  " << t << "sec." << endl;
	//sumMatrix(c);
}

void forDirectiveMatrixMul(vector<vector<int>> a, vector<vector<int>> b)
{
	int size = a[0].size();
	vector<vector<int>> c(size, vector<int>(size, 0));
	double t = omp_get_wtime();
#pragma omp parallel for num_threads(4)
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j)
			for (int k = 0; k < size; ++k)
			{
				c[i][j] += a[i][k] * b[k][j];
			}

	t = omp_get_wtime() - t;
	cout << "For directive time = " << t << "sec." << endl;
	//sumMatrix(c);
}

void linearMatrixMul(vector<vector<int>> a, vector<vector<int>> b)
{
	int size = a[0].size();
	vector<vector<int>> c(size, vector<int>(size, 0));
	double t = omp_get_wtime();
#pragma omp parallel num_threads(4)
	{
		int threadCount = omp_get_num_threads();
		int kol = size / threadCount;
		int cur = omp_get_thread_num();

		for (int i = kol * cur; i < kol * (cur + 1); ++i) {
			for (int j = 0; j < size; ++j) {
				for (int k = 0; k < size; ++k) {
					c[i][j] += a[i][k] * b[k][j];
				}
			}
		}
	}

	t = omp_get_wtime() - t;
	cout << "Linear algorithm time = " << t << "sec." << endl;
	//sumMatrix(c);
}

void blockyMatrixMul(vector<vector<int>> a, vector<vector<int>> b)
{
	int size = a[0].size();
	vector<vector<int>> c(size, vector<int>(size, 0));
	double t = omp_get_wtime();
#pragma omp parallel num_threads(4)
	{
		int threadCount = omp_get_num_threads();
		int blockSize = size / threadCount;
		int cur = omp_get_thread_num();
		for (int k = 0; k < threadCount; ++k)
		{
			for (int i = 0; i < blockSize; ++i)
			{
				for (int j = 0; j < blockSize; ++j)
				{
					for (int m = 0; m < size; ++m) {
						c[((cur + k) % threadCount) * blockSize + i][cur * blockSize + j] += a[((cur + k) % threadCount) * blockSize + i][m] * b[m][cur * blockSize + j];
					}
				}
			}

		}
	}
	t = omp_get_wtime() - t;
	cout << "Blocky algorithm time = " << t << "sec." << endl;
	//sumMatrix(c);
}

void printVector(vector<double> v) {
	int size = v.size();
	for (int i = 0; i < size; i++)
		cout << v[i] << " ";
	cout << endl;
}

bool checkResult(vector<vector<double>> a, vector<double> b, vector<double> x, double eps) {
	int size = a[0].size();
	vector<double> realB(size, 0.0);
	for (int i = 0; i < size; ++i)
		for (int j = 0; j < size; ++j)
			realB[i] += a[i][j] * x[j];

	for (int i = 0; i < size; ++i)
	{
		if (fabs(b[i] - realB[i]) >= eps) return false;
	}
	return true;
}

vector<double> Jacobi(vector<vector<double>> a, vector<double> b, vector<double> x, double eps)
{
	int size = a[0].size();
	vector<double> tempX(size);
	double norm;
	double t = omp_get_wtime();
	do {
		for (int i = 0; i < size; i++) {
			tempX[i] = b[i];
			for (int j = 0; j < size; ++j) {
				if (i != j)
					tempX[i] -= a[i][j] * x[j];
			}
			tempX[i] /= a[i][i];
		}
		norm = fabs(x[0] - tempX[0]);
		for (int j = 1; j < size; ++j) {
			if (fabs(x[j] - tempX[j]) > norm)
				norm = fabs(x[j] - tempX[j]);
			x[j] = tempX[j];
		}
	} while (norm >= eps);
	cout << "Serial Jacobi's time =  " << omp_get_wtime() - t << "sec." << endl;
	return x;
}

vector<double> JacobiParallel(vector<vector<double>> a, vector<double> b, vector<double> x, double eps)
{
	int size = a[0].size();
	vector<double> tempX(size);
	double norm;
	double t = omp_get_wtime();
	do {
#pragma omp parallel num_threads(4)
		{
			int threadCount = omp_get_num_threads();
			int cur = omp_get_thread_num();
			int task = size / threadCount;
			for (int i = cur * task; i < (cur + 1) * task; i++) {
				tempX[i] = b[i];
				for (int j = 0; j < size; ++j) {
					if (i != j)
						tempX[i] -= a[i][j] * x[j];
				}
				tempX[i] /= a[i][i];
			}
		}
		norm = fabs(x[0] - tempX[0]);
		for (int j = 0; j < size; ++j) {
			if (fabs(x[j] - tempX[j]) > norm)
				norm = fabs(x[j] - tempX[j]);
			x[j] = tempX[j];
		}
	} while (norm > eps);
	cout << "Parallel Jacobi's time =  " << omp_get_wtime() - t << "sec." << endl;
	return tempX;
}

double function(double x)
{
	return (sin(x) + sqrt(x)) / sqrt(x);
}

double Simpson(double from, double to, double eps)
{
	double steps = 100.0;
	double result, oldResult;
	double step = (to - from) / steps;
	result = oldResult = 0.0;

	for (int i = 1; i <= steps; ++i)
		result += function(from + (i - 1) * step) + 4 * function(from + (i - 0.5) * step) + function(from + i * step);
	result *= (step / 6);

	while (fabs(result - oldResult) > eps)
	{
		oldResult = result;
		step = step / 2.0;
		steps = (to - from) / step;
		for (int i = 1; i <= steps; ++i)
			result += function(from + (i - 1) * step) + 4 * function(from + (i - 0.5) * step) + function(from + i * step);
		result *= (step / 6.0);
	}
	//cout << "Result is " << result << endl;
	return result;
}

void SimpsonParalell(double from, double to, double eps)
{
	double t = omp_get_wtime();
	double sum = 0.0;
#pragma omp parallel num_threads(4)
	{
		int threadCount = omp_get_num_threads();
		double kol = (to - from) / (double)threadCount;
		double cur = omp_get_thread_num();
		sum += Simpson(kol * cur + from, kol * (cur + 1) + from, eps);
	}
	cout << "Result is " << sum << endl;
	cout << "Parallel Simpson`s time = " << omp_get_wtime() - t << endl;
}

bool isSorted(vector<int> arr)
{
	int size = arr.size();
	for (int i = 1; i < size; ++i)
	{
		if (arr[i - 1] > arr[i]) return false;
	}
	return true;
}

vector<int> bubbleSort(vector<int> a)
{
	int size = a.size();
	double time = omp_get_wtime();
	for (int i = size; i > 1; i--)
	{
		for (int j = 1; j < i; j++)
		{
			if (a[j] < a[j - 1])
			{
				int t = a[j];
				a[j] = a[j - 1];
				a[j - 1] = t;
			}
		}
	}
	cout << "Serial Bubble Sort time: " << omp_get_wtime() - time << "sec." << endl;
	return a;
}

vector<int> bubbleSortParallel(vector<int> a) {
	int size = a.size();
	double time = omp_get_wtime();
	int bound = size / 2 - (((size % 2) == 0) ? 1 : 0);

	for (int i = 0; i < size; i++)
	{
		if (i % 2 == 0) {
#pragma omp parallel for num_threads(4)
			for (int j = 0; j < size / 2; j++)
				if (a[2 * j] > a[2 * j + 1]) {
					int temp = a[2 * j];
					a[j * 2] = a[2 * j + 1];
					a[2 * j + 1] = temp;
				}
		}
		else
#pragma omp parallel for num_threads(4)
			for (int j = 0; j < bound; j++)
				if (a[2 * j + 1] > a[2 * j + 2]) {
					int temp = a[2 * j + 1];
					a[2 * j + 1] = a[2 * j + 2];
					a[2 * j + 2] = temp;
				}
	}
	cout << "Parallel Bubble Sort time: " << omp_get_wtime() - time << "sec." << endl;
	return a;
}

vector<int> shellSort(vector<int> a)
{
	int size = a.size();
	double time = omp_get_wtime();
	int step = size / 2;
	while (step > 0) {
		for (int i = 0; i < size - step; i++) {
			int j = i;
			while (j >= 0 && a[j] > a[j + step]) {
				int temp = a[j];
				a[j] = a[j + step];
				a[j + step] = temp;
				j--;
			}
		}
		step /= 2;
	}
	cout << "Time Shell: " << omp_get_wtime() - time << "sec." << endl;
	return a;
}

vector<int> shellSortParallel(vector<int> a)
{
	int size = a.size();
	int step = size / 2;
	double time = omp_get_wtime();
	while (step >= 4) {
#pragma omp parallel for num_threads(4)
		for (int i = 0; i < (size - step); i++)
		{
			int j = i;
			while (j >= 0 && a[j] > a[j + step])
			{
#pragma omp critical 
				{
					int temp = a[j];
					a[j] = a[j + step];
					a[j + step] = temp;
					j--;
				}
			}
		}
		step /= 2;
	}
#pragma omp parallel for num_threads(2)
	for (int i = 0; i < (size - 2); i++)
	{
		int j = i;
		while (j >= 0 && a[j] > a[j + 2])
		{
#pragma omp critical 
			{
				int temp = a[j];
				a[j] = a[j + 2];
				a[j + 2] = temp;
				j--;
			}
		}
	}
	for (int i = 0; i < (size - 1); i++)
	{
		int j = i;
		while (j >= 0 && a[j] > a[j + 1])
		{
#pragma omp critical 
			{
				int temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = temp;
				j--;
			}
		}
	}
	cout << "Time Shell Parallel: " << omp_get_wtime() - time << "sec." << endl;
	return a;
}

vector<int> quickSort(vector<int> &arr, int left, int right)
{
	if (left < right) {
		double pivot = arr[left];
		int p = left;
		for (int i = left + 1; i < right; i++)
			if (arr[i] < pivot) {
				p++;
				int temp = arr[i];
				arr[i] = arr[p];
				arr[p] = temp;
			}
		int temp = arr[left];
		arr[left] = arr[p];
		arr[p] = temp;
		quickSort(arr, left, p);
		quickSort(arr, p + 1, right);
	}
	return arr;
}

vector<int> quickSort(vector<int> &arr)
{
	return quickSort(arr, 0, arr.size());
}

vector<int> quickSortParallel(vector<int> &arr, int left, int right)
{
	int leftP = left, rightP = right;
	int middle = arr[(leftP + rightP) / 2];
	do {
		while (arr[leftP] < middle)
			leftP++;
		while (arr[rightP] > middle)
			rightP--;

		if (leftP <= rightP) {
			int temp = arr[rightP];
			arr[rightP] = arr[leftP];
			arr[leftP] = temp;
			leftP++;
			rightP--;
		}
	} while (leftP < rightP);

#pragma omp parallel num_threads(2)
#pragma omp sections
	{
#pragma omp section
		{
			if (left < rightP)
				quickSortParallel(arr, left, rightP);
		}
#pragma omp section
		{
			if (leftP < right)
				quickSortParallel(arr, leftP, right);
		}

	}
	return arr;
}

vector<int> quickSortParallel(vector<int> &arr)
{
	return quickSortParallel(arr, 0, arr.size() - 1);
}

int main()
{
	double t;
	
	int size;
	cout << "Insert matrix size: ";
	cin >> size;

	vector<vector<int>> A(size, vector<int>(size, 1));
	vector<vector<int>> B(size, vector<int>(size, 2));
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) serialMatrixMul(A, B);
	cout << "Average serial algorithm time =  " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) forDirectiveMatrixMul(A, B);
	cout << "Average for directive time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) linearMatrixMul(A, B);
	cout << "Average linear algorithm time =  " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;

	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) blockyMatrixMul(A, B);
	cout << "Average blocky algorithm time =  " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;

	double eps;
	cout << "With eps = ";
	cin >> eps;

	vector<vector<double>> a(size, vector<double>(size, 0.0));
	vector<double> b(size, 0.0);
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < i; ++j) {
			if ((j % 2) == 0) a[i][j] = 1.0;
			else a[i][j] = -1.0;
		}
		a[i][i] = 10.0;
		if ((i % 2) == 0) b[i] = 10.0;
		else b[i] = 11.0;
	}
	vector<double> x(size, 0.0);
	x[0] = 1.0;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) Jacobi(a, b, x, eps);
	cout << "Average serial Jacobi's time =  " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (checkResult(a, b, Jacobi(a, b, x, eps), eps * 10)) cout << "Result is correct" << endl;
	else cout << "Result is wrong" << endl;

	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) JacobiParallel(a, b, x, eps);
	cout << "Average paralell Jacobi's time =  " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (checkResult(a, b, JacobiParallel(a, b, x, eps), eps * 10)) cout << "Result is correct" << endl;
	else cout << "Result is wrong" << endl;

	double from, to;
	cout << "From: ";
	cin >> from;
	cout << "To: ";
	cin >> to;
	cout << "With eps = ";
	cin >> eps;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) Simpson(from, to, eps);
	cout << "Average serial Simpson`s time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) SimpsonParalell(from, to, eps);
	cout << "Average paralell Simpson`s time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	
	cout << "Input array size: ";
	cin >> size;
	vector<int> arr(size);

	for (int i = 0; i < size; ++i) arr[i] = rand() % 666;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) bubbleSort(arr);
	cout << "Average Bubble Sort time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (isSorted(bubbleSort(arr))) cout << "Bubble Sort works correct" << endl;
	else cout << "Result is wrong" << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) bubbleSortParallel(arr);
	cout << "Average parallel Bubble Sort time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (isSorted(bubbleSortParallel(arr))) cout << "Parallel Bubble Sort works correct" << endl;
	else cout << "Result is wrong" << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) shellSort(arr);
	cout << "Average Shell Sort time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (isSorted(shellSort(arr))) cout << "Shell Sort works correct" << endl;
	else cout << "Result is wrong" << endl;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) shellSortParallel(arr);
	cout << "Average parallel Shell Sort time = " << (omp_get_wtime() - t) / 10.0 << "sec." << endl;
	if (isSorted(shellSortParallel(arr))) cout << "Parallel Shell Sort works correct" << endl;
	else cout << "Result is wrong" << endl;
	
	vector<int> test = arr;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) { quickSort(test); test = arr; }
	cout << "Average serial Quick Sort time = " << omp_get_wtime() - t << "sec." << endl;
	if (isSorted(quickSort(test))) cout << "Quick Sort works correct" << endl;
	else cout << "Result is wrong" << endl;

	test = arr;
	
	t = omp_get_wtime();
	for (int i = 0; i < 10; ++i) { quickSortParallel(test); test = arr; }
	cout << "Average parallel Quick Sort time = " << omp_get_wtime() - t << "sec." << endl;
	if (isSorted(quickSortParallel(test))) cout << "Parallel Quick Sort works correct" << endl;
	else cout << "Result is wrong" << endl;
	cout << endl;

	system("pause");
	return 0;
}