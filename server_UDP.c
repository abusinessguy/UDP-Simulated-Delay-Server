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

struct mesg {
    int label;
    double position[6];
    struct timeval timeSent;
};

#define ERROR (-1)

int main(int argc, char *argv[]) {
    int server, results1, results2;
    struct sockaddr_in serverAddr, robotAddr;
    int addrLen = sizeof(serverAddr);
    struct mesg receivedMesg, robotMesg;
    receivedMesg.label = 1;
    ssize_t bytes_received;
    struct timeval currentTime;
    int unsentMesg; //flag to check if a new message arrived
    double delay; //measure current delay
    double intendedDelay; // delay we intend to insert, will increase by "delayIncrease"
    double delayLevelPeriod; //measure current amount of time for delay period
    double intendedDelayLevelPeriod; //Period we will rest at for some intended delay
    struct timeval periodTime;//measure current amount of time for delay period
    int delayIncrease; //amount delay will increase by each period

    // Create socket for original client
    server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(2000); // Assuming port 2000 for original client
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    int bind_result = bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bind_result == -1) {
        perror("bind failed");
        // Handle error (e.g., exit or continue)
    } else {
        // Binding successful, continue with the program
        printf("Binding successful\n");
    }

    // Create socket for robot_client
    int robotServer = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    robotAddr.sin_family = PF_INET;
    robotAddr.sin_port = htons(2200); // Assuming port 3000 for robot_client
    robotAddr.sin_addr.s_addr = INADDR_ANY;


    fcntl(server,F_SETFL,fcntl(server,F_GETFL) | O_NONBLOCK);
    fcntl(robotServer,F_SETFL,fcntl(robotServer,F_GETFL) | O_NONBLOCK);

    intendedDelay = 5;
    delayIncrease = 5;
    intendedDelayLevelPeriod = 5000;
    gettimeofday(&periodTime, NULL);


    while (receivedMesg.label<300) {
        //Calculate intendedDelay
        gettimeofday(&currentTime, NULL);
        delayLevelPeriod = ((currentTime.tv_sec - periodTime.tv_sec)*1000 + (currentTime.tv_usec - periodTime.tv_usec)/1000);
        if (delayLevelPeriod >= intendedDelayLevelPeriod){
            intendedDelay += delayIncrease;
            gettimeofday(&periodTime, NULL);
        }

        //printf("loop\n");
        // Receive message from the original client
        bytes_received = recvfrom(server, &receivedMesg, sizeof(receivedMesg), 0, (struct sockaddr*)&serverAddr, &addrLen);
        if (bytes_received == -1) {
            // Handle error (e.g., exit or continue)
            //perror("recvfrom client failed");
        } else {
            // Process received data
            printf("Received position from client: label=%d, position=%lf\n", receivedMesg.label, receivedMesg.position[0]);
            receivedMesg.position[0]++;
            unsentMesg = 1;
        }
        gettimeofday(&currentTime, NULL);
        delay = ((currentTime.tv_sec - receivedMesg.timeSent.tv_sec)*1000 + (currentTime.tv_usec - receivedMesg.timeSent.tv_usec)/1000);
        if ((delay >= intendedDelay) && (unsentMesg == 1)){
            // Send the original message to the robot_client
            results1=sendto(robotServer, &receivedMesg, sizeof(receivedMesg), 0, (struct sockaddr*)&robotAddr, sizeof(robotAddr));
            //printf("delay : %lf ms\n",delay);
            unsentMesg = 0;
        }
        // printf("loop2\n");
        // Receive position data from the robot_client
        bytes_received = recvfrom(robotServer, &robotMesg, sizeof(robotMesg), 0, (struct sockaddr*)&robotAddr, &addrLen);
        if (bytes_received == -1) {
            // Handle error (e.g., exit or continue)
            //perror("recvfrom robot failed");
        } else {
            // Print the received position data for verification
            printf("Received position from robot_client: label=%d, position=%lf\n", robotMesg.label, robotMesg.position[0]);
            // Introduce a delay (if needed)

            // Send the received position data to the original client
            results2=sendto(server, &robotMesg, sizeof(robotMesg), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }

        
        
        
    }

    close(server);
    close(robotServer);

    return 0;
}
