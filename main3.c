#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>

struct sort_arg 
{
    int* array;
    int i;
    int n;
    int m;
};

struct only_merge_a 
{
    int* array;
    int min_1;
    int min_2;
    int min_3;
    int max_1;
    int max_2;
    int* res;
};

struct merge_a 
{
    int* array;
    int* res;
    int n;
    pthread_t* threads;
    int i;
    int g;
    int p;
    int thread_id;
    struct only_merge_a* only_merge_a;
};

int cmp(const void* x, const void* y) 
{
    return (*(int*)x - *(int*)y);
}

void swap(int *a, int *b)
 {
    int temp = *b;
    *b = *a;
    *a = temp;
}

int bin(int* array, int key, int min, int max)
 {
    max++;
    while (min < max) 
    {
        int middle = (min + max) / 2;
        if (key > array[middle]) 
        {
            min = middle + 1;
        }
        else {
            max = middle;
        }
    }
    return max;
}

void* single_f_chipping(void* args) 
{
    struct sort_arg* arg = args;
    qsort(arg->array + arg->i, arg->i + arg->m <= arg->n ? (size_t)arg->m : (size_t)(arg->n - arg->i),
          sizeof(int), cmp);
}

void chipping(int* array, int n, int m, pthread_t* threads, int p) 
{
    int counter_threads = 0;

    struct sort_arg* args = (struct sort_arg*)malloc(sizeof(struct sort_arg) * p);
    for (int i = 0; i < n; )
     {
        for (int g = 0; g < p; g++) 
        {
            if (i >= n)
             {
                break;
            }

            struct sort_arg arg;
            arg.array = array;
            arg.i = i;
            arg.n = n;
            arg.m = m;
            args[g] = arg;
            pthread_create(threads + g, NULL, single_f_chipping, args + g);
            counter_threads++;
            i += m;
        }
        for (int k = 0; k < counter_threads; k++) 
        {
            pthread_join(threads[k], NULL);
        }
        counter_threads = 0;
    }
    free(args);
}

void* single_f_merge(void* args) 
{
    struct only_merge_a* arg = args;

    int* array = arg->array;
    int min_1 = arg->min_1;
    int max_1 = arg->max_1;
    int min_2 = arg->min_2;
    int max_2 = arg->max_2;
    int* res = arg->res;
    int min_3 = arg->min_3;

    int it_1 = 0;
    int it_2 = 0;

    while(min_1 + it_1 <= max_1 && min_2 + it_2 <= max_2)
     {
        if (array[min_1 + it_1] < array[min_2 + it_2])
        {
            res[min_3 + it_1 + it_2] = array[min_1 + it_1];
            it_1++;
        } else 
        {
            res[min_3 + it_1 + it_2] = array[min_2 + it_2];
            it_2++;
        }
    }

    while (min_1 + it_1 <= max_1)
    {
        res[min_3 + it_1 + it_2] = array[min_1 + it_1];
        it_1++;
    }

    while (min_2 + it_2 <= max_2) 
    {
        res[min_3 + it_1 + it_2] = array[min_2 + it_2];
        it_2++;
    }
}

void merge(int* array, int min_1, int max_1, int min_2, int max_2, int* res, int min_3,
           pthread_t* threads, int p, int thread_id, struct only_merge_a* only_merge_arg) {
    int size_1 = max_1 - min_1 + 1;
    int size_2 = max_2 - min_2 + 1;

    if (size_1 < size_2) 
    {
        swap(&min_1, &min_2);
        swap(&max_1, &max_2);
        swap(&size_1, &size_2);
    }
    if (size_1 != 0) 
    {
        int middle_1 = (max_1 + min_1) / 2;
        int middle_2 = bin(array, array[middle_1], min_2, max_2);
        int middle_3 = min_3 + (middle_1 - min_1) + (middle_2 - min_2);
        res[middle_3] = array[middle_1];

        struct only_merge_a arg1;
        struct only_merge_a arg2;

        arg1.min_1 = min_1;
        arg1.max_1 = middle_1 - 1;
        arg1.min_2 = min_2;
        arg1.max_2 = middle_2 - 1;
        arg1.min_3 = min_3;
        arg1.array = array;
        arg1.res = res;

        arg2.min_1 = middle_1 + 1;
        arg2.max_1 = max_1;
        arg2.min_2 = middle_2;
        arg2.max_2 = max_2;
        arg2.min_3 = middle_3 + 1;
        arg2.array = array;
        arg2.res = res;

        if (p > 1) 
        {
            only_merge_arg[thread_id] = arg1;
            only_merge_arg[thread_id + p / 2] = arg2;
            pthread_create(threads + thread_id + p / 2, NULL, single_f_merge, only_merge_arg + thread_id + p / 2);
            single_f_merge(only_merge_arg + thread_id);
            pthread_join(threads[thread_id + p / 2], NULL);
        } else 
        {
            single_f_merge(&arg1);
            single_f_merge(&arg2);
        }
    }
}

