// for hw on shiyanlou of os course
// producer & consumer
// wql
// gantiandongdi wojingranxiechulaile

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include<sys/mman.h>
#include<sys/wait.h>


#define N 10
#define M 500


int buf[N];
pthread_t producer_thread, consumer_thread1, consumer_thread2;
sem_t *chars, *space, *lock;
int producer_item = 0;
pthread_mutexattr_t mutexattr;


// no itoa() && atoi() in Linux, so I write them myself
void my_itoa(int num, char *s)
{

    if (num == 0)
    {
        s[0] = '0';
        //printf("Final s: %s\n", s);
        return;
    }

    char tmps[10];

    int cnt = 0;
    int re;
    while(num != 0)
    {  
        re = num % 10;
        num = num / 10;
        tmps[cnt++] = '0' + re;
    }

    // reverse
    cnt--;
    for (int j = 0; j < 10; j++)
    {
        s[j] = tmps[cnt--];
        if (cnt < 0)
        {
            break;
        }
    }
    //printf("Final s: %s\n", s);

}

void* producer(void* var) {
    //printf("The Producer pid: %d\n", getpid());
    //fflush(stdout);
    fclose(fopen("store.txt", "w"));    // clear file

    while(producer_item <= M) {
    	// wait
        sem_wait(space);
        sem_wait(lock);
        //pthread_mutex_lock(&mutex);
        //printf("start producing\n");
        //fflush(stdout);

        // semaphore "space" makes sure that buf will surely have space for producer to put number in, so we don't need to check and wait here.
        //sleep(10);
        // produce
        char tmp[10];
        my_itoa(producer_item, tmp);
        producer_item++;

        int j;
        for (j = 0; j < 10; j++)
        {
            if (tmp[j] == '\0' || tmp[j] == '\n')
            {
                tmp[j] = '\n';
                break;
            }
        }

        FILE *fp = fopen("store.txt", "a");
        fprintf(fp, "%s", tmp);
        fflush(fp);
        fclose(fp);
        //printf("%d produce: %s", getpid(), tmp);
        //fflush(stdout);

        sleep(1);

        //signal
        //pthread_mutex_unlock(&mutex);
        sem_post(lock);
        sem_post(chars);
        //printf("unlock\n");
        //fflush(stdout);

        //sleep(1);
    }
}

void* consumer(void* var) {
    //printf("The Consumer pid: %d\n", getpid());
    //fflush(stdout);
    while(1) {
        //printf("waiting...\n");
    	// wait
        sem_wait(chars);
        sem_wait(lock);
        //pthread_mutex_lock(&mutex);
        //printf("I'm in...\n");
        //fflush(stdout);
        // semaphore "chars" makes sure that there will surely be resources in buf, so we don't need to wait here.

        // get the number & print it
        //sleep(10);
        char buffer[10];
        int j;
        FILE *fp = fopen("store.txt", "r");
        fgets(buffer, sizeof(buffer), fp);
        //printf("%d consumes: %s", getpid(), buffer);
        printf("%d: %s", getpid(), buffer);
        fflush(stdout);

        //printf("start rewrite...");
        //fflush(stdout);
        // rewrite
        FILE *fp2 = fopen("tmp.txt", "w");
        while(!feof(fp))
        {
            if (fgets(buffer, sizeof(buffer), fp) == NULL)
                break;
            //printf("copy %s", buffer);
            //fflush(stdout);
            fprintf(fp2, "%s", buffer);
            fflush(fp);
            fflush(fp2);
        }
        fclose(fp);
        fclose(fp2);
        //sleep(10);
        remove("store.txt");
        rename("tmp.txt", "store.txt");
        //printf("rename\n");
        //fflush(stdout);


        sleep(1);

        // signal
        //pthread_mutex_unlock(&mutex);
        sem_post(lock);
        sem_post(space);

        sleep(3);
    }
}



int main(int argc, char *argv[])
{

    //pthread_mutexattr_init(&mutexattr);  
    //pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);    
	//pthread_mutex_init(&mutex,&mutexattr);

    // incase it has been linked.
    sem_unlink("/chars");
    sem_unlink("/space");
    sem_unlink("/lock");
    chars = sem_open("/chars", O_CREAT, 0644, 0);
    space = sem_open("/space", O_CREAT, 0644, N);
    lock = sem_open("/lock", O_CREAT, 0644, 1);


    // multiple processes
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Error occurrs in forking...\n");
    }
    else if (pid == 0)
    {
        //printf("Child process\n");
        pthread_create(&consumer_thread1, NULL, consumer, NULL);
    }
    else
    {
        //printf("Parent process\n");
        pid_t pid2 = fork();
        if (pid2 < 0)
        {
            printf("Error occurrs in forking...\n");
        }
        else if (pid2 == 0)
        {
            //printf("Child process2\n");
            pthread_create(&consumer_thread2, NULL, consumer, NULL);
        }
        else
        {
            //printf("Still parent process\n");
            pthread_create(&producer_thread, NULL, producer, NULL);
            
        }
    }



    pthread_exit(NULL);
        
}