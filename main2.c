#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>

typedef struct sort_a 
{
    int* array_a;
    int* tmp;
    int n;
    int m;
} sort_a;

typedef struct merge_a 
{
    int size_min;
    int size_max;
    int* min;
    int* max;
    int* t;
    int* tmp;
    int m;
} merge_a;

int cmp (const void * a, const void * b) 
{
  return ( *(int*)a - *(int*)b );
}

void merge(merge_a* a) 
{
    int i = 0;
    int g = 0;
    int result = 0;

    while (i < a->size_min && g < a->size_max) 
    {
        if (a->min[i] < a->max[g]) 
        {
            a->tmp[result] = a->min[i];
            ++i;
        } else 
        {
            a->tmp[result] = a->max[g];
            ++g;
        }
        ++result;
    }

    while (i < a->size_min) 
    {
        a->tmp[result] = a->min[i];
        ++i;
        ++result;
    }
    while(g < a->size_max) 
    {
        a->tmp[result] = a->max[g];
        ++g;
        ++result;
    }
    if (a->tmp != a->t) 
    {
        memcpy(a->t, a->tmp, (a->size_min + a->size_max) * sizeof(int));
    }
}

void merge_sort(sort_a* a) 
{
    if (a->n <= a->m) 
    {
        qsort(a->array_a, a->n, sizeof(int), cmp);
    } else 
    {
        int middle = a->n / 2;
        sort_a min_a = 
        {
            .array_a = a->array_a,
            .tmp = a->tmp,
            .n = middle,
            .m = a->m,
        };
        sort_a max_a = 
        {
            .array_a = a->array_a + middle,
            .tmp = a->tmp + middle,
            .n = a->n - middle,
            .m = a->m,
        };
        merge_sort(&min_a);
        merge_sort(&max_a);
        merge_a merge_a = 
        {
            .size_min = middle,
            .size_max = a->n - middle,
            .min = a->array_a,
            .max = a->array_a + middle,
            .t = a->array_a,
            .tmp = a->tmp,
            .m = a->m,
        };
        merge(&merge_a);
    }
}

int bin(int* array_a, int min, int max, int res) 
{
    int middle = min + (max - min) / 2;
    if (max - min == 0) 
        return max;
     else if (array_a[middle] == res) 
        return middle;
     else if (array_a[middle] > res) 
           return bin(array_a, min, middle, res);
     else 
        return bin(array_a, middle + 1, max, res);
}

void merge_par(merge_a* a) 
{
    if (a->size_min < a->m || a->size_max < a->m) {
        merge(a);
    } else {
        int min_res = a->min[a->size_min / 2];
        int max_pos = bin(a->max, 0, a->size_max, min_res);

        merge_a min_a = {
            .size_min = a->size_min / 2 ,
            .size_max = max_pos,
            .min = a->min,
            .max = a->max,
            .t = a->t,
            .tmp = a->tmp,
            .m = a->m,
        };
        merge_a max_a = {
            .size_min = a->size_min - a->size_min / 2,
            .size_max = a->size_max - max_pos,
            .min = a->min + a->size_min / 2,
            .max = a->max + max_pos,
            .t = a->t + a->size_min / 2 + max_pos,
            .tmp = a->tmp + a->size_min / 2 + max_pos,
            .m = a->m,
        };
        #pragma omp parallel sections
        {
            #pragma omp section
            merge_par(&min_a);

            #pragma omp section
            merge_par(&max_a);
        }
    }
}

void merge_par_sort(sort_a* a) 
{
    if (a->n <= a->m) 
    {
        qsort(a->array_a, a->n, sizeof(int), cmp);
    } else {
        int middle = a->n / 2;
        sort_a min_a = 
        {
            .array_a = a->array_a,
            .tmp = a->tmp,
            .n = middle,
            .m = a->m,
       };
        sort_a max_a =
        {
            .array_a = a->array_a + middle,
            .tmp = a->tmp + middle,
            .n = a->n - middle,
            .m = a->m,
        };
        #pragma omp parallel sections
        {
            #pragma omp section
            merge_par_sort(&min_a);

            #pragma omp section
            merge_par_sort(&max_a);
        }
        merge_a merge_a = 
        {
            .size_min = middle,
            .size_max = a->n - middle,
            .min = a->array_a,
            .max = a->array_a + middle,
            .t = a->tmp,
            .tmp = a->tmp,
            .m = a->m,
        };
        merge_par(&merge_a);
        memcpy(a->array_a, merge_a.t, sizeof(int) * a->n);
    }
}

int main(int argc, char** argv) {
    int n; // количество чисел в сортируемом массиве
    int p; // количество потоков
    int m; // максимальный размер чанка

    // считывание данных и выделение памяти
    if (argc != 4)
     {
        printf("Error");
        return 0;
    }
    
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    p = atoi(argv[3]);
    omp_set_num_threads(p);

    int* array = (int*) malloc(n * sizeof(int));
    int* tmp = (int*) malloc(n * sizeof(int));
    FILE* data = fopen("data.txt", "w");
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        array[i] = rand();
        fprintf(data, "%d ", array[i]);
    }
    fprintf(data, "\n");

    sort_a a = 
    {
        .array_a = array,
        .tmp = tmp,
        .n = n,
        .m = m,
    };

    double start_time = omp_get_wtime();
    if (p == 1) {
        merge_sort(&a);
    } else {
        merge_par_sort(&a);
    }
    double end_time = omp_get_wtime();
    double work_time = end_time - start_time;
    FILE* st = fopen("st.txt", "w");
    fprintf(st, "%.5fs %d %d %d\n", work_time, n, m, p);
    fclose(st);

    for (int i = 0; i < n; i++) {
        fprintf(data, "%d ", a.array_a[i]);
    }
    fprintf(data, "\n");
    fclose(data);

    free(array);
    free(tmp);
    return 0;
}