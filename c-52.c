/*
 REQUIRES -lm TO COMPILE
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//#include <math.h> // for log10()
#define TIMESLOT 2000
#define MAXWAIT 5000000
#define N 20 //number of stations
#define LOOPER 50 //number of times to run program to collect results
#define LAMBDA 20

int lowestFirstTrans = 9999999;

struct station {
	int id;
	int sent;
	int collisions;
	int wait;
};

double randomNumber(int max)  {
	double f = ( (double)rand() / (double)RAND_MAX  );
	return (0.000001 + f);
}

void cycle(struct station *station, int *collisionPrev, int *collisionNow, int *collisionNext, int *finishedCount, int *avgdelays, int counter)
{

	if(station -> wait == 0)  {//this station transmits this cycle
		if(*collisionNow == 1)  {//if there is no collision then it is sent; if there is only one station transmitting it is sent
			printf("Station %d sent successfully\n", station -> id);
			avgdelays[*finishedCount] += counter;

         if (counter < lowestFirstTrans)
            lowestFirstTrans = counter;

			station -> sent++;
			*finishedCount = *finishedCount + 1;
		}
		else  {//if there is collision, exponential backoff
			station -> collisions++;
			//station -> wait = randomNumber1(exponent(2, station -> collisions));//max of 2^c
			double u = randomNumber(1);
			double logu = log10(u);
			station -> wait = (-1*LAMBDA*logu);
			printf("Station %d set to sleep for %d cycles\n", station -> id, station -> wait);
		}
	}
//*collisionPrev == 1 || 
	if(station -> wait == 1 || station -> wait == 2)  {//counter to detect if there will be a collision next cycle
		*collisionNext = *collisionNext + 1;
	}
	station -> wait--;
}

int main() {
	int m;
	int i;
	static int avgdelays[N]; //in order of which is successful
	for (m = 0; m < LOOPER; m++)
	{
		srand(time(NULL));

		int finishedCount = 0;
		struct station stations[N];

		int collisionPrev = 0;
		int collisionNow = N;//number of stations transmitting in the currecnt cycle
		int collisionNext = N;//counts the number of stations transmitting next cycle
		int counter = 0;//counts the number of cycles

		for(i = 0; i < N; i++)  {
			stations[i].id = i;
			stations[i].sent = 0;
			stations[i].collisions = 0;
			stations[i].wait = 0;
		}

		while(finishedCount < 1)  {
			printf("Cycle %d\n", counter);
			collisionPrev = collisionNow;
			collisionNow = collisionNext;
			collisionNext = 0;
			for(i = 0; i < N; i++)  {
				if(stations[i].sent == 0)  {
					cycle(&stations[i], &collisionPrev,&collisionNow, &collisionNext, &finishedCount, &avgdelays, counter);
					if(stations[i].sent == 1)  {
						break;
					}
				}
				//usleep(TIMESLOT);
				counter++;
			}
		}
		printf("First station finished. Took %d cycles.\n", counter);

		printf("\n\n");

	}

		avgdelays[0] /= LOOPER;
		printf("Average for successful station 1: %d\n", avgdelays[0]);
      printf("Lowest delay: %d cycles.\n", lowestFirstTrans);

	return 0;
}
