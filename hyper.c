
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t lock;
 int x = 0;
struct myArguements {
    int n;
};

void *threadFunc(void *arg)
{
   // struct myArguements * args = (struct myArguements *) arg;
 //   printf("My thread: %d\n", args->n);

    for(int i=0; i < 100000; i++) {
        pthread_mutex_lock(&lock);
        x++;
       pthread_mutex_unlock(&lock);
    }
}

int main()

{
    pthread_t tid[100];

    for (int i = 0; i < 100; i++)
    {
       // struct myArguements *a = (struct myArguements *) malloc(sizeof(struct myArguements));
      //  a->n = i;
        pthread_create(&tid[i], NULL, threadFunc, NULL);
    
    }
     for (int i = 0; i < 100; i++)
    {
        pthread_join(tid[i], NULL);
    
    }

  //  pthread_exit(NULL);
    printf("%d\n", x);
    return 0;
}