#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

double rand1();

int main(int argc, char **argv)
 {
  int a, b, x, n1, p1;
  double p;
  int b_end = 0;
 // int i;
  long all_steps = 0;
  double start_time, end_time;
  FILE *stats;

  if (argc != 7)
    {
        printf("Wrong\n");
        return 0;
    }

   a = atoi(argv[1]);
   b = atoi(argv[2]);
   x = atoi(argv[3]);
   n1 = atoi(argv[4]);
   p = atof(argv[5]);
   p1 = atoi(argv[6]);


   srand(time(0));
   start_time = omp_get_wtime();
   omp_set_num_threads(p1);
   #pragma omp parallel for reduction(+:all_steps, b_end)
   
   for(int i=0; i<n1; i++)
    {
        int pos = x;
        int steps_done = 0;        

        while (pos !=b && pos !=a)
        {
            if (rand1() <=p) {
                pos++;
            } else {
                pos--;
            }
            steps_done++;
        }
    
        if (pos == b)
        {
            b_end++;
            all_steps += steps_done;
        }
    }

   end_time = omp_get_wtime() - start_time;




   stats = fopen("stats.txt", "w");
   if (stats == NULL)
    {
        printf("Can't open the file\n");
        exit(-1);
    }
  fprintf(stats, "%f %f %f %d %d %d %d %f %d\n",
          (double) b_end / n1,
          (double) all_steps / n1,
          end_time,
          a, b, x, n1, p, p1);
  fclose(stats);
}

  double rand1() {
    int random;
    static __thread unsigned int point = 0;
    if (point == 0)
    {
        point = (unsigned int)rand();
    }
    random = rand_r(&point);
    return (double)random / RAND_MAX;
}


