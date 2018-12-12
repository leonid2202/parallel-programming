#ifndef __SHELLSORT_H__
#define __SHELLSORT_H__

#define TYPE char
#define MPI_TYPE MPI_CHAR

void shellSort(TYPE* arr, int n)
{
	int step;
	double tmp;
	for (step = n/2; step > 0; step /=2)
		for (int i = step; i < n; ++i)
			for (int j = i - step; (j >= 0) && (arr[j] > arr[j + step]); j -= step)
			{
				tmp = arr[j];
				arr[j] = arr[j + step];
				arr[j + step] = tmp;
			}
}

void merge(TYPE* arr1, int n1, TYPE* arr2, int n2, TYPE* res)
{
	int i, j, k;
	i = 0; j = 0; k = 0;
	while ((i < n1) && (j < n2))
		if (arr1[i]<arr2[j])
		{
			res[k] = arr1[i];
			i++; k++;
		}
		else
		{
			res[k] = arr2[j];
			j++; k++;
		}
	if (i == n1)
		while (j<n2)
		{
			res[k] = arr2[j];
			j++; k++;
		}
	else
		while (i<n1)
		{
			res[k] = arr1[i];
			i++; k++;
		}
}

#endif // __SHELLSORT_H__