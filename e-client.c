#include <stdlib.h> /* exit */
#include <string.h> /* memset */
#include <unistd.h> /* close */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>

#define TIMESLOTS 5000
#define TIMEOUT 4 //seconds to wait for ack before moving on
#define PORT 9930
#define BUFLEN 1024

/**
* 
*
*
**/

double get_time_ms();
int randomNumber(int max);
double randomFrac();
int exponent(int base, int power);
int poisson(int l);

int main(int argc, char *argv[])
{
   //check args
   if (argc != 3) 
   {
      fprintf(stderr,"usage: %s ip lambda\n", argv[0]);
      exit(0);
   }

   int lambda = atoi(argv[2]);
   struct sockaddr_in si_other, si_me;
   int s, i, slen=sizeof(si_other);
   char buf[BUFLEN];
   char buf2[BUFLEN]; //second buffer to receive
     
   //socket()
   s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     
   //complicated stuff for sending
   memset((char *) &si_other, 0, sizeof(si_other));
   si_other.sin_family = AF_INET;
   si_other.sin_port = htons(PORT);
     
   //complicated stuff for binding and receiving
   memset( (char *) &si_me, 0, sizeof(si_me) );
   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(PORT);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
   //bind() because it needs to receive
   bind(s, &si_me, sizeof(si_me));
     
   //inet_aton() converts ip entered
   inet_aton(argv[1], &si_other.sin_addr);
     
   //timeout
   struct timeval tv;
   tv.tv_sec = TIMEOUT; 
   setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

   int success = 0;
   int lostpackets = 0;
   int waitslots = 0; //send packet to start
   int something = 0; //something for algorithm probably
   int collisions = 0;
   srand(time(NULL));

   //start clock
   double starttime = get_time_ms();

   //main loop, sends a packet, waits for an ack
   for (i = 0; i < TIMESLOTS; i++)
   {
      if ((waitslots == 0))
      {
         printf("Sending packet %d\n", (success+1));
         
         sendto(s, buf, BUFLEN, 0, &si_other, slen);
         recvfrom(s, buf2, BUFLEN, 0, &si_other, &slen);

         if (strcmp("SUCCESS", buf2) == 0) 
         {
            success++;
            printf("%d. Server received packet successfully.\n\n", i);
            waitslots = poisson(lambda);
         }

         else if (strcmp("COLLISION", buf2) == 0) //it collided with something!
         {
            collisions++;
            waitslots = randomNumber(exponent(2, collisions));//max of 2^c
            printf("%d. Packet collided, backing off for %d slots.\n", i, waitslots);
         }

         else //packet lost, shouldn't happen much with small size
         {
            lostpackets++;
            printf("%d. Packet lost.\n\n", i);
         }

         bzero(buf2, BUFLEN); //empty receive buffer at end
      }
      else
      {
         
            printf("%d. Waiting %d more slot(s).\n", i, waitslots);
            waitslots--;
         

      }

      usleep(800); //sleep at end of this slot 
   }
    
   //end clock
   double endtime = get_time_ms();
   double resulttime = (endtime - starttime) / success;
   double throughput = ((double)success / (double)TIMESLOTS);
   double throughput2 = ( (double)(success*BUFLEN*8.0) / ((endtime-starttime)/1000.0) ); //bps version

   printf("Packets sent: %d\n", success);
   printf("Packets lost: %d\n", lostpackets);
   printf("Average RTT (including backoff): %f milliseconds\n", resulttime);
   printf("Throughput: %f packets/timeslots\n", throughput);
   printf("Throughput: %f bps\n", throughput2);

   close(s);
}

double get_time_ms()
{
   struct timeval t;
   gettimeofday(&t, NULL);
   return (t.tv_sec + (t.tv_usec / 1000000.0)) * 1000.0;
}

int poisson(int l)
{
   double u = randomFrac();
printf("u %f ", u);
   double logu = log10(u);
printf("logu %f\n", logu);
   return (int)(-1*l*logu);
}

double randomFrac()
{
   double f = 0.000001 + ((double)rand() / (double)RAND_MAX);
   return f;
}

int randomNumber(int max)  
{
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
