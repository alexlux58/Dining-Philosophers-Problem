#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/*
 * name: Alex Lux
 * lab: 07 part 2 philosophers
 * date: 3/22/2021
 */

#define SLEEP_TIME ( (rand() % 5 + 1) * 1000)

void *philosopher(void *id);

void test(int i);

void pickup(int id);

void putdown(int id);

pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t *self;

int *state;

enum {THINKING, HUNGRY, EATING};

int numOfSeats, numOfTurns;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: philosophers <number of seats> <number of turns>");
        return 1;
    }

    numOfSeats = strtod(argv[1], NULL);
    numOfTurns = strtod(argv[2], NULL);

    self = calloc(numOfSeats, sizeof(pthread_cond_t));
    state = calloc(numOfSeats, sizeof(int));

    // set the seed for random number generator
    srand(time(NULL));

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    printf("Start a dinner for %d diners\n", numOfSeats);

    pthread_t philosopher_tid[numOfSeats];

    long i;

    for(i = 0; i < numOfSeats; i++)
        state[i] = THINKING;

    pthread_mutex_init(&monitor_mutex, NULL);

    for (i = 0; i < numOfSeats; i++)
        pthread_cond_init(self + i, NULL);

    for (i = 0; i < numOfSeats; i++)
        pthread_create(&philosopher_tid[i], NULL, philosopher, (void *) i);

    for (i = 0; i < numOfSeats; i++)
        pthread_join(philosopher_tid[i], NULL);

    pthread_mutex_destroy(&monitor_mutex);

    for(i = 0; i < numOfSeats; i++)
        pthread_cond_destroy(self + i);

    printf("Dinner is no more.\n");

    return 0;
}

void pickup(int i)
{
    pthread_mutex_lock(&monitor_mutex);
    state[i] = HUNGRY;
    test(i);

    if(state[i] != EATING)
        pthread_cond_wait(&(self[i]), &monitor_mutex);

    pthread_mutex_unlock(&monitor_mutex);
}

void putdown(int i)
{
    pthread_mutex_lock(&monitor_mutex);
    state[i] = THINKING;
    // test left and right neighbors
    test((i + (numOfSeats-1)) % numOfSeats);
    test((i + 1) % numOfSeats);
    pthread_mutex_unlock(&monitor_mutex);
}

void test(int i)
{
    if((state[(i + (numOfSeats-1)) % numOfSeats]) != EATING
        && (state[i] == HUNGRY) && (state[(i+1) % numOfSeats] != EATING))
    {
        state[i] = EATING;
        pthread_cond_signal(&(self[i]));
    }
}


void *philosopher(void *num)
{
    int id = (long) num;

    printf("Philsopher no. %d sits at the table.\n", id);

    // philosophers arrive at the table at various times
    usleep(SLEEP_TIME);

    int i;
    for (i = 0; i < numOfTurns; i++)
    {
        printf("Philsopher no. %d gets hungry for the %d time!\n", id, i + 1);

        printf("Philsopher no. %d tries to grab chopsticks %d and %d\n",
               id, id, (id + 1) % numOfSeats);

        pickup(id);

        printf("Philosopher no. %d is eating! \n", id);

        usleep(SLEEP_TIME * 1);

        printf("Philsopher no. %d stopped eating\n", id);

        putdown(id);

        printf("Philsopher no. %d put down chopsticks %d and %d\n",
               id, id, (id + 1) % numOfSeats);

        printf("Philsopher no. %d finished turn %d\n", id, i + 1);
    }

    printf(">>>>>> Philsopher no. %d finished meal. <<<<<<\n", id);

    pthread_exit(NULL);
}

