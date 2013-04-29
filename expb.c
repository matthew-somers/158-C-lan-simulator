#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define TIMESLOT 2000
#define MAXWAIT 5000000
#define N 100 //number of stations
#define LOOPER 100; //number of times to run program to collect results

struct station {
int id;
int sent;
int collisions;
int wait;
};

int randomNumber(int max)  {
  //printf("max: %d\n", max);
  return 1 + (rand() % max);
}

int exponent(int base, int power)
{
  int result = base;
  if(power == 0)
    return 1;
  if(power == 1)
    return base;
  return base * exponent(base, power-1);
}

void cycle(struct station *station, int *collisionNow, int *collisionNext, int *finishedCount, int *avgdelays, int counter)
{
  
  if(station -> wait == 0)  {//this station transmits this cycle
    if(*collisionNow == 1)  {//if there is no collision then it is sent
      printf("Station %d sent successfully\n", station -> id);
      avgdelays[*finishedCount] += counter;
      station -> sent++;
      *finishedCount = *finishedCount + 1;
    }
    else  {//if there is collision, exponential backoff
      station -> collisions++;
      station -> wait = randomNumber(exponent(2, station -> collisions));//max of 2^c
      printf("Station %d set to sleep for %d cycles\n", station -> id, station -> wait);
    }
  }
  
  if(station -> wait == 1)  {//counter to detect if there will be a collision next cycle
    *collisionNext = *collisionNext + 1;
  }
  station -> wait--;
}

int main() {

int m;
int i;
static int avgdelays[N]; //in order of which is successful
for (m = 0; m < 100; m++) //LOOPER not working??
{
     useconds_t maxwait = MAXWAIT; /* 5 seconds */
     useconds_t current = TIMESLOT; /* Start with 2ms */

     srand(time(NULL));

     int finishedCount = 0;
     struct station stations[N];

     int collisionNow = N;//number of stations transmitting in the currecnt cycle
     int collisionNext = N;//counts the number of stations transmitting next cycle
     int counter = 0;//counts the number of cycles

     for(i = 0; i < N; i++)  {
       stations[i].id = i;
       stations[i].sent = 0;
       stations[i].collisions = 0;
       stations[i].wait = 0;
     }

     while(finishedCount < N)  {
       printf("Cycle %d\n", counter);
       collisionNow = collisionNext;
       collisionNext = 0;
       for(i = 0; i < N; i++)  {
         if(stations[i].sent == 0)  {
           cycle(&stations[i], &collisionNow, &collisionNext, &finishedCount, &avgdelays, counter);
           if(stations[i].sent == 1)  {//record successful cycle #
             stations[i].sent = counter;
           }
         }
         //usleep(TIMESLOT);
         counter++;
       }
     }
     printf("All Stations finished. Took %d cycles.\n", counter);

     /*for(i = 0; i < N; i++)  {
      avgdelays[i] += (stations[i].sent-1); //subtract 1 because of error
       printf("Station %d sent at cycle %d\n", stations[i].id, stations[i].sent);
     }*/
      printf("\n\n");

}
  
  int average = 0;
  for(i = 0; i < N; i++)  {
   avgdelays[i] /= LOOPER;
   average += avgdelays[i];
   printf("Average delay for successful station %d: %d cycles\n", (i+1), avgdelays[i]);
  }

   average /= N;
   printf("Average delay for all %d stations: %d cycles\n\n", N, average);
  return 0;
}
