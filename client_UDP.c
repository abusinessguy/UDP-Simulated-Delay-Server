#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

// struct    timeval  {
//   time_t        tv_sec ;   //used for seconds
//   suseconds_t       tv_usec ;   //used for microseconds
// };

struct mesg {
int label;
double position[6];
struct timeval timeSent;
};

#define ERROR (-1)

int main (int nba, char *arg[]) {
int result;
int nsend;
int nconnect;
struct mesg commandMesg;
struct mesg positionMesg;
int addr;
long int Te;
struct timeval currentTime;
struct timeval lastMesgTime;
double delay;
double mesgDelay;

struct sockaddr_in sockAddr, sock;
int serveur, client, err, nConnect, longaddr , results, resultr;

    serveur=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    sockAddr.sin_family=PF_INET;
    sockAddr.sin_port=htons(2000); 
    sockAddr.sin_addr.s_addr=inet_addr(arg[1]);
    addr=sizeof(sockAddr);

commandMesg.label=0;
commandMesg.position[0]=0.0;
commandMesg.position[1]=8.0;
gettimeofday(&commandMesg.timeSent, NULL);

positionMesg.label=0;
positionMesg.position[0]=0.0;

Te=200; // Te=200ms

fcntl(serveur,F_SETFL,fcntl(serveur,F_GETFL) | O_NONBLOCK); 
gettimeofday(&lastMesgTime, NULL);
do{

//usleep(Te); //Delay

gettimeofday(&currentTime, NULL);
mesgDelay = ((currentTime.tv_sec - lastMesgTime.tv_sec)*1000 + (currentTime.tv_usec - lastMesgTime.tv_usec)/1000);
if (mesgDelay > Te){
    commandMesg.label++;
    commandMesg.position[0]++;
    gettimeofday(&commandMesg.timeSent, NULL);
    results=sendto(serveur,&commandMesg,sizeof(commandMesg),0,(struct sockaddr*)&sockAddr,sizeof(sockAddr));
    //Sent
    printf("\n client : \n  label=%d sentPos=%lf size= %d ",commandMesg.label,commandMesg.position[0], results);

    gettimeofday(&lastMesgTime, NULL);
}

resultr=recvfrom(serveur,&positionMesg,sizeof(positionMesg), 0,(struct sockaddr*)&sockAddr,&addr);

//Recieved
if (resultr == -1) {
    //perror("recvfrom delay failed");
    // Handle error (e.g., exit or continue)
    } else {
    printf("\n Recieved : \n  label=%d sentPos=%lf recPos=%lf size= %d ",positionMesg.label,commandMesg.position[0], positionMesg.position[0], results);
    // printf("seconds : %ld\nmicro seconds : %ld",
    //     commandMesg.timeSent.tv_sec, commandMesg.timeSent.tv_usec);
    gettimeofday(&currentTime, NULL);
    delay = (currentTime.tv_sec - positionMesg.timeSent.tv_sec)*1000 + (currentTime.tv_usec - positionMesg.timeSent.tv_usec)/1000;
    printf("delay : %lf ms\n",delay);
    }
}while(positionMesg.label<300);

close(serveur);

return 0;

}