void* single_merge(void* args) 
{
    struct merge_a* arg = args;
    int min_1 = arg->g;
    int max_1 = arg->g + arg->i / 2 - 1;
    int min_2 = arg->g + arg->i / 2;
    int max_2 = arg->g + arg->i - 1;
    if (max_2 < arg->n) 
    {
        merge(arg->array, min_1, max_1, min_2, max_2, arg->res, min_1, arg->threads,
              arg->p, arg->thread_id, arg->only_merge_a);
    } else 
    {
        if (min_2 < arg->n) 
        {
            merge(arg->array, min_1, max_1, min_2, arg->n - 1, arg->res, min_1, arg->threads,
                  arg->p, arg->thread_id, arg->only_merge_a);
        }
    }
}

void merge_sort(int* array, int n, int m, int* res_array, pthread_t* threads, int p) {
    chipping(array, n, m, threads, p);
    memcpy(res_array, array, sizeof(int) * n);

    if (p > 1) 
    {
        int counter_threads = 0;

        struct merge_a* merge_arg = (struct merge_a*)malloc(sizeof(struct merge_a) * p / 2);
        struct only_merge_a* only_merge_arg = (struct only_merge_a*)malloc(sizeof(struct only_merge_a) * p);

        for (int i = 2 * m; i <= 2 * n; i *= 2) 
        {
            for (int g = 0; g < n;) 
            {

                for (int k = 0; k < p / 2; k++) 
                {
                    if (g >= n) 
                    {
                        break;
                    }
                    struct merge_a arg;
                    arg.array = array;
                    arg.res = res_array;
                    arg.n = n;
                    arg.threads = threads;
                    arg.p = p;
                    arg.thread_id = k;
                    arg.i = i;
                    arg.g = g;
                    arg.only_merge_a = only_merge_arg;
                    merge_arg[k] = arg;
                    pthread_create(threads + k, NULL, single_merge, merge_arg + k);
                    counter_threads++;
                    g += i;
                }
                for (int l = 0; l < counter_threads; l++) 
                {
                    pthread_join(threads[l], NULL);
                }
                counter_threads = 0;
            }
            memcpy(array, res_array, sizeof(int) * n);
        }
        free(merge_arg);
        free(only_merge_arg);
    } else {
        struct merge_a* merge_arg = (struct merge_a*)malloc(sizeof(struct merge_a));
        for (int i = 2 * m; i <= 2 * n; i *= 2) 
        {
            for (int g = 0; g < n; g += i) 
            {
                struct merge_a arg;
                arg.array = array;
                arg.res = res_array;
                arg.n = n;
                arg.threads = threads;
                arg.p = p;
                arg.thread_id = 0;
                arg.i = i;
                arg.g = g;
                arg.only_merge_a = NULL;
                merge_arg[0] = arg;
                single_merge(merge_arg);
            }
            memcpy(array, res_array, sizeof(int) * n);
        }
        free(merge_arg);
    }
}

int main(int argc, char **argv) 
{
    srand(time(NULL));

    if (argc != 4) 
    {
        printf("Not enough arguments!\n");
        return 0;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int p = atoi(argv[3]);

    int* array = (int*)malloc(sizeof(int) * n);
    if (array == NULL) 
    {
        printf("Memory error");
    }
    for (int i = 0; i < n; i++) 
    {
        array[i] = rand() % 100;
    }

    FILE* data = fopen("data.txt", "w");
    for (int i = 0; i < n; i++) 
    {
        fprintf(data, "%d ", array[i]);
    }
    fprintf(data, "\n");

    pthread_t* threads = (pthread_t*)malloc((sizeof(pthread_t) * p));
    if (threads == NULL) 
    {
        printf("Memory error");
    }

    int* array_for_qsort = (int*)malloc(sizeof(int) * n);
    if (array_for_qsort == NULL) 
    {
        printf("Memory error");
    }
    memcpy(array_for_qsort, array, sizeof(int) * n);

    int* array_res = (int*)malloc(sizeof(int) * n);
    if (array_res == NULL) 
    {
        printf("Memory error");
    }

    double start_time_merge = omp_get_wtime();
    merge_sort(array, n, m, array_res, threads, p);
    double end_time_merge = omp_get_wtime();

    double start_time_quick = omp_get_wtime();
    qsort(array_for_qsort, n, sizeof(int), cmp);
    double end_time_quick = omp_get_wtime();

    printf("Merge: %lf\n", end_time_merge - start_time_merge);
    printf("Qsort: %lf\n", end_time_quick - start_time_quick);

    for (int i = 0; i < n; i++) 
    {
        fprintf(data, "%d ", array_res[i]);
    }

    FILE * stats = fopen("stats.txt", "w");
    fprintf(stats, "%lfs %d %d %d", end_time_merge - start_time_merge, n, m, p);

    fclose(data);
    fclose(stats);
    free(array_res);
    free(array_for_qsort);
    free(threads);
    free(array);

    return 0;
}