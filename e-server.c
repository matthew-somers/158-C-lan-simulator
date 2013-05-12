#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9930
#define BUFLEN 1024
#define TIMEOUT 1 //in usec, aka 800ms

/**
* TODO: test it!
*
**/

int main(void)
{
   struct sockaddr_in si_me, si_other, si_other2;
   int s, i, slen=sizeof(si_other);
   char buf[BUFLEN];

   // socket()
   s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

   //complicated stuff for receiving
   memset((char *) &si_other, 0, sizeof(si_other));
   si_other.sin_family = AF_INET;
   si_other.sin_port = htons(PORT);

   memset((char *) &si_other2, 0, sizeof(si_other2));
   si_other2.sin_family = AF_INET;
   si_other2.sin_port = htons(PORT);

   //complicated stuff for sending acks back
   memset( (char *) &si_me, 0, sizeof(si_me) );
   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(PORT);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY);

   //bind()
   bind(s, &si_me, sizeof(si_me));

   //timeout socket so it can send
   struct timeval tv;
   tv.tv_sec = TIMEOUT; 
   setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

   int received = 0;

   //main loop, packets collide if within 800ms of each other
   while (1) 
   {
      bzero(buf, BUFLEN);
      recvfrom(s, buf, BUFLEN, 0, &si_other, &slen);

      if (strcmp("", buf) != 0) //we received something
      {
         if (received == 0)
         {
            si_other2 = si_other; //apparently this works
            received++; //first received now
         }

         else //both received, COLLISION!
         {
            printf("Collision occured between packets.\n");

            sprintf(buf, "COLLISION");
            printf("Sending \"COLLISION\" to %s\n", inet_ntoa(si_other.sin_addr));
            printf("Sending \"COLLISION\" to %s\n", inet_ntoa(si_other2.sin_addr));

            sendto(s, buf, BUFLEN, 0, &si_other, slen);
            sendto(s, buf, BUFLEN, 0, &si_other2, slen);
            received = 0;
         }
      }
      
      else //nothing received in one cycle
      {
         if (received == 1) //one packet received earlier, no collision
         {
            printf("Successfully received packet from %s:%d\n", 
               inet_ntoa(si_other2.sin_addr), ntohs(si_other2.sin_port));

            printf("Sending \"SUCCESS\" to %s\n\n", inet_ntoa(si_other2.sin_addr));
            
            sprintf(buf, "SUCCESS");
            sendto(s, buf, BUFLEN, 0, &si_other, slen);
            received = 0;
         }
      }

      bzero(buf, BUFLEN); //empty buffer at end
   }

   close(s);
   return 0;
}
